/*
 * app.h — Constantes de la aplicación y estado de proceso (señales).
 * Única razón de cambio: configuración global del programa.
 */
#ifndef APP_H
#define APP_H

#define APP_NAME    "DirWalker"
#define APP_PROMPT  ">>>> "
#define MAX_INPUT   4096      /* bytes máximos por línea de entrada */
#define NFTW_MAXFD  64        /* descriptores que nftw puede abrir */

/* Instala el handler de SIGINT (bandera, nunca salida brusca). */
void app_install_signals(void);

/* true si el usuario pidió cancelar (Ctrl+C). */
bool app_stop_requested(void);

/* Limpia la bandera al comenzar una nueva operación. */
void app_reset_stop(void);

#endif /* APP_H */
