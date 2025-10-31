/* Implementatie van tree_t.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "tree.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bereken de ouder van een kind met de pointer naar het kind.
#define OFFSET_OF(type, member) ((size_t) & ((type*)0)->member)
#define CONTAINER_OF(ptr, type, member)                      \
    ({                                                       \
        const __typeof__(((type*)0)->member)* _mptr = (ptr); \
        (type*)((char*)_mptr - OFFSET_OF(type, member));     \
    })

// Formaat van tree_arena_t is ongeveer 32KiB, zodat het in
// de L1 cache past van de meeste moderne CPU's.
#define TREE_ARENA_SIZE 768
typedef struct {
    tree_arena_handle_t h;
    tree_t d[TREE_ARENA_SIZE + 1];
    int di;
    int f[TREE_ARENA_SIZE + 1];
    int fi;
    tree_arena_err_e err;
} tree_arena_t;

tree_t* tree_arena_remove_node(
    tree_arena_handle_t const* const handle, tree_t* tree) {
    tree_arena_t* t = CONTAINER_OF(handle, tree_arena_t, h);
    int i = tree - t->d;

    if (i < 0 || i >= t->di) {
        return tree;
    }

    t->f[t->fi++] = i;
    memset(&t->d[i], 0, sizeof(tree_t));
    return NULL;
}

tree_t* tree_arena_get_dummy(
    tree_arena_handle_t const* const handle) {
    tree_arena_t* t = CONTAINER_OF(handle, tree_arena_t, h);
    return &t->d[TREE_ARENA_SIZE];
}

tree_t* tree_arena_new_node(tree_arena_handle_t const* const handle) {
    tree_arena_t* t = CONTAINER_OF(handle, tree_arena_t, h);

    if (t->err != TREE_ARENA_ERR_NONE || t->di >= TREE_ARENA_SIZE) {
        t->err = TREE_ARENA_ERR_OVERFILLED;
        return tree_arena_get_dummy(handle);
    }

    if (t->fi > 0) {
        return &t->d[t->f[t->fi--]];
    }

    return &t->d[t->di++];
}

tree_arena_err_e tree_arena_get_err(
    tree_arena_handle_t const* const handle) {
    return CONTAINER_OF(handle, tree_arena_t, h)->err;
}

void tree_arena_clear(tree_arena_handle_t const* const handle) {
    tree_arena_t* t = CONTAINER_OF(handle, tree_arena_t, h);
    t->fi = 0;
    t->di = 0;
    t->err = TREE_ARENA_ERR_NONE;
    memset(&t->d, 0, sizeof(tree_t) * TREE_ARENA_SIZE);
}

const tree_arena_handle_t* tree_arena_malloc(void) {
    tree_arena_t* t = calloc(1, sizeof(tree_arena_t));
    return &t->h;
}

void tree_arena_free(tree_arena_handle_t const* const handle) {
    tree_arena_t* t = CONTAINER_OF(handle, tree_arena_t, h);
    free(t);
}
