/*
 * os.c — Detección del sistema operativo (uname) y raíz del recorrido.
 */
#include "os.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

bool os_detect(OsInfo *info)
{
    info->is_linux = false;
    info->sysname[0] = '\0';

    struct utsname u;
    if (uname(&u) != 0) {
        return false;
    }

    snprintf(info->sysname, sizeof(info->sysname), "%s", u.sysname);
    info->is_linux = (strcmp(u.sysname, "Linux") == 0);
    return true;
}

bool os_home_path(char *buf, size_t size)
{
    /*
     * DIRWALKER_ROOT: respaldo para pruebas y recorridos alternativos.
     * En uso normal siempre se resuelve el home real del usuario.
     */
    const char *root = getenv("DIRWALKER_ROOT");
    if (root == nullptr || root[0] == '\0') {
        struct passwd *pw = getpwuid(getuid());
        if (pw != nullptr && pw->pw_dir != nullptr && pw->pw_dir[0] != '\0') {
            root = pw->pw_dir;
        } else {
            root = getenv("HOME");
        }
    }
    if (root == nullptr || root[0] == '\0') {
        return false;
    }
    if (snprintf(buf, size, "%s", root) >= (int)size) {
        return false;
    }
    return true;
}
