# DirWalker

Explorador de rutas de solo lectura para terminal (C23, Linux): recorre el `$HOME`
del usuario con `nftw()` en modo `FTW_PHYS`, guarda todas las rutas en memoria y las
imprime como **lista plana** o **árbol jerárquico** paginado, con interfaz a color
True Color y sanitización anti-inyección ANSI.

```
 >>>> [1] Iniciar recorrido      >>>> 1
      [2] Salir                  ───── RESUMEN ─────
                                    Directorios  ....  4  ·  Archivos  ....  6 ...
```

## Requisitos

| Requisito | Detalle |
|---|---|
| Sistema | Linux (el programa se autodetecta y rechaza otros SO) |
| Compilador | GCC ≥ 14 con `-std=c23` |
| Dependencias | Ninguna: solo libc/glibc (ftw, termios, pwd, utsname) |

## Compilación

```bash
make          # build/dirwalker (-std=c23 -O2, sin warnings con -Wall -Wextra -Wpedantic)
make run      # compila y ejecuta
make debug    # reconstruye con -g -O0
make clean    # borra build/
```

## Uso

```bash
./build/dirwalker
```

| Entrada | Efecto |
|---|---|
| `1` en el menú | Recorre `$HOME` (incluye ocultos, no sigue symlinks) → resumen de conteos y tiempo |
| `1` / `2` tras el resumen | Imprime lista plana / árbol jerárquico |
| `0` | Vuelve al menú sin imprimir |
| `ENTER` | Página siguiente durante la impresión |
| `q` | Termina la lista antes del final |
| `s` / `n` | Confirmación de volver al menú o salir |
| `Ctrl+C` | Cancela recorrido o impresión: libera memoria, terminal intacta, vuelve al menú |
| `2` / `Ctrl+D` | Despedida y salida (status 0) |

> [!NOTE]
> `DIRWALKER_ROOT=<ruta>` cambia la raíz del recorrido (usado en pruebas);
> `NO_COLOR=1` desactiva los escapes de color, no la limpieza de pantalla.

## Características

- **Solo lectura**: el programa jamás crea, edita ni borra archivos.
- **Sanitización**: los nombres del disco pasan por `sanitize_display()`; un archivo
  llamado literalmente `\033[31m...` se imprime como texto inerte (`\e[31m...`),
  neutralizando *terminal escape injection*.
- **Verdadero estado de symlinks**: con `FTW_PHYS` los enlaces rotos se detectan
  mediante `stat()` explícito.
- **Menú siempre limpio**: la terminal se limpia (`\033[2J\033[H`) al arrancar y en
  cada repintado del menú.

## Interfaz y paleta

True Color (`\033[38;2;R;G;Bm`), definido solo en `include/palette.h`:

| Rol | Color | Hex |
|---|---|---|
| Banner, acentos | Jungle Green | `#44af69` |
| Errores, salir, enlaces rotos | Strawberry Red | `#f8333c` |
| Etiquetas de opción, symlinks, advertencias | Orange | `#fcab10` |
| Prompt `>>>>`, directorios, separadores | Pacific Blue | `#2b9eb3` |
| Texto neutro y archivos | Sand Dune | `#dbd5b5` |

## Arquitectura

```
include/ y src/ — un módulo, una responsabilidad
  main.c      bucle del menú y orquestación (única política de alto nivel)
  app.c/h     constantes globales + bandera SIGINT cooperativa
  os.c/h      uname() → ¿Linux? · getpwuid() → $HOME (respaldo: DIRWALKER_ROOT, HOME)
  ui.c/h      banner, menú, prompt, mensajes, despedida, limpieza de pantalla
  color.c/h   emisión de escapes ANSI (no-ops si NO_COLOR)
  palette.h   los 5 colores RGB (fuente única)
  input.c/h   lectura por read(2): input_line() e input_key()
  walk.c/h    nftw(FTW_PHYS) → callback → store + Stats
  store.c/h   vector dinámico de Entradas (dueño de las cadenas sanitizadas)
  render.c/h  resumen, vista plana, vista árbol, paginación (termios raw)
  sanitize.c/h  bytes de control → marcadores visibles
```

Dependencias permitidas (sin ciclos): `main → todos`; `render → store, ui, color, input, app`;
`walk → store, sanitize, app`; `ui → color, input, os`. `render` **no** conoce `walk`
ni el sistema de archivos; `walk` **no** imprime nada.

## Pruebas rápidas

```bash
# flujos por tubería (el programa tolera stdin no-tty)
printf '2\n'            | ./build/dirwalker          # salir con despedida
printf 'x\n2\n'         | ./build/dirwalker          # opción inválida → repinta
printf '1\n1\nn\n'      | ./build/dirwalker          # recorrer → plana → salir
printf '1\n2\nn\n'      | ./build/dirwalker          # recorrer → árbol → salir

# sin fugas ni UB
gcc -std=c23 -g -O1 -fsanitize=address,undefined -D_GNU_SOURCE -Iinclude src/*.c -o /tmp/dw-asan
printf '1\n2\nn\n' | DIRWALKER_ROOT=/tmp ./tmp/dw-asan
```

## Licencia

Pendiente de definir.
