/* Header van tree_t. Maakt gebruik van een arena implementatie
 * om de data in ruimtelijke lokaliteit bij elkaar te houden.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __TREE_H
#define __TREE_H

#include <stddef.h>

#include "token.h"

typedef enum {
    TREE_ARENA_ERR_NONE = 0,    // Geen errors.
    TREE_ARENA_ERR_OVERFILLED,  // De arena is vol.
} tree_arena_err_e;

typedef struct TREE_T tree_t;
struct TREE_T {
    token_t token;
    tree_t *left;
    tree_t *right;
};

typedef void *tree_arena_handle_t;

// Maak een nieuwe node binnen de page van de arena, wanneer de arena
// vol is wordt er een dummy gereturned. De fout kan opgevangen worden
// met tree_arena_get_err(), zet TREE_ARENA_ERR_OVERFILLED. De arena
// moet met tree_arena_clear() gereset worden om weer functioneel te
// worden.
tree_t *tree_arena_new_node(tree_arena_handle_t const *const handle);

tree_t *tree_arena_remove_node(
    tree_arena_handle_t const *const handle, tree_t *tree);

// Neem een dummy uit de arena, de dummy wordt gedealloceerd samen met
// het free'n van de arena.
tree_t *tree_arena_get_dummy(tree_arena_handle_t const *const handle);

// Lees de error flag van een arena, de error flag wordt gereset door
// tree_arena_clear().
tree_arena_err_e tree_arena_get_err(
    tree_arena_handle_t const *const handle);

// Maak de hele arena leeg, let op dat bestaande pointers naar nodes
// blijven bestaan. Deze zullen echter wijzen naar nodes die leeg zijn
// of mogelijk na een clear opnieuw worden vrijgegeven.
void tree_arena_clear(tree_arena_handle_t const *const handle);

const tree_arena_handle_t *const tree_arena_malloc(void);
void tree_arena_free(tree_arena_handle_t const *const handle);

// Kopieer subboom naar een andere subboom.
static inline tree_t *tree_deepcopy_sub(
    tree_arena_handle_t const *const h, tree_t const *const src) {
    tree_t *r = tree_arena_new_node(h);
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return r;
    }

    token_copy(&r->token, &src->token);

    if (src->left != NULL) {
        r->left = tree_deepcopy_sub(h, src->left);
    }

    if (src->right != NULL) {
        r->right = tree_deepcopy_sub(h, src->right);
    }

    return r;
}

// Verplaatst de src node naar de trg node. De src node raakt verloren
// in de subboom.
static inline void tree_move_node(tree_t *trg,
                                  tree_t const *const src) {
    token_copy(&trg->token, &src->token);
    trg->left = src->left;
    trg->right = src->right;
}

// Vervang alle variabele tokens met naam x met de waarde value.
static inline void tree_substitute_x(tree_t *tree, double value) {
    if (tree == NULL) {
        return;
    }

    if (tree->token.type == TOKEN_TYPE_VARIABLE &&
        tree->token.value.variable == 'x') {
        tree->token.value.number = value;
        tree->token.type = TOKEN_TYPE_NUMBER;
    }

    tree_substitute_x(tree->left, value);
    tree_substitute_x(tree->right, value);
}

#endif  // __TREE_H
