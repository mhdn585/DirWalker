/*
 * os.h — Detección del sistema operativo y resolución del directorio raíz.
 * Única razón de cambio: dónde y cómo se obtiene esa información del SO.
 */
#ifndef OS_H
#define OS_H

#include <stddef.h>

typedef struct {
    bool is_linux;      /* true si el kernel informado es Linux */
    char sysname[65];   /* nombre legible devuelto por uname(3) */
} OsInfo;

/* Rellena info. Devuelve false si uname() falla. */
bool os_detect(OsInfo *info);

/* Escribe en buf la raíz del recorrido (getpwuid -> HOME). false si no hay. */
bool os_home_path(char *buf, size_t size);

#endif /* OS_H */
