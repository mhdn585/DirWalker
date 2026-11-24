/*
 * input.c — Lectura desde el descriptor 0 con read(2), sin usar la caché de
 * stdio: así el paginador (tecla a tecla) y el menú nunca se roban bytes.
 */
#include "input.h"

#include "app.h"

#include <errno.h>
#include <unistd.h>

bool input_line(char *buf, size_t size)
{
    size_t o = 0;
    unsigned char c;

    for (;;) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1) {
            if (c == '\n' || c == '\r') {
                buf[o] = '\0';
                return true;
            }
            if (o + 1 < size) {
                buf[o++] = (char)c;
            }
            /* Si desborda, el resto de la línea se descarta: el buffer
             * del próximo prompt queda limpio. */
            continue;
        }
        if (n < 0 && errno == EINTR) {
            /* Ctrl+C: devolver el control al llamador con la bandera puesta. */
            buf[o] = '\0';
            return !app_stop_requested();
        }
        /* EOF (n == 0) o error irreversible. */
        buf[o] = '\0';
        return false;
    }
}

int input_key(void)
{
    unsigned char c;

    for (;;) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1) {
            return (int)c;
        }
        if (n < 0 && errno == EINTR) {
            return -1; /* el llamador distingue EOF vs. señal con app_stop */
        }
        if (n == 0) {
            return -1; /* EOF */
        }
    }
}
