/* Header van een tree_t naar dot file vertaler.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __FILE_H
#define __FILE_H

#include <stdio.h>

#include "err.h"
#include "tree.h"

typedef enum {
    FILE_RT_OK = 0,
    FILE_RT_ERR,
} file_rt_e;

file_rt_e file_write_tree(FILE* f, tree_t* root);

// Niet geimplementeert, maar zou de gelezen nodes en verbindingen los
// van elkaar in een array/linked list kunnen plaatsten. Wanneer de
// volledige file is geparsed kan dan met deze informatie een nieuwe
// tree opgebouwd worden.
file_rt_e file_read_tree(FILE* f, tree_t* root);

#endif  // __FILE_H