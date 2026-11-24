/*
 * color.c — Emisión de secuencias ANSI True Color (\033[38;2;R;G;Bm).
 * Sin dependencias externas. Si NO_COLOR está definido en el entorno,
 * todas las funciones se vuelven no-ops (cortesía con terminales sin color).
 */
#include "color.h"

#include <stdio.h>
#include <stdlib.h>

static bool g_enabled = true;

void c_init(void)
{
    g_enabled = (getenv("NO_COLOR") == nullptr);
}

void c_fg(int r, int g, int b)
{
    if (g_enabled) {
        printf("\033[38;2;%d;%d;%dm", r, g, b);
    }
}

void c_bold(void)
{
    if (g_enabled) {
        fputs("\033[1m", stdout);
    }
}

void c_dim(void)
{
    if (g_enabled) {
        fputs("\033[2m", stdout);
    }
}

void c_reset(void)
{
    if (g_enabled) {
        fputs("\033[0m", stdout);
    }
}
