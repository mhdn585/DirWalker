/*
 * render.c — Vista plana, vista en árbol y paginación por pantalla.
 * Depende solo del store (datos ya sanitizados): no conoce nftw ni el SO.
 *
 * El árbol se dibuja directamente del orden de llegada: nftw entrega las
 * entradas en preorden DFS, así que el orden del store YA es el orden del
 * árbol. Con una pila monótona se calcula en O(n) si cada entrada es la
 * última de sus hermanos (para elegir ├── o └──), sin reconstruir punteros.
 */
#include "render.h"

#include "app.h"
#include "color.h"
#include "input.h"
#include "palette.h"
#include "ui.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/* ── paginación ─────────────────────────────────────────────────────── */

typedef struct {
    int per_page;   /* entradas por pantalla */
    struct termios saved;
    bool raw;
} Pager;

static int terminal_rows(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row >= 10) {
        return (int)ws.ws_row;
    }
    return 24;
}

static void pager_open(Pager *p)
{
    p->per_page = terminal_rows() - 6;
    if (p->per_page < 4) {
        p->per_page = 4;
    }
    p->raw = false;
    if (tcgetattr(STDIN_FILENO, &p->saved) == 0) {
        struct termios r = p->saved;
        r.c_lflag &= ~(unsigned)(ICANON | ECHO); /* ISIG intacto: Ctrl+C sigue vivo */
        r.c_cc[VMIN] = 1;
        r.c_cc[VTIME] = 0;
        p->raw = (tcsetattr(STDIN_FILENO, TCSANOW, &r) == 0);
    }
}

static void pager_close(Pager *p)
{
    if (p->raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &p->saved);
        p->raw = false;
    }
}

/* Espera tecla al pie de página. PAGE_DONE = continuar. */
static PageOutcome pager_wait(const Pager *p)
{
    (void)p;
    ui_pager_footer();
    for (;;) {
        if (app_stop_requested()) {
            return PAGE_STOPPED;
        }
        int k = input_key();
        if (k < 0) {
            return app_stop_requested() ? PAGE_STOPPED : PAGE_QUIT;
        }
        if (k == '\n' || k == '\r' || k == ' ') {
            return PAGE_DONE;
        }
        if (k == 'q' || k == 'Q') {
            return PAGE_QUIT;
        }
        /* cualquier otra tecla se ignora */
    }
}

/* Devuelve false para interrumpir el bucle de impresión. */
static bool pager_step(Pager *p, size_t i, size_t total, PageOutcome *out)
{
    if ((i + 1u) % (size_t)p->per_page == 0u && i + 1u < total) {
        *out = pager_wait(p);
        return (*out == PAGE_DONE);
    }
    return true;
}

/* ── helpers de color por tipo ──────────────────────────────────────── */

static void color_for_type(EntryType t)
{
    switch (t) {
    case T_DIR:
        c_fg(RGB_PACIFIC);
        c_bold();
        break;
    case T_FILE:
        c_fg(RGB_SAND);
        break;
    case T_LINK:
        c_fg(RGB_ORANGE);
        break;
    case T_LINK_DEAD:
        c_fg(RGB_STRAW);
        break;
    case T_OTHER:
        c_fg(RGB_JUNGLE);
        break;
    }
}

/* ── resumen ────────────────────────────────────────────────────────── */

static void stat_row(const char *label, size_t value)
{
    c_fg(RGB_ORANGE);
    printf("   %18s", label);
    c_reset();
    c_fg(RGB_SAND);
    printf("  ....  %zu\n", value);
    c_reset();
}

void render_summary(const Store *st, const Stats *s)
{
    putchar('\n');
    c_fg(RGB_PACIFIC);
    c_bold();
    fputs(" ───────────────────────── RESUMEN ─────────────────────────\n",
          stdout);
    c_reset();
    stat_row("Directorios", s->dirs);
    stat_row("Archivos", s->files);
    stat_row("Enlaces", s->links);
    stat_row("Enlaces rotos", s->dead);
    stat_row("Sin acceso", s->unreadable);
    stat_row("Otros", s->other);
    stat_row("Total entradas", st->count);

    c_fg(RGB_ORANGE);
    fputs("           Tiempo", stdout);
    c_reset();
    c_fg(RGB_SAND);
    printf("  ....  %.2f s\n", s->secs);
    c_reset();
}

/* ── vista plana ────────────────────────────────────────────────────── */

PageOutcome render_flat(const Store *st)
{
    if (st->count == 0) {
        ui_info("El recorrido no produjo ninguna entrada.");
        return PAGE_DONE;
    }

    Pager pg;
    pager_open(&pg);

    PageOutcome out = PAGE_DONE;
    for (size_t i = 0; i < st->count; i++) {
        const Entry *e = &st->items[i];

        c_fg(RGB_ORANGE);
        fputs(entry_type_tag(e->type), stdout);
        c_reset();
        color_for_type(e->type);
        fputs(e->path, stdout);
        c_reset();
        putchar('\n');

        if (!pager_step(&pg, i, st->count, &out)) {
            break;
        }
    }

    pager_close(&pg);
    return out;
}

/* ── vista en árbol ─────────────────────────────────────────────────── */

PageOutcome render_tree(const Store *st, const char *root)
{
    const size_t n = st->count;
    if (n == 0) {
        ui_info("El recorrido no produjo ninguna entrada.");
        return PAGE_DONE;
    }

    uint8_t *is_last = malloc(n * sizeof *is_last);
    uint32_t *stack = malloc(n * sizeof *stack);
    if (is_last == nullptr || stack == nullptr) {
        free(is_last);
        free(stack);
        ui_error("Sin memoria para dibujar el árbol.");
        return PAGE_QUIT;
    }

    /* Pila monótona: para cada entrada, si es la última entre sus
     * hermanos (el siguiente índice con nivel <= tiene nivel menor). */
    size_t top = 0;
    for (size_t i = 0; i <= n; i++) {
        unsigned cur = (i == n) ? 0u : (unsigned)st->items[i].level;
        while (top > 0 && (unsigned)st->items[stack[top - 1]].level >= cur) {
            size_t j = stack[--top];
            is_last[j] = (i == n || (unsigned)st->items[i].level
                          < (unsigned)st->items[j].level) ? 1u : 0u;
        }
        if (i < n) {
            stack[top++] = (uint32_t)i;
        }
    }

    /* Pasada de dibujo: anc[d] = índice del ancestro vivo a profundidad d. */
    uint32_t anc[256];
    memset(anc, 0, sizeof anc);

    Pager pg;
    pager_open(&pg);

    PageOutcome out = PAGE_DONE;
    for (size_t i = 0; i < n; i++) {
        const Entry *e = &st->items[i];
        unsigned L = (unsigned)e->level;

        if (L == 0u) {
            /* Raíz del recorrido: ruta completa, sin conectores. */
            c_fg(RGB_PACIFIC);
            c_bold();
            fputs(e->path, stdout);
            c_reset();
            putchar('\n');
            anc[0] = (uint32_t)i;
        } else {
            if (L < 256u) {
                c_fg(RGB_JUNGLE);
                for (unsigned d = 1u; d < L; d++) {
                    fputs(is_last[anc[d]] ? "    " : "│   ", stdout);
                }
                fputs(is_last[i] ? "└── " : "├── ", stdout);
                c_reset();
                anc[L] = (uint32_t)i;
            } else {
                c_fg(RGB_JUNGLE);
                fputs("├── ", stdout); /* profundidad extrema: indentación plana */
                c_reset();
            }
            color_for_type(e->type);
            fputs(e->path + e->base, stdout);
            c_reset();
            putchar('\n');
        }

        if (!pager_step(&pg, i, n, &out)) {
            break;
        }
    }

    pager_close(&pg);
    free(is_last);
    free(stack);
    (void)root;
    return out;
}
