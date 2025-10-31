/* Implementatie van een command-line interface.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "ascii.h"
#include "diff.h"
#include "file.h"
#include "parser.h"
#include "simp.h"
#include "token.h"

typedef enum {
    CLI_RT_OK = 0,
    CLI_RT_ERR,
    CLI_RT_END,
    CLI_RT_ERR_BIG,
} cli_rt_e;

typedef struct {
    tree_arena_handle_t const *ah;  // arena handle.
    tree_arena_handle_t const
        *bh;          // arena handle, voor buffer rotatie.
    tree_t *r;        // root van de ABS tree.
    parser_buf_t *b;  // buffer van karakters om te tokenizen.
} cli_parser_data_t;

cli_rt_e cli_parser_exp(cli_parser_data_t *pdata) {
    tree_arena_clear(pdata->ah);
    pdata->r = tree_arena_new_node(pdata->ah);
    if (pdata->r == NULL) {
        printf("ERR! Failed to parse expression, it is too long.\n");
        return CLI_RT_ERR_BIG;
    }

    parser_rt_e rt =
        parser_tokenize_string(pdata->ah, pdata->b, pdata->r);

    if (rt != PARSER_RT_OK || *(pdata->b->p) != '\0') {
        printf(
            "ERR! Unable to parse string, invalid grammar used: "
            "{\n\t"
            "%s\n}\n",
            pdata->b->d);
        pdata->r = NULL;
    }

    return CLI_RT_OK;
}

cli_rt_e cli_parser_dot(cli_parser_data_t *pdata) {
    while (ascii_char_is_whitespace[(int)*(pdata->b->p)]) {
        pdata->b->p++;
    };

    if (*(pdata->b->p) == '\0') {
        printf("ERR! No filename provided.\n");
        return CLI_RT_ERR;
    }

    FILE *f = fopen(pdata->b->p, "w");

    if (f == NULL) {
        printf("ERR! Failed to open file.\n");
        return CLI_RT_ERR;
    }

    if (file_write_tree(f, pdata->r) != FILE_RT_OK) {
        printf("ERR! Failed to write DOT file from tree.\n");
    }

    fclose(f);
    return CLI_RT_OK;
}

bool cli_tree_print_should_bracket[] = {
    [0 ... TOKEN_TYPE_INVALID] = true, [TOKEN_TYPE_MINUS] = true,
    [TOKEN_TYPE_PLUS] = true,          [TOKEN_TYPE_MULTIPLY] = true,
    [TOKEN_TYPE_DIVIDE] = true,        [TOKEN_TYPE_SIN] = false,
    [TOKEN_TYPE_COS] = false,          [TOKEN_TYPE_POWER] = true,
    [TOKEN_TYPE_NUMBER] = false,       [TOKEN_TYPE_VARIABLE] = false,
    [TOKEN_TYPE_PI] = false,
};

void cli_tree_print(tree_t const *const r) {
    // Print brackets wanneer de volgende token een volgens de mapping
    // hierboven een bracket benoodzaakt. Niet wanneer de volgende
    // operator dezelfde operator is als de huidige.
    token_string_t string = {0};  // de token als string.
    bool bp = false;              // is er een bracket geopend?

    token_string(&r->token, string);

    if (token_get_cat(&r->token) & TOKEN_CAT_OP_UNAIR) {
        printf("%s(", string);
        bp = true;
    }

    if (!bp && r->left && r->left->token.type != r->token.type &&
        cli_tree_print_should_bracket[r->left->token.type]) {
        printf("(");
        bp = true;
    }

    if (r->left) {
        cli_tree_print(r->left);
    }

    if (bp) {
        printf("\b) ");
        bp = false;
    }

    if (token_get_cat(&r->token) &
        (TOKEN_CAT_OP_BINAIR | TOKEN_CAT_SYMBOL)) {
        printf("%s ", string);
    }

    if (r->right && r->right->token.type != r->token.type &&
        cli_tree_print_should_bracket[r->right->token.type]) {
        printf("(");
        bp = true;
    }

    if (r->right) {
        cli_tree_print(r->right);
    }

    if (bp) {
        printf("\b) ");
    }
}

cli_rt_e cli_parser_print(cli_parser_data_t *pdata) {
    if (pdata->r == NULL) {
        printf("ERR! No tree has been initialized.\n");
        return CLI_RT_ERR;
    }

    cli_tree_print(pdata->r);
    printf("\n");
    return CLI_RT_OK;
}

cli_rt_e cli_parser_end(cli_parser_data_t *pdata) {
    return CLI_RT_END;
}

cli_rt_e cli_parser_eval(cli_parser_data_t *pdata) {
    while (ascii_char_is_whitespace[(int)*(pdata->b->p)]) {
        pdata->b->p++;
    };

    double v;  // waarde van x gegeven in de input.
    if (parser_read_double(pdata->b, &v) == PARSER_RT_OK) {
        tree_substitute_x(pdata->r, v);
    } else {
        printf(
            "ERR! Unable to read the value of x from the input.\n");
        return CLI_RT_ERR;
    }

    return CLI_RT_OK;
}

cli_rt_e cli_parser_simp(cli_parser_data_t *pdata) {
    // Errors worden al geprint tijdens het simplificeren, dus de
    // return waarde hoeft niet gevalideerd te worden. Ook hoeft het
    // programma niet gestopt te worden.
    if (pdata->r == NULL) {
        printf("ERR! No tree has been initialized.\n");
        return CLI_RT_ERR;
    }

    simp_tree(pdata->r);
    return CLI_RT_OK;
}

cli_rt_e cli_parser_diff(cli_parser_data_t *pdata) {
    if (pdata->r == NULL) {
        printf("ERR! No tree has been initialized.\n");
        return CLI_RT_ERR;
    }

    tree_arena_clear(pdata->bh);
    tree_t *r = diff_tree(pdata->r, pdata->bh);
    if (r == NULL) {
        printf(
            "ERR! Failed to differientiate the expression, it is "
            "likely too long.\n");
        return CLI_RT_ERR;
    }

    tree_arena_handle_t const *b = pdata->bh;
    pdata->bh = pdata->ah;
    pdata->ah = b;
    pdata->r = r;
    return CLI_RT_OK;
}

cli_rt_e cli_parser_invalid(cli_parser_data_t *pdata) {
    printf("ERR! Invalid input provided.\n");
    return CLI_RT_ERR;
}

cli_rt_e cli_parser_help(cli_parser_data_t *pdata) {
    printf(
        "# exp <expression> \t; loads the expression, expression "
        "must be in polish notation.\n");
    printf(
        "# print \t\t; print the loaded expression in infix "
        "notation.\n");
    printf("# simp \t\t\t; simplify the loaded expression.\n");
    printf(
        "# eval <value> \t\t; substitute x within the loaded "
        "expression for <value>, value may be point seperated.\n");
    printf(
        "# dot <filename> \t; write the loaded expression to a DOT "
        "file format.\n");
    printf(
        "# diff \t\t\t; differentiates the loaded expression on "
        "x.\n");
    printf("# end \t\t\t; end the program.\n");
    printf("# help \t\t\t; print help.\n");
    return CLI_RT_OK;
}

typedef enum {
    CLI_MENU_OPTION_EXP,
    CLI_MENU_OPTION_PRINT,
    CLI_MENU_OPTION_DOT,
    CLI_MENU_OPTION_END,
    CLI_MENU_OPTION_DIFF,
    CLI_MENU_OPTION_SIMP,
    CLI_MENU_OPTION_EVAL,
    CLI_MENU_OPTION_HELP,
    CLI_MENU_OPTION_INVALID,
} cli_menu_option_e;

typedef cli_rt_e (*cli_parser_t)(cli_parser_data_t *pdata);

cli_parser_t cli_menu[] = {
    [0 ... CLI_MENU_OPTION_INVALID] = cli_parser_invalid,
    [CLI_MENU_OPTION_EXP] = cli_parser_exp,
    [CLI_MENU_OPTION_PRINT] = cli_parser_print,
    [CLI_MENU_OPTION_DOT] = cli_parser_dot,
    [CLI_MENU_OPTION_END] = cli_parser_end,
    [CLI_MENU_OPTION_SIMP] = cli_parser_simp,
    [CLI_MENU_OPTION_EVAL] = cli_parser_eval,
    [CLI_MENU_OPTION_HELP] = cli_parser_help,
    [CLI_MENU_OPTION_DIFF] = cli_parser_diff,
};

cli_rt_e cli_print_top(cli_parser_data_t *pdata) {
    printf("# Simple calculator by Jenny Vermeltfoort, s3787494\n");
    printf("# Run with flag '-s' to make it silent.\n");
    cli_menu[CLI_MENU_OPTION_HELP](pdata);
    return CLI_RT_OK;
}

cli_rt_e cli_print_top_silent(cli_parser_data_t *pdata) {
    return CLI_RT_OK;
}

cli_rt_e cli_print_prompt(cli_parser_data_t *pdata) {
    printf("$ ");
    return CLI_RT_OK;
}

cli_rt_e cli_print_prompt_silent(cli_parser_data_t *pdata) {
    return CLI_RT_OK;
}

void cli_loop(bool silent) {
    tree_arena_handle_t const *ah = tree_arena_malloc();
    tree_arena_handle_t const *bh = tree_arena_malloc();
    cli_parser_t print_top[] = {
        [false] = cli_print_top, [true] = cli_print_top_silent};
    cli_parser_t print_prompt[] = {
        [false] = cli_print_prompt, [true] = cli_print_prompt_silent};
    parser_buf_t b = {0};
    cli_parser_data_t pdata = {
        .ah = ah,
        .bh = bh,
        .r = NULL,
        .b = &b,
    };

    print_top[silent](&pdata);

    cli_rt_e rt = CLI_RT_OK;
    while (rt != CLI_RT_END &&
           print_prompt[silent](&pdata) == CLI_RT_OK &&
           fgets(b.d, PARSER_STRING_BUFFER_SIZE, stdin) != NULL) {
        // Het zou eigenlijk beter zijn om in secties de data uit
        // stdin te lezen om te enige afhandeling van buffer overflows
        // te voorkomen. Nu leiden een buffer overflow in een invalid
        // command, terwijl het eigen nog een onderdeel is van
        // bijvoorbeeld een syntax boom. Heb alleen geen zin om dit te
        // veranderen, heb al genoeg tijd besteed aan deze opdracht.

        for (int i = 0; b.d[i] != '\0'; i++) {
            b.d[i] = (b.d[i] == '\n') ? 0 : b.d[i];
        }
        b.p = &b.d[0];

        cli_menu_option_e i = CLI_MENU_OPTION_INVALID;
        if (b.d[0] == 'p' && b.d[1] == 'r')
            i = CLI_MENU_OPTION_PRINT;
        else if (b.d[0] == 'e' && b.d[1] == 'x')
            i = CLI_MENU_OPTION_EXP;
        else if (b.d[0] == 'e' && b.d[1] == 'n')
            i = CLI_MENU_OPTION_END;
        else if (b.d[0] == 'd' && b.d[1] == 'o')
            i = CLI_MENU_OPTION_DOT;
        else if (b.d[0] == 's' && b.d[1] == 'i')
            i = CLI_MENU_OPTION_SIMP;
        else if (b.d[0] == 'e' && b.d[1] == 'v')
            i = CLI_MENU_OPTION_EVAL;
        else if (b.d[0] == 'd' && b.d[1] == 'i')
            i = CLI_MENU_OPTION_DIFF;
        else if (b.d[0] == 'h' && b.d[1] == 'e')
            i = CLI_MENU_OPTION_HELP;

        while (!ascii_char_is_whitespace[(int)*(b.p)] &&
               *(b.p) != '\0') {
            b.p++;
        };

        // De function call is verantwoordelijk voor het verplaatsen
        // van de pointer naar de volgende stuk text in de buffer.
        rt = cli_menu[i](&pdata);
    }

    tree_arena_free(pdata.ah);
    tree_arena_free(pdata.bh);
}
