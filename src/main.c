/*
 * main.c — DirWalker: detección del SO, bucle del menú y orquestación.
 * Política de alto nivel únicamente; no imprime formatos ni recorre.
 */
#include "app.h"
#include "color.h"
#include "input.h"
#include "os.h"
#include "palette.h"
#include "render.h"
#include "store.h"
#include "ui.h"
#include "walk.h"

#include <stdio.h>
#include <string.h>

static char *ltrim(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

/*
 * Ejecuta el ciclo completo de un recorrido: nftw -> resumen ->
 * elección de formato -> impresión paginada.
 * Devuelve true para volver al menú, false para salir del programa.
 */
static bool run_walk(const char *root)
{
    Store store;
    if (!store_init(&store)) {
        ui_error("No hay memoria para iniciar el recorrido.");
        return true;
    }

    ui_info("Recorriendo, esto puede tardar unos segundos...");
    Stats stats;
    WalkStatus ws = walk_home(&store, root, &stats);

    if (ws == WALK_CANCELLED) {
        store_free(&store);
        app_reset_stop();
        ui_warn("Recorrido cancelado. Memoria liberada.");
        return true;
    }
    if (ws == WALK_FAILED) {
        store_free(&store);
        app_reset_stop();
        ui_error("El recorrido falló (revisa permisos o la ruta raíz).");
        return true;
    }

    render_summary(&store, &stats);

    PageOutcome outcome = PAGE_DONE;
    bool printed = false;
    char line[MAX_INPUT];

    for (;;) {
        ui_format_menu();
        ui_prompt();
        if (!input_line(line, sizeof line)) {
            break; /* EOF o Ctrl+C: omitir la impresión */
        }
        const char *choice = ltrim(line);
        if (strcmp(choice, "0") == 0) {
            break;
        }
        if (strcmp(choice, "1") == 0) {
            outcome = render_flat(&store);
            printed = true;
            break;
        }
        if (strcmp(choice, "2") == 0) {
            outcome = render_tree(&store, root);
            printed = true;
            break;
        }
        ui_error("Opción no válida: escribe 1, 2 o 0.");
    }

    store_free(&store);
    app_reset_stop();

    if (printed) {
        switch (outcome) {
        case PAGE_DONE:
            ui_info("Impresión completada.");
            break;
        case PAGE_QUIT:
            ui_warn("Saliste de la lista antes del final.");
            break;
        case PAGE_STOPPED:
            ui_warn("Impresión interrumpida. Memoria liberada.");
            break;
        }
    }

    return printed ? ui_confirm_menu() : true;
}

int main(void)
{
    app_install_signals();
    c_init();
    ui_clear(); /* el programa arranca siempre con la terminal limpia */

    /* 1) ¿Estamos en Linux? */
    OsInfo os = {};
    os_detect(&os);
    if (!os.is_linux) {
        c_reset();
        fprintf(stderr, "\n[" APP_NAME "] Este programa solo es compatible con Linux.\n");
        fprintf(stderr, "             Sistema detectado: %s\n\n",
                os.sysname[0] ? os.sysname : "desconocido");
        return 1;
    }

    /* 2) Raíz del recorrido: el home del usuario. */
    char root[MAX_INPUT];
    if (!os_home_path(root, sizeof root)) {
        fprintf(stderr, "\n[" APP_NAME "] No se pudo determinar el directorio home.\n");
        return 1;
    }

    /* 3) Bucle del menú. */
    c_reset();
    char line[MAX_INPUT];
    for (;;) {
        ui_clear(); /* un solo DirWalker en pantalla, nunca banners apilados */
        ui_banner(&os, root);
        ui_menu();
        ui_prompt();

        if (!input_line(line, sizeof line)) {
            if (app_stop_requested()) {
                app_reset_stop(); /* Ctrl+C en el menú: repintar */
                continue;
            }
            break; /* EOF (Ctrl+D): salida limpia */
        }

        const char *opt = ltrim(line);
        if (strcmp(opt, "1") == 0) {
            if (!run_walk(root)) {
                break;
            }
        } else if (strcmp(opt, "2") == 0) {
            break;
        } else {
            ui_error("Opción no válida. Escribe 1 o 2.");
        }
    }

    ui_farewell();
    c_reset(); /* nunca dejar escapes sueltos en la terminal */
    return 0;
}
