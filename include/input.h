/*
 * input.h — Lectura robusta desde stdin (fd 0, sin stdio para no mezclar
 * buffers con la lectura de teclas del paginador).
 * Única razón de cambio: cómo llegan los bytes del teclado.
 */
#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

/*
 * Lee una línea (sin contar \n ni \r) en buf, descartando el resto si es
 * demasiado larga. Devuelve false ante EOF o interrupción por señal.
 */
bool input_line(char *buf, size_t size);

/* Lee un byte sin esperar Enter (requiere modo raw previo). -1 en EOF/error. */
int input_key(void);

#endif /* INPUT_H */
