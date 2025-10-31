/* Header van een command-line interface implementatie.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __CLI_H
#define __CLI_H

#include <stdbool.h>

#include "tree.h"

// silent bepaald of er randzaken worden geprint.
void cli_loop(bool silent);

#endif  // __CLI_H
