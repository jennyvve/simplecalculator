/* De implementatie van een interpreter die een ABS boom versimpeld.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "simp.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define PI acos(0.0f) * 2.0f

typedef enum {
    SIMP_RT_OK = 0,
    SIMP_RT_NUMERICAL,  // Return value om te bepalen of een symbool
                        // verwerkt kan worden als nummer.
} simp_rt_e;

simp_rt_e simp_symbol_pi(token_t* t, double* n) {
    *n = PI;
    return SIMP_RT_NUMERICAL;
}

simp_rt_e simp_symbol_number(token_t* t, double* n) {
    *n = t->value.number;
    return SIMP_RT_NUMERICAL;
}

simp_rt_e simp_symbol_invalid(token_t* t, double* n) {
    return SIMP_RT_OK;
}

typedef simp_rt_e (*simp_symbol_t)(token_t* t, double* n);

// Map token types aan de numerieke waarde. Wanneer een token geen
// numerieke waarde heeft wordt SIMP_RT_OK terug gegeven, wanneer een
// token wel een numerieke waarde heeft wordt SIMP_RT_NUMERICAL terug
// gegeven en wordt 'c' de waarde van de token.
simp_symbol_t simp_map_symbol[] = {
    [0 ... TOKEN_TYPE_INVALID] = simp_symbol_invalid,
    [TOKEN_TYPE_PI] = simp_symbol_pi,
    [TOKEN_TYPE_NUMBER] = simp_symbol_number,
};

// Lees twee numerieke waardes uit twee tokens. Plaats de waardes in
// de pointers, slaat rechts over wanneer de rechtse token een NULL
// pointer is.
static inline simp_rt_e simp_op_get_numerical(token_t* left,
                                              token_t* right,
                                              double* value_left,
                                              double* value_right) {
    simp_rt_e rt = SIMP_RT_OK;

    rt = simp_map_symbol[left->type](left, value_left);
    if (rt == SIMP_RT_NUMERICAL && right) {
        rt = simp_map_symbol[right->type](right, value_right);
    }

    return rt;
}

void simp_op_plus(tree_t* t) {
    double vl, vr;  // numerieke waarde van de linkse en rechtse node.

    if (t->left == NULL || t->right == NULL) {
        return;
    }

    if (token_cmp_number(&t->left->token, 0.0f)) {  // 0 + expr = expr
        tree_move_node(t, t->right);
    } else if (token_cmp_number(&t->right->token,
                                0.0f)) {  // expr + 0 = expr
        tree_move_node(t, t->left);
    } else if (simp_op_get_numerical(&t->left->token,
                                     &t->right->token, &vl,
                                     &vr) == SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, vl + vr);
        t->left = t->right = NULL;
    }
}

void simp_op_minus(tree_t* t) {
    double vl, vr;  // numerieke waarde van de linkse en rechtse node.

    if (t->left == NULL || t->right == NULL) {
        return;
    }

    if (token_cmp_number(&t->left->token, 0.0f)) {  // 0 - expr = expr
        token_make_type(&t->token, TOKEN_TYPE_MULTIPLY);
        token_make_number(&t->left->token, -1.0f);
    } else if (token_cmp_number(&t->right->token,
                                0.0f)) {  // expr - 0 = expr
        tree_move_node(t, t->left);
    } else if (token_cmp_variable(
                   &t->left->token,
                   &t->right->token)) {  // expr - expr = 0
        token_make_number(&t->token, 0.0f);
        t->left = t->right = NULL;
    } else if (simp_op_get_numerical(&t->left->token,
                                     &t->right->token, &vl,
                                     &vr) == SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, vl - vr);
        t->left = t->right = NULL;
    }
}

void simp_op_multiply(tree_t* t) {
    double vl, vr;  // numerieke waarde van de linkse en rechtse node.

    if (t->left == NULL || t->right == NULL) {
        return;
    }

    if (token_cmp_number(&t->left->token,
                         1.0f)) {  // expr * 1 = expr
        tree_move_node(t, t->right);
    } else if (token_cmp_number(&t->right->token,
                                1.0f)) {  // 1 * expr = expr
        tree_move_node(t, t->left);
    } else if (token_cmp_number(&t->left->token, 0.0f) ||
               token_cmp_number(&t->right->token,
                                0.0f)) {  // 0 * expr = expr * 0 = 0
        token_make_number(&t->token, 0.0f);
        t->left = t->right = NULL;
    } else if (simp_op_get_numerical(&t->left->token,
                                     &t->right->token, &vl,
                                     &vr) == SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, vl * vr);
        t->left = t->right = NULL;
    }
}

void simp_op_divide(tree_t* t) {
    double vl, vr;  // numerieke waarde van de linkse en rechtse node.

    if (t->left == NULL || t->right == NULL) {
        return;
    }

    if (token_cmp_number(&t->right->token, 0.0f)) {  // expr / 0 = err
        printf("!ERR Division by 0.\n");
    } else if (token_cmp_variable(
                   &t->left->token,
                   &t->right->token)) {  // expr / 1 = expr
        token_make_number(&t->token, 1.0f);
        t->left = t->right = NULL;
    } else if (simp_op_get_numerical(&t->left->token,
                                     &t->right->token, &vl,
                                     &vr) == SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, vl / vr);
        t->left = t->right = NULL;
    }
}

void simp_op_power(tree_t* t) {
    double vl, vr;  // numerieke waarde van de linkse en rechtse node.

    if (t->left == NULL || t->right == NULL) {
        return;
    }

    if (token_cmp_number(&t->right->token,
                         1.0f)) {  // expr ^ 1 = expr
        tree_move_node(t, t->left);
    } else if (token_cmp_number(&t->left->token,
                                0.0f)) {  // 0 ^ expr = 0
        token_make_number(&t->token, 0.0f);
        t->right = t->left = NULL;
    } else if (token_cmp_number(&t->right->token,
                                0.0f)) {  // expr ^ 0 = 1
        token_make_number(&t->token, 1.0f);
        t->right = t->left = NULL;
    } else if (simp_op_get_numerical(&t->left->token,
                                     &t->right->token, &vl,
                                     &vr) == SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, pow(vl, vr));
        t->right = t->left = NULL;
    }
}

void simp_op_sin(tree_t* t) {
    double vl;  // numerieke waarde van de linkse node.

    if (simp_op_get_numerical(&t->left->token, NULL, &vl, NULL) ==
        SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, sin(vl));
        t->left = NULL;
    }
}

void simp_op_cos(tree_t* t) {
    double vl;  // numerieke waarde van de linkse node.

    if (simp_op_get_numerical(&t->left->token, NULL, &vl, NULL) ==
        SIMP_RT_NUMERICAL) {
        token_make_number(&t->token, cos(vl));
        t->left = NULL;
    }
}

void simp_op_invalid(tree_t* t) {}

typedef void (*simp_op_t)(tree_t* t);

/* Map operaties aan een token type, wanneer een token geen operatie
 * is gebeurd er niets. De operatie functie calls managen de volledige
 * subboom, het is dus mogelijk dat een operatie de nodes binnen een
 * subboom aanpast, verwijderd, of toevoegd.
 */
simp_op_t simp_map_op[] = {
    [0 ... TOKEN_TYPE_INVALID] = simp_op_invalid,
    [TOKEN_TYPE_PLUS] = simp_op_plus,
    [TOKEN_TYPE_MINUS] = simp_op_minus,
    [TOKEN_TYPE_MULTIPLY] = simp_op_multiply,
    [TOKEN_TYPE_DIVIDE] = simp_op_divide,
    [TOKEN_TYPE_POWER] = simp_op_power,
    [TOKEN_TYPE_SIN] = simp_op_sin,
    [TOKEN_TYPE_COS] = simp_op_cos,
};

void simp_tree(tree_t* tree) {
    if (tree == NULL) {
        return;
    }

    simp_tree(tree->left);
    simp_tree(tree->right);

    /* Zet binnen de operaties de takken van de node gewoon op NULL,
     * deze worden gedealloceerd in de arena. */
    simp_map_op[tree->token.type](tree);
}
