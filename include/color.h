/*
 * color.h — Secuencias ANSI de color real (38;2;R;G;B) sin dependencias.
 * Única razón de cambio: cómo se emiten escapes a la terminal.
 */
#ifndef COLOR_H
#define COLOR_H

/* Lee variables de entorno (NO_COLOR) y habilita/deshabilita el color. */
void c_init(void);

/* foreground true color: \033[38;2;R;G;Bm */
void c_fg(int r, int g, int b);

void c_bold(void);   /* \033[1m  */
void c_dim(void);    /* \033[2m  */
void c_reset(void);  /* \033[0m  */

#endif /* COLOR_H */
