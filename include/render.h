/*
 * render.h — Presentación del almacén: resumen, lista plana, árbol y
 * paginación por pantalla.
 * Única razón de cambio: cómo se ve la información capturada.
 */
#ifndef RENDER_H
#define RENDER_H

#include "store.h"
#include "walk.h"

typedef enum {
    PAGE_DONE,     /* se imprimió todo */
    PAGE_QUIT,     /* el usuario salió de la lista con q */
    PAGE_STOPPED,  /* canceló con Ctrl+C */
} PageOutcome;

/* Imprime el resumen de estadísticas del recorrido. */
void render_summary(const Store *st, const Stats *stats);

/* Lista plana [D]/[F]... paginada. */
PageOutcome render_flat(const Store *st);

/* Árbol jerárquico paginado. root solo para mensajes de error. */
PageOutcome render_tree(const Store *st, const char *root);

#endif /* RENDER_H */
