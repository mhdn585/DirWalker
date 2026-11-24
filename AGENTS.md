# AGENTS.md — DirWalker

Guía para agentes de IA que trabajen en este repositorio.

## Qué es

CLI en **C23** (solo Linux, solo libc + glibc) que recorre `$HOME` con `nftw()`,
acumula las rutas en un vector en memoria y las imprime paginadas (lista plana o
árbol). **Solo lectura**: nunca debe crear, borrar, chmod'ear ni escribir en el
sistema de archivos. Interfaz de menú con ANSI True Color y paleta fija.

## Comandos

```bash
make                      # build/dirwalker — DEBE quedar sin warnings (-Wall -Wextra -Wpedantic)
make debug                # -g -O0 (reconstruye limpio)
make clean
gcc -std=c23 -g -O1 -fsanitize=address,undefined -D_GNU_SOURCE -Iinclude src/*.c -o /tmp/dw   # chequeo de fugas/UB
printf '1\n1\nn\n' | DIRWALKER_ROOT=/tmp/dwtest ./build/dirwalker         # flujo por tubería (stdin no-tty OK)
```

## Estructura y reglas de dependencias

| Módulo | Responsabilidad única |
|---|---|
| `src/main.c` | Detección SO + bucle del menú. Única política de alto nivel; nada de printf de formato aquí |
| `src/app.c` + `include/app.h` | Constantes (`APP_PROMPT`, `MAX_INPUT`, `NFTW_MAXFD`) y bandera cooperativa de SIGINT |
| `src/os.c` | `uname()` → `is_linux`; `getpwuid()` → raíz (`DIRWALKER_ROOT` → `pw_dir` → `$HOME`) |
| `src/ui.c` | TODO texto/formato de interfaz: banner, menú, prompt, errores, `ui_clear()`, despedida |
| `src/color.c` + `include/palette.h` | Emisión de escapes `38;2;R;G;B`; los 5 colores viven SOLO en `palette.h` |
| `src/input.c` | `read(2)` byte a byte: `input_line()` (descarta exceso) e `input_key()` |
| `src/walk.c` | `nftw(FTW_PHYS)`, callback → clasificación + sanitización → `store_add()`, `Stats` |
| `src/store.c` | Vector dinámico (`realloc` x2); `Entry = {path, len, base, level, type}` |
| `src/render.c` | Resumen, plana, árbol (pila monótona para `├──/└──`), paginación con termios |
| `src/sanitize.c` | Bytes de control → `\e` / `\xHH` (anti terminal-escape-injection) |

Dependencias **en una sola dirección**: `render` no incluye `walk` ni `<ftw.h>`;
`walk` no imprime; `ui` no sabe de nftw. No introducir ciclos.

## Convenciones no negociables

1. **C23 real**: `-std=c23`; usa `bool` palabra clave, `nullptr`, inicializadores vacíos `{}`. No incluir `<stdbool.h>`.
2. **Sin dependencias externas** ni subprocesos: solo libc/glibc y ANSI.
3. **Entrada por `read(2)`, nunca `fgets`/`getchar`**: stdio y `read` mezclarían buffers con el paginador.
4. **Propiedad de memoria**: `store_add()` se queda con `path` (la libera él si falla); `store_free()` libera todo. Ante cualquier error, liberar y restaurar termios/colores (`c_reset()`).
5. **Sanitizar al entrar al store, no al imprimir**: todo texto surgido del disco pasa por `sanitize_display()` una sola vez. Nunca imprimir bytes crudos del filesystem.
6. **Colores solo vía `color.c`**: prohibido escribir `\033[` a mano fuera de `color.c` (y `ui_clear`, que emite `\033[2J\033[H` deliberadamente aunque `NO_COLOR` active).
7. **SIGINT es cooperativo**: el handler solo pone la bandera (`app_stop_requested()`); el callback de nftw devuelve `-1` y el paginador rompe. Nunca `exit()` desde el handler.
8. **termios**: el paginador limpia `ICANON|ECHO` pero **mantiene `ISIG`** (si no, Ctrl+C muere) y restaura siempre al salir.

## Trampas concretas de este proyecto

- glibc: los miembros son `buf->base` / `buf->level`, **no** `ftw_base`/`ftw_level`.
- Con `FTW_PHYS` glibc nunca entrega `FTW_SLN`: los enlaces rotos se detectan haciendo `stat()` explícito sobre el symlink en el callback.
- `nftw()` no admite puntero de contexto → `g_ctx` estático en `walk.c` (un recorrido a la vez).
- Los headers propios necesitan `<stddef.h>` para `size_t` (C23 no lo inyecta).
- El árbol NO requiere reconstruir padres: `nftw` llega en preorden DFS; el orden del store ya es el orden de impresión.
- Con stdin por tubería `tcgetattr` falla → el paginador degrada a lectura normal; no asumir TTY.
- El menú se repinta con `ui_clear()` en cada iteración del bucle (diseño pedido): no "arreglar" eso imprimiendo sin limpiar.
- Pruebas tortura conocidas que deben seguir pasando: nombres con `\033[`, newline, BEL/TAB en el nombre; directorio `chmod 000` (cuenta "sin acceso", no aborta).

## Flujo de validación de cambios

1. `make` → cero warnings.
2. Flujos por tubería (`2`, `x\n2`, `1\n1\nn`, `1\n2\nn`, `1\n0\n2`).
3. ASan+UBSan con `DIRWALKER_ROOT` → cero fugas/UB.
4. Si tocaste presentación: verificar neutralización (`grep '\\e\\['`) y que `NO_COLOR=1` emite 0 bytes `ESC` de color.
