/*
 * walk.c — Recorrido con nftw(3) en modo FTW_PHYS (no sigue symlinks,
 * evita ciclos y duplicados). Las entradas se sanitizan al entrar al
 * store, una sola vez, y nunca se toca nada en disco: solo lectura.
 */
#include "walk.h"

#include "app.h"
#include "sanitize.h"
#include "store.h"

#include <errno.h>
#include <ftw.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef struct {
    Store *store;
    Stats *stats;
} WalkCtx;

/* nftw(3) no permite puntero de contexto: uno solo por recorrido, estático. */
static WalkCtx g_ctx;

static int walk_cb(const char *fpath, const struct stat *sb, int typeflag,
                   struct FTW *buf)
{
    (void)sb;

    if (app_stop_requested()) {
        errno = EINTR;
        return -1; /* nftw aborta; main libera el store */
    }

    EntryType type;
    switch (typeflag) {
    case FTW_F:
        type = T_FILE;
        g_ctx.stats->files++;
        break;
    case FTW_D:
        type = T_DIR;
        g_ctx.stats->dirs++;
        break;
    case FTW_DNR: /* directorio sin permiso de lectura: se cuenta, no aborta */
        type = T_DIR;
        g_ctx.stats->dirs++;
        g_ctx.stats->unreadable++;
        break;
    case FTW_SL:
        /* Con FTW_PHYS, glibc nunca da FTW_SLN: resolvemos nosotros. */
        {
            struct stat target;
            if (stat(fpath, &target) == 0) {
                type = T_LINK;
                g_ctx.stats->links++;
            } else {
                type = T_LINK_DEAD;
                g_ctx.stats->dead++;
            }
        }
        break;
    case FTW_SLN: /* enlace roto (otros ports de libc) */
        type = T_LINK_DEAD;
        g_ctx.stats->dead++;
        break;
    default:      /* FTW_NS y demás */
        type = T_OTHER;
        g_ctx.stats->other++;
        break;
    }

    size_t len = strlen(fpath);
    size_t base_out = 0;
    char *disp = sanitize_display(fpath, len, (size_t)buf->base,
                                  &base_out);
    if (disp == nullptr) {
        errno = ENOMEM;
        return -1;
    }
    if (!store_add(g_ctx.store, disp, base_out, buf->level, type)) {
        errno = ENOMEM; /* store_add ya liberó disp */
        return -1;
    }
    return 0;
}

WalkStatus walk_home(Store *store, const char *root, Stats *stats)
{
    *stats = (Stats){};
    app_reset_stop();

    g_ctx.store = store;
    g_ctx.stats = stats;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int rc = nftw(root, walk_cb, NFTW_MAXFD, FTW_PHYS);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    stats->secs = (double)(t1.tv_sec - t0.tv_sec)
                + (double)(t1.tv_nsec - t0.tv_nsec) / 1e9;

    if (rc != 0) {
        return app_stop_requested() ? WALK_CANCELLED : WALK_FAILED;
    }
    return WALK_DONE;
}
