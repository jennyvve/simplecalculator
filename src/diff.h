/* Header van een ABS boom differentieerder.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __DIFF_H
#define __DIFF_H

#include "tree.h"

tree_t* diff_tree(tree_t* t, tree_arena_handle_t const* const h);

#endif  // __DIFF_H
