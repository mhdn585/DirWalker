/*
 * walk.h — Recorrido del árbol de directorios mediante nftw(3).
 * Única razón de cambio: qué y cómo se visita del sistema de archivos.
 * No imprime ni formatea: solo recolecta en el store (DIP: render depende
 * del store, no del recorrido).
 */
#ifndef WALK_H
#define WALK_H

#include <stddef.h>

#include "store.h"

typedef enum {
    WALK_DONE,       /* recorrido completo */
    WALK_CANCELLED,  /* interrumpido por el usuario (SIGINT) */
    WALK_FAILED,     /* error: ver errno (ENOENT, ENOMEM, EACCES en raíz...) */
} WalkStatus;

typedef struct {
    size_t dirs;        /* directorios accesibles (incluye no legibles) */
    size_t files;       /* archivos y otros objetos con stat correcto */
    size_t links;       /* enlaces simbólicos */
    size_t dead;        /* enlaces rotos */
    size_t unreadable;  /* directorios sin permiso de lectura */
    size_t other;       /* restantes */
    double secs;        /* duración del recorrido */
} Stats;

/*
 * Recorre root (típicamente $HOME) con nftw + FTW_PHYS (no sigue symlinks)
 * y acumula las entradas ya sanitizadas en store. Rellena stats y devuelve
 * el estado.
 */
WalkStatus walk_home(Store *store, const char *root, Stats *stats);

#endif /* WALK_H */
