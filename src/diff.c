/* Implementatie die een ABS boom diffirentieert. De volgende regels
 * zijn geimplementeerd/differientieerbaar:
 *  - constante
 *  - machtsverheffing
 *  - productregel
 *  - quotientregel
 *  - sin / cos
 *  - somregel
 *
 * De regels worden enkel toegepast op de variable 'x'.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "diff.h"

#include "simp.h"

typedef tree_t* (*diff_op_t)(tree_t const* const t,
                             tree_arena_handle_t const* const h);

diff_op_t diff_map_op[TOKEN_TYPE_INVALID + 1];

tree_t* diff_op_product(tree_t const* const t,
                        tree_arena_handle_t const* const h) {
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* f = tree_deepcopy_sub(h, t->left);
    tree_t* df = diff_map_op[t->left->token.type](t->left, h);
    tree_t* g = tree_deepcopy_sub(h, t->right);
    tree_t* dg = diff_map_op[t->right->token.type](t->right, h);

    tree_t* r = tree_arena_new_node(h);  // root
    r->left = tree_arena_new_node(h);
    r->right = tree_arena_new_node(h);
    r->left->left = f;
    r->left->right = dg;
    r->right->left = g;
    r->right->right = df;
    token_make_type(&r->token, TOKEN_TYPE_PLUS);
    token_make_type(&r->left->token, TOKEN_TYPE_MULTIPLY);
    token_make_type(&r->right->token, TOKEN_TYPE_MULTIPLY);

    return r;
}

tree_t* diff_op_sum(tree_t const* const t,
                    tree_arena_handle_t const* const h) {
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* r = tree_arena_new_node(h);  // root
    token_copy(&r->token, &t->token);
    r->left = diff_map_op[t->left->token.type](t->left, h);
    r->right = diff_map_op[t->right->token.type](t->right, h);

    return r;
}

tree_t* diff_op_variable(tree_t const* const t,
                         tree_arena_handle_t const* const h) {
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* r = tree_arena_new_node(h);  // root

    double v = (t->token.value.variable == 'x') ? 1.0f : 0.0f;
    token_make_number(&r->token, v);

    return r;
}

tree_t* diff_op_quotient(tree_t const* const t,
                         tree_arena_handle_t const* const h) {
    // doel: / f g -> / - * f' g - f g' ^ g 2
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* f = tree_deepcopy_sub(h, t->left);
    tree_t* df = diff_map_op[t->left->token.type](t->left, h);
    tree_t* g = tree_deepcopy_sub(h, t->right);
    tree_t* dg = diff_map_op[t->right->token.type](t->right, h);

    tree_t* d = tree_arena_new_node(h);  // denominator/noemer
    d->left = tree_deepcopy_sub(h, t->right);
    d->right = tree_arena_new_node(h);
    token_make_type(&d->token, TOKEN_TYPE_POWER);
    token_make_number(&d->right->token, 2.0f);

    tree_t* n = tree_arena_new_node(h);  // numerator/teller
    n->left = tree_arena_new_node(h);
    n->right = tree_arena_new_node(h);
    n->left->left = df;
    n->left->right = g;
    n->right->left = f;
    n->right->right = dg;
    token_make_type(&n->token, TOKEN_TYPE_MINUS);
    token_make_type(&n->left->token, TOKEN_TYPE_MULTIPLY);
    token_make_type(&n->right->token, TOKEN_TYPE_MULTIPLY);

    tree_t* r = tree_arena_new_node(h);  // doel
    r->left = n;
    r->right = d;
    token_make_type(&r->token, TOKEN_TYPE_DIVIDE);

    return r;
}

tree_t* diff_op_constant(tree_t const* const t,
                         tree_arena_handle_t const* const h) {
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* r = tree_arena_new_node(h);  // root
    token_make_number(&r->token, 0.0f);
    return r;
}

tree_t* diff_op_power(tree_t const* const t,
                      tree_arena_handle_t const* const h) {
    // doel: ^ f(g) num ->  * * num ^ f'(g) num g'
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* df = tree_arena_new_node(h);  // f'(g)
    tree_t* dg = diff_map_op[t->left->token.type](t->left, h);  // g'

    tree_t* n = tree_arena_new_node(h);   // x^n -> n * x ^ (m)
    tree_t* e = tree_deepcopy_sub(h, t);  // original expression

    // f' = ^ e num -> * num ^ e (num-1)
    token_make_type(&df->token, TOKEN_TYPE_MULTIPLY);  // *_1
    df->left = n;
    df->right = e;
    token_make_number(&n->token, t->right->token.value.number);
    token_make_number(&e->right->token,
                      (e->right->token.value.number - 1.0f));

    tree_t* r = tree_arena_new_node(h);  // doel
    r->left = df;
    r->right = dg;
    token_make_type(&r->token, TOKEN_TYPE_MULTIPLY);  // * f'(g) g'

    return r;
}

tree_t* diff_op_sin(tree_t const* const t,
                    tree_arena_handle_t const* const h) {
    // doel: sin(f) -> cos(f) * f'
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* dsin = tree_deepcopy_sub(h, t);
    token_make_type(&dsin->token, TOKEN_TYPE_COS);

    tree_t* df = diff_map_op[t->left->token.type](t->left, h);

    tree_t* r = tree_arena_new_node(h);  // doel
    r->left = dsin;
    r->right = df;
    token_make_type(&r->token, TOKEN_TYPE_MULTIPLY);

    return r;
}

tree_t* diff_op_cos(tree_t const* const t,
                    tree_arena_handle_t const* const h) {
    // doel: cos (f) -> -sin(f) * f'
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return tree_arena_get_dummy(h);
    }

    tree_t* sin = tree_deepcopy_sub(h, t);  // cos_f = sin
    token_make_type(&sin->token, TOKEN_TYPE_SIN);

    tree_t* dcos = tree_arena_new_node(h);  // -sin
    dcos->left = tree_arena_new_node(h);
    dcos->right = sin;
    token_make_type(&dcos->token, TOKEN_TYPE_MULTIPLY);
    token_make_number(&dcos->left->token, -1.0f);

    tree_t* df = diff_map_op[t->left->token.type](t->left, h);

    tree_t* r = tree_arena_new_node(h);  // doel
    r->left = dcos;
    r->right = df;
    token_make_type(&r->token, TOKEN_TYPE_MULTIPLY);

    return r;
}

tree_t* diff_op_invalid(tree_t const* const t,
                        tree_arena_handle_t const* const h) {
    return tree_arena_get_dummy(h);
}

diff_op_t diff_map_op[] = {
    [0 ... TOKEN_TYPE_INVALID] = diff_op_invalid,
    [TOKEN_TYPE_VARIABLE] = diff_op_variable,
    [TOKEN_TYPE_NUMBER] = diff_op_constant,
    [TOKEN_TYPE_PLUS] = diff_op_sum,
    [TOKEN_TYPE_MINUS] = diff_op_sum,
    [TOKEN_TYPE_COS] = diff_op_cos,
    [TOKEN_TYPE_SIN] = diff_op_sin,
    [TOKEN_TYPE_MULTIPLY] = diff_op_product,
    [TOKEN_TYPE_DIVIDE] = diff_op_quotient,
    [TOKEN_TYPE_POWER] = diff_op_power,
    [TOKEN_TYPE_PI] = diff_op_constant,
};

tree_t* diff_tree(tree_t* t, tree_arena_handle_t const* const h) {
    if (t == NULL || h == NULL) {
        return NULL;
    }

    simp_tree(t);

    // De implementatie maakt gebruik van de arena error handling om
    // te bepalen of alle nieuwe nodes binnen de page van de arena
    // vallen. Hierdoor hoeven we niet steeds te kijken of een node
    // correct is aangemaakt. Je zult door de implementatie heen zien:
    //
    //    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
    //        return tree_arena_get_dummy(h);
    //    }
    //
    // De functie returned dus gewoon een dummy node wanneer de arena
    // vol is, hierdoor krijgen we geen segfaults en gaat het
    // programma gewoon door. Is wat efficienter dan constant alle
    // node pointers te moeten matchen met NULL.
    tree_t* r = diff_map_op[t->token.type](t, h);
    if (tree_arena_get_err(h) != TREE_ARENA_ERR_NONE) {
        return NULL;
    }

    simp_tree(r);

    return r;
}
