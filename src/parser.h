/* Header van een parser die een string omzet naar een
 * expressie boom. De parser verwacht een expressie in poolse notatie.
 * Tokens moeten gescheiden zijn met een spatie.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __PARSER_H
#define __PARSER_H

#include "tree.h"

#define PARSER_STRING_BUFFER_SIZE 1024

typedef struct {
    char* p;  // huidige pointer naar de te consumeren karakter in een
              // de buffer.
    char d[PARSER_STRING_BUFFER_SIZE];  // de buffer.
} parser_buf_t;

typedef enum {
    PARSER_RT_OK = 0,
    PARSER_RT_ERR,
    PARSER_RT_END,
    PARSER_RT_INVALID_EXPR,
    PARSER_RT_INVALID_CHAR,  // invalide karakter is gelezen.
} parser_rt_e;

// Leest een double uit de buffer vanaf buf->p.
parser_rt_e parser_read_double(parser_buf_t* buf, double* n);

// Maakt een boom met tokens van de string in de buffer.
parser_rt_e parser_tokenize_string(
    tree_arena_handle_t const* const thandle, parser_buf_t* buf,
    tree_t* node);

#endif  // __PARSER_H
