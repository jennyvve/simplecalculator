/* Header van een tree_t naar dot file vertaler.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#include "file.h"

void _file_write_tree(FILE* f, tree_t* root, int* i) {
    token_string_t string = {0};
    int ir = *i;

    token_string(&root->token, string);
    fprintf(f, "\t%i [label=\"%s\"]\n", *i, string);

    if (root->left) {
        (*i)++;
        fprintf(f, "\t%i -> %i\n", ir, *i);
        _file_write_tree(f, root->left, i);
    }

    if (root->right) {
        (*i)++;
        fprintf(f, "\t%i -> %i\n", ir, *i);
        _file_write_tree(f, root->right, i);
    }
}

file_rt_e file_write_tree(FILE* f, tree_t* root) {
    if (f == NULL || root == NULL) {
        return FILE_RT_ERR;
    }

    fprintf(f, "digraph G {\n");
    int i = 1;
    _file_write_tree(f, root, &i);
    fprintf(f, "}\n");

    return FILE_RT_OK;
}
