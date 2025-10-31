/* Implementatie voor token_t.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "token.h"

#include <stdio.h>
#include <string.h>

token_cat_e token_type_to_cat[] = {
    [0 ... TOKEN_TYPE_INVALID] = TOKEN_CAT_INVALID,
    [TOKEN_TYPE_MINUS] = TOKEN_CAT_OP_BINAIR,
    [TOKEN_TYPE_PLUS] = TOKEN_CAT_OP_BINAIR,
    [TOKEN_TYPE_MULTIPLY] = TOKEN_CAT_OP_BINAIR,
    [TOKEN_TYPE_DIVIDE] = TOKEN_CAT_OP_BINAIR,
    [TOKEN_TYPE_SIN] = TOKEN_CAT_OP_UNAIR,
    [TOKEN_TYPE_COS] = TOKEN_CAT_OP_UNAIR,
    [TOKEN_TYPE_POWER] = TOKEN_CAT_OP_BINAIR,
    [TOKEN_TYPE_NUMBER] = TOKEN_CAT_SYMBOL,
    [TOKEN_TYPE_VARIABLE] = TOKEN_CAT_SYMBOL,
    [TOKEN_TYPE_PI] = TOKEN_CAT_SYMBOL,
};

int token_type_to_string_lenghts[] = {
    [0 ... TOKEN_TYPE_INVALID] = 0,
    [TOKEN_TYPE_MINUS] = 1,
    [TOKEN_TYPE_PLUS] = 1,
    [TOKEN_TYPE_MULTIPLY] = 1,
    [TOKEN_TYPE_DIVIDE] = 1,
    [TOKEN_TYPE_SIN] = 3,
    [TOKEN_TYPE_COS] = 3,
    [TOKEN_TYPE_POWER] = 1,
    [TOKEN_TYPE_NUMBER] = 0,
    [TOKEN_TYPE_VARIABLE] = 0,
    [TOKEN_TYPE_PI] = 2,
};

char token_type_to_string[][TOKEN_STRING_LENGTH] = {
    [0 ... TOKEN_TYPE_INVALID] = "", [TOKEN_TYPE_MINUS] = "-",
    [TOKEN_TYPE_PLUS] = "+",         [TOKEN_TYPE_MULTIPLY] = "*",
    [TOKEN_TYPE_DIVIDE] = "/",       [TOKEN_TYPE_SIN] = "sin",
    [TOKEN_TYPE_COS] = "cos",        [TOKEN_TYPE_POWER] = "^",
    [TOKEN_TYPE_NUMBER] = "",        [TOKEN_TYPE_VARIABLE] = "",
    [TOKEN_TYPE_PI] = "pi",
};

token_cat_e token_get_cat(token_t const* const token) {
    return token_type_to_cat[token->type];
}

void token_string(token_t const* const token, token_string_t string) {
    if (token->type == TOKEN_TYPE_VARIABLE) {
        string[0] = token->value.variable;
    } else if (token->type == TOKEN_TYPE_NUMBER) {
        sprintf(string, "%g", token->value.number);
    } else {
        memcpy(string, token_type_to_string[token->type],
               token_type_to_string_lenghts[token->type]);
    }
}
