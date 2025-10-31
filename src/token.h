/* Header voor token_t. Een token bestaat uit een type, categorie, en
 * waarde. Categorieen zijn onderverdeeld in symbolen (variabelen,
 * nummers, constanten), unairy operators (sin, cos), en binary
 * operators (min, plus, multiplicatie, etc.).
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __TOKEN_H
#define __TOKEN_H

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

// Voor double compare.
#define EPSILON 1e-9
// Maximum aantal karakters voor een double.
#define TOKEN_STRING_LENGTH DBL_MAX_10_EXP + 2

typedef char token_string_t[TOKEN_STRING_LENGTH];

typedef enum {
    TOKEN_TYPE_MINUS,
    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_MULTIPLY,
    TOKEN_TYPE_DIVIDE,
    TOKEN_TYPE_SIN,
    TOKEN_TYPE_COS,
    TOKEN_TYPE_POWER,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_VARIABLE,
    TOKEN_TYPE_PI,
    TOKEN_TYPE_INVALID,
} token_type_e;

typedef enum {
    TOKEN_CAT_SYMBOL = 0x1,
    TOKEN_CAT_OP_UNAIR = 0x2,
    TOKEN_CAT_OP_BINAIR = 0x4,
    TOKEN_CAT_INVALID = 0x8,
} token_cat_e;

typedef union {
    char variable;
    double number;
} token_value_u;

typedef struct {
    token_type_e type;
    token_value_u value;
} token_t;

// Haal de categorie van een token op afhankelijk aan de type.
token_cat_e token_get_cat(token_t const* const token);

// Converteerd een token naar een string.
void token_string(token_t const* const token, token_string_t string);

static inline bool token_is_operation(token_t const* const t) {
    return (token_get_cat(t) &
            (TOKEN_CAT_OP_UNAIR | TOKEN_CAT_OP_BINAIR));
}

static inline bool token_is_operation_unairy(token_t const* const t) {
    return (token_get_cat(t) & TOKEN_CAT_OP_UNAIR);
}

static inline bool token_is_operation_binairy(
    token_t const* const t) {
    return (token_get_cat(t) & TOKEN_CAT_OP_BINAIR);
}

static inline bool token_is_number(token_t const* const t) {
    return (t->type == TOKEN_TYPE_NUMBER);
}

static inline bool token_cmp_double(double a, double b) {
    return (fabs(a - b) < EPSILON);
}

static inline bool token_cmp_number(token_t const* const t,
                                    double n) {
    return (t->type == TOKEN_TYPE_NUMBER &&
            token_cmp_double(t->value.number, n));
}

static inline bool token_cmp_variable(token_t const* const t1,
                                      token_t const* const t2) {
    return (t1->type == TOKEN_TYPE_VARIABLE &&
            t2->type == TOKEN_TYPE_VARIABLE &&
            t1->value.variable == t2->value.variable);
}

static inline void token_copy(token_t* const trg,
                              token_t const* const src) {
    memcpy(trg, src, sizeof(token_t));
}

static inline void token_make_number(token_t* const t, double value) {
    t->type = TOKEN_TYPE_NUMBER;
    t->value.number = value;
}

static inline void token_make_type(token_t* const t,
                                   token_type_e type) {
    t->type = type;
    t->value.number = 0.0f;
}

static inline void token_make_variable(token_t* const t, char name) {
    t->type = TOKEN_TYPE_VARIABLE;
    t->value.variable = name;
}

#endif  // __TOKEN_H
