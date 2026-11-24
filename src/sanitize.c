/*
 * sanitize.c — Convierte bytes de control (incl. ESC) en marcadores
 * visibles para que ningún nombre de archivo pueda inyectar secuencias
 * ANSI, BEL, movimientos de cursor, etc. en la terminal.
 */
#include "sanitize.h"

#include <stdlib.h>

static const char HEX[] = "0123456789ABCDEF";

char *sanitize_display(const char *raw, size_t len, size_t base_in,
                       size_t *base_out)
{
    /* Peor caso: cada byte se expande a 4 ("\xHH") + NUL. */
    char *out = malloc(len * 4u + 1u);
    if (out == nullptr) {
        return nullptr;
    }

    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        if (base_out != nullptr && i == base_in) {
            *base_out = o;
        }
        unsigned char ch = (unsigned char)raw[i];
        if (ch == 0x1B) {
            out[o++] = '\\';
            out[o++] = 'e';
        } else if (ch < 0x20 || ch == 0x7F) {
            out[o++] = '\\';
            out[o++] = 'x';
            out[o++] = HEX[(ch >> 4) & 0x0F];
            out[o++] = HEX[ch & 0x0F];
        } else {
            out[o++] = (char)ch;
        }
    }

    /* base_in fuera de rango (no ocurre con nftw, defensa extra). */
    if (base_out != nullptr && base_in >= len) {
        *base_out = 0;
    }

    out[o] = '\0';
    return out;
}
