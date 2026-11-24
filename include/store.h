/*
 * store.h — Almacén dinámico en memoria de las entradas del recorrido.
 * Única razón de cambio: la estructura de datos del listado capturado.
 */
#ifndef STORE_H
#define STORE_H

#include <stddef.h>

typedef enum {
    T_DIR,        /* directorio */
    T_FILE,       /* archivo regular u otro objeto con stat OK */
    T_LINK,       /* enlace simbólico (sin seguir) */
    T_LINK_DEAD,  /* enlace simbólico roto */
    T_OTHER,      /* stat falló u otro tipo */
} EntryType;

typedef struct {
    char *path;       /* ruta completa YA sanitizada, de propiedad del store */
    size_t len;       /* longitud de path sin NUL */
    size_t base;      /* offset del basename en path */
    unsigned short level; /* profundidad que reportó nftw (raíz = 0) */
    EntryType type;
} Entry;

typedef struct {
    Entry *items;
    size_t count;
    size_t cap;
} Store;

/* Reserva la capacidad inicial. false si no hay memoria. */
bool store_init(Store *st);

/*
 * Transfiere la propiedad de path al store (ya sanitizado). En caso de
 * fallo libera path y devuelve false.
 */
bool store_add(Store *st, char *path, size_t base, int level, EntryType type);

/* Libera todas las cadenas y el arreglo. Seguro de llamar con count==0. */
void store_free(Store *st);

/* Etiqueta corta según tipo: [D] [F] [L] [X] [?] (6 bytes con espacio). */
const char *entry_type_tag(EntryType t);

#endif /* STORE_H */
