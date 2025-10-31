/* De implementatie voor een parser die een string omzet naar een
 * expressie boom.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "parser.h"

#include <stdbool.h>
#include <stdlib.h>

#include "ascii.h"

#define SYMBOL_LENGTH_SIN 3
#define SYMBOL_LENGTH_COS 3
#define SYMBOL_LENGTH_PI 2

token_type_e parser_char_token_type_translator[ASCII_SYMBOLS] = {
    [0 ...(ASCII_SYMBOLS - 1)] = TOKEN_TYPE_INVALID,
    ['*'] = TOKEN_TYPE_MULTIPLY,
    ['+'] = TOKEN_TYPE_PLUS,
    ['-'] = TOKEN_TYPE_MINUS,
    ['/'] = TOKEN_TYPE_DIVIDE,
    ['^'] = TOKEN_TYPE_POWER,
};

void parser_read_int(parser_buf_t* buf, int* n, int* length) {
    while (ascii_char_is_number[(int)*(buf->p)]) {
        *n = *n * 10 + (*(buf->p) - 48);
        (*length)++;
        buf->p++;
    }
}

parser_rt_e parser_read_double(parser_buf_t* buf, double* n) {
    int c = 0;     // characteristic
    int m = 0;     // mantissa
    int ml = 0;    // mantissa length
    double p = 1;  // polarity of n

    if (*(buf->p) == '-') {
        p = -1;
        buf->p++;
    }

    parser_read_int(buf, &c, ({
                        int l;
                        &l;
                    }));
    if (*(buf->p) == '.' || *(buf->p) == ',') {
        buf->p++;
        parser_read_int(buf, &m, &ml);
        if (ml == 0) {
            return PARSER_RT_INVALID_CHAR;
        }
    }

    // verplaats de getallan van de mantissa in een double.
    double md = (double)m;  // mantissa cast als double
    while (ml--) {
        md /= 10.0;
    }

    *n = ((double)c + md) * p;
    return PARSER_RT_OK;
}

parser_rt_e parser_token_number(parser_buf_t* buf, token_t* token) {
    double n;  // waarde van de token.

    if (buf->p[0] == '-' && !ascii_char_is_number[(int)buf->p[1]]) {
        token_make_type(token, TOKEN_TYPE_MINUS);
        buf->p++;
        return PARSER_RT_OK;
    }

    parser_rt_e rt = parser_read_double(buf, &n);
    if (rt != PARSER_RT_OK) {
        return rt;
    }

    token_make_number(token, n);
    return rt;
}

parser_rt_e parser_token_text(parser_buf_t* buf, token_t* token) {
    if (buf->p[0] == 's' && buf->p[1] == 'i' && buf->p[2] == 'n') {
        token_make_type(token, TOKEN_TYPE_SIN);
        buf->p += SYMBOL_LENGTH_SIN;
    } else if (buf->p[0] == 'c' && buf->p[1] == 'o' &&
               buf->p[2] == 's') {
        token_make_type(token, TOKEN_TYPE_COS);
        buf->p += SYMBOL_LENGTH_COS;
    } else if (buf->p[0] == 'p' && buf->p[1] == 'i') {
        token_make_type(token, TOKEN_TYPE_PI);
        buf->p += SYMBOL_LENGTH_PI;
    } else {
        token_make_variable(token, *(buf->p));
        buf->p++;
    }

    return PARSER_RT_OK;
}

parser_rt_e parser_token_single(parser_buf_t* buf, token_t* token) {
    token_make_type(
        token, parser_char_token_type_translator[(int)*(buf->p)]);
    buf->p++;
    return PARSER_RT_OK;
}

parser_rt_e parser_token_invalid(parser_buf_t* buf, token_t* token) {
    return PARSER_RT_INVALID_CHAR;
}

typedef parser_rt_e (*token_parser_t)(parser_buf_t* buf,
                                      token_t* token);

token_parser_t parser_token[ASCII_SYMBOLS] = {
    [0 ...(ASCII_SYMBOLS - 1)] = parser_token_invalid,
    ['*'] = parser_token_single,
    ['+'] = parser_token_single,
    ['-'] = parser_token_number,
    ['/'] = parser_token_single,
    ['0' ... '9'] = parser_token_number,
    ['A' ... 'Z'] = parser_token_text,
    ['^'] = parser_token_single,
    ['a' ... 'z'] = parser_token_text,
};

int parser_cat_leaf_balance_value[] = {
    [0 ...(TOKEN_CAT_INVALID)] = 0,
    [TOKEN_CAT_SYMBOL] = -1,
    [TOKEN_CAT_OP_UNAIR] = 1,
    [TOKEN_CAT_OP_BINAIR] = 2,
};

parser_rt_e _parser_tokenize_string(
    tree_arena_handle_t const* const thandle, parser_buf_t* buf,
    tree_t* node, int* leaf_balance) {
    if (tree_arena_get_err(thandle) != TREE_ARENA_ERR_NONE) {
        return PARSER_RT_ERR;
    }

    while (ascii_char_is_whitespace[(int)*(buf->p)]) {
        buf->p++;
    };

    if (*(buf->p) == '\0') {
        return PARSER_RT_END;
    }

    if (parser_token[(int)*(buf->p)](buf, &node->token) !=
        PARSER_RT_OK) {
        return PARSER_RT_INVALID_CHAR;
    }

    if (token_is_operation(&node->token)) {
        node->left = tree_arena_new_node(thandle);
        parser_rt_e rt = _parser_tokenize_string(
            thandle, buf, node->left, leaf_balance);
        if (rt != PARSER_RT_OK) {
            node->left = tree_arena_remove_node(thandle, node->left);
        }
        if (rt == PARSER_RT_INVALID_CHAR) {
            return rt;
        }
        if (node->left && token_is_operation(&node->left->token)) {
            (*leaf_balance)--;
        }
    }

    if (token_is_operation_binairy(&node->token)) {
        node->right = tree_arena_new_node(thandle);
        parser_rt_e rt = _parser_tokenize_string(
            thandle, buf, node->right, leaf_balance);
        if (rt != PARSER_RT_OK) {
            node->right =
                tree_arena_remove_node(thandle, node->right);
        }
        if (rt == PARSER_RT_INVALID_CHAR) {
            return rt;
        }
        if (node->right && token_is_operation(&node->right->token)) {
            (*leaf_balance)--;
        }
    }

    *leaf_balance +=
        parser_cat_leaf_balance_value[token_get_cat(&node->token)];

    return PARSER_RT_OK;
}

parser_rt_e parser_tokenize_string(
    tree_arena_handle_t const* const thandle, parser_buf_t* buf,
    tree_t* node) {
    if (node == NULL) {
        return PARSER_RT_ERR;
    }

    int leaf_balance = 0;
    parser_rt_e rt =
        _parser_tokenize_string(thandle, buf, node, &leaf_balance);
    if (rt == PARSER_RT_OK && leaf_balance != 0) {
        return PARSER_RT_INVALID_EXPR;
    }
    return rt;
}
