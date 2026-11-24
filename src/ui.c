/*
 * ui.c — Banner, menú, prompt y mensajes. Aquí vive TODO texto de la
 * interfaz; ningún otro módulo imprime prosa directamente.
 */
#include "ui.h"

#include "app.h"
#include "color.h"
#include "input.h"
#include "palette.h"

#include <stdio.h>

static const char BANNER[] =
" ██████████    ███            █████   ███   █████           ████  █████                        \n"
"░░███░░░░███  ░░░            ░░███   ░███  ░░███           ░░███ ░░███                         \n"
" ░███   ░░███ ████  ████████  ░███   ░███   ░███   ██████   ░███  ░███ █████  ██████  ████████ \n"
" ░███    ░███░░███ ░░███░░███ ░███   ░███   ░███  ░░░░░███  ░███  ░███░░███  ███░░███░░███░░███\n"
" ░███    ░███ ░███  ░███ ░░░  ░░███  █████  ███    ███████  ░███  ░██████░  ░███████  ░███ ░░░ \n"
" ░███    ███  ░███  ░███       ░░░█████░█████░    ███░░███  ░███  ░███░░███ ░███░░░   ░███     \n"
" ██████████   █████ █████        ░░███ ░░███     ░░████████ █████ ████ █████░░██████  █████    \n"
"░░░░░░░░░░   ░░░░░ ░░░░░          ░░░   ░░░       ░░░░░░░░ ░░░░░ ░░░░ ░░░░░  ░░░░░░  ░░░░░     \n";

void ui_clear(void)
{
    /* Control de pantalla, no color: se emite incluso con NO_COLOR. */
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
}

void ui_banner(const OsInfo *os, const char *root)
{
    putchar('\n');
    c_fg(RGB_JUNGLE);
    c_bold();
    fputs(BANNER, stdout);
    c_reset();
    putchar('\n'); /* línea de separación entre el banner y la info */
    c_fg(RGB_SAND);
    c_dim();
    printf("   " APP_NAME " v1.0  ·  SO: %s  ·  raíz del recorrido: %s\n",
           os->sysname, root);
    c_reset();
    putchar('\n');
}

void ui_menu(void)
{
    c_fg(RGB_PACIFIC);
    fputs(" ────────────────────────────── MENÚ ──────────────────────────────\n",
          stdout);
    c_reset();
    fputs("   ", stdout);
    c_fg(RGB_ORANGE);
    c_bold();
    fputs("[1]", stdout);
    c_reset();
    c_fg(RGB_SAND);
    fputs("  Iniciar recorrido\n", stdout);
    c_reset();
    fputs("   ", stdout);
    c_fg(RGB_STRAW);
    c_bold();
    fputs("[2]", stdout);
    c_reset();
    c_fg(RGB_SAND);
    fputs("  Salir\n", stdout);
    c_reset();
    putchar('\n'); /* línea de separación entre el menú y el prompt */
}

void ui_prompt(void)
{
    c_fg(RGB_PACIFIC);
    c_bold();
    fputs(APP_PROMPT, stdout);
    c_reset();
    fflush(stdout);
}

void ui_error(const char *msg)
{
    c_fg(RGB_STRAW);
    c_bold();
    printf("  ✗ %s\n", msg);
    c_reset();
}

void ui_warn(const char *msg)
{
    c_fg(RGB_ORANGE);
    printf("  ! %s\n", msg);
    c_reset();
}

void ui_info(const char *msg)
{
    c_fg(RGB_SAND);
    printf("  · %s\n", msg);
    c_reset();
}

void ui_format_menu(void)
{
    putchar('\n');
    c_fg(RGB_PACIFIC);
    fputs(" ──────────────────── FORMATO DE IMPRESIÓN ────────────────────\n",
          stdout);
    c_reset();

    fputs("   ", stdout);
    c_fg(RGB_ORANGE);
    c_bold();
    fputs("[1]", stdout);
    c_reset();
    c_fg(RGB_SAND);
    fputs("  Lista plana con tipo\n", stdout);
    c_reset();

    fputs("   ", stdout);
    c_fg(RGB_ORANGE);
    c_bold();
    fputs("[2]", stdout);
    c_reset();
    c_fg(RGB_SAND);
    fputs("  Árbol jerárquico\n", stdout);
    c_reset();

    fputs("   ", stdout);
    c_fg(RGB_PACIFIC);
    c_bold();
    fputs("[0]", stdout);
    c_reset();
    c_fg(RGB_SAND);
    fputs("  Volver al menú sin imprimir\n", stdout);
    c_reset();
}

void ui_pager_footer(void)
{
    c_fg(RGB_SAND);
    c_dim();
    fputs(" ──────────  ENTER: siguiente página  ·  q: terminar lista  ──────────\n",
          stdout);
    c_reset();
    fflush(stdout);
}

bool ui_confirm_menu(void)
{
    char buf[MAX_INPUT];

    putchar('\n');
    c_fg(RGB_SAND);
    fputs(" ¿Desea volver al menú? (s/n): ", stdout);
    c_reset();
    fflush(stdout);

    for (;;) {
        if (!input_line(buf, sizeof(buf))) {
            return false; /* EOF o Ctrl+C: salir */
        }
        const char *b = buf;
        while (*b == ' ' || *b == '\t') {
            b++;
        }
        if (*b == 's' || *b == 'S') {
            return true;
        }
        if (*b == 'n' || *b == 'N') {
            return false;
        }
        ui_error("Responde s (sí) o n (no).");
    }
}

void ui_farewell(void)
{
    putchar('\n');
    c_fg(RGB_JUNGLE);
    c_bold();
    printf(" ¡Hasta la próxima! " APP_NAME " ha terminado.\n");
    c_reset();
}
