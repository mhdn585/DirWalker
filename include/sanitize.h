/*
 * sanitize.h — Neutralización de secuencias de escape en nombres de
 * archivos: nada leído del disco puede emitir escapes ANSI crudos a la
 * terminal (anti terminal-escape-injection).
 * Única razón de cambio: las reglas de qué bytes son “peligrosos”.
 */
#ifndef SANITIZE_H
#define SANITIZE_H

#include <stddef.h>

/*
 * Devuelve una cadena nueva (malloc) apta para imprimir, o nullptr si no hay
 * memoria. Reglas:
 *   ESC (0x1B)      -> \e
 *   otros < 0x20    -> \xHH
 *   DEL (0x7F)      -> \x7F
 *   0x80+ (UTF-8)   -> intacto
 * base_in es el offset del basename dentro de raw; base_out recibe el
 * offset equivalente ya sanitizado.
 */
char *sanitize_display(const char *raw, size_t len, size_t base_in,
                       size_t *base_out);

#endif /* SANITIZE_H */
