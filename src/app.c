/*
 * app.c — Estado de proceso: handler de SIGINT como bandera cooperativa.
 * El handler solo marca; los puntos de chequeo (callback de nftw, paginador,
 * lectura del menú) deciden cómo detenerse limpiamente.
 */
#include "app.h"

#include <signal.h>

static volatile sig_atomic_t g_stop;

static void on_sigint(int sig)
{
    (void)sig;
    g_stop = 1;
}

void app_install_signals(void)
{
    struct sigaction sa = { .sa_handler = on_sigint };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
}

bool app_stop_requested(void)
{
    return g_stop != 0;
}

void app_reset_stop(void)
{
    g_stop = 0;
}
