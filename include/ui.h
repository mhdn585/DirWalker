/*
 * ui.h — Capa de interfaz de terminal: banner, menú, prompt, mensajes.
 * Única razón de cambio: el aspecto y los textos de la interfaz.
 */
#ifndef UI_H
#define UI_H

#include "os.h"

/* Limpia la pantalla y lleva el cursor al inicio (\033[2J\033[H). */
void ui_clear(void);

void ui_banner(const OsInfo *os, const char *root);
void ui_menu(void);
void ui_prompt(void);              /* ">>>> " sin salto de línea */

void ui_error(const char *msg);    /* rojo   */
void ui_warn(const char *msg);     /* naranja */
void ui_info(const char *msg);     /* neutro  */

void ui_format_menu(void);         /* ¿plana, árbol o menú? */
void ui_pager_footer(void);        /* pie de página del paginador */

/* Pide confirmación para volver al menú. false = salir. */
bool ui_confirm_menu(void);

void ui_farewell(void);            /* mensaje de despedida */

#endif /* UI_H */
