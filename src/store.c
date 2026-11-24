/*
 * store.c — Vector dinámico de entradas con crecimiento exponencial.
 */
#include "store.h"

#include <stdlib.h>
#include <string.h>

#define STORE_INITIAL_CAP 1024u

bool store_init(Store *st)
{
    st->items = malloc(STORE_INITIAL_CAP * sizeof(Entry));
    if (st->items == nullptr) {
        st->count = 0;
        st->cap = 0;
        return false;
    }
    st->count = 0;
    st->cap = STORE_INITIAL_CAP;
    return true;
}

bool store_add(Store *st, char *path, size_t base, int level, EntryType type)
{
    if (st->count == st->cap) {
        size_t newcap = st->cap * 2u;
        Entry *tmp = realloc(st->items, newcap * sizeof(Entry));
        if (tmp == nullptr) {
            free(path);
            return false;
        }
        st->items = tmp;
        st->cap = newcap;
    }

    Entry *e = &st->items[st->count++];
    e->path = path;
    e->len = strlen(path);
    e->base = (base <= e->len) ? base : 0u;
    e->level = (level > 65535) ? 65535u : (unsigned short)level;
    e->type = type;
    return true;
}

void store_free(Store *st)
{
    for (size_t i = 0; i < st->count; i++) {
        free(st->items[i].path);
    }
    free(st->items);
    st->items = nullptr;
    st->count = 0;
    st->cap = 0;
}

const char *entry_type_tag(EntryType t)
{
    switch (t) {
    case T_DIR:       return "[D] ";
    case T_FILE:      return "[F] ";
    case T_LINK:      return "[L] ";
    case T_LINK_DEAD: return "[X] ";
    case T_OTHER:     return "[?] ";
    }
    return "[?] ";
}
