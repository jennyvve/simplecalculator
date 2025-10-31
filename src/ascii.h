/* Header met verschillende ascii hulp functies.
 *
 * Door: Jenny Vermeltfoort, Universiteit van Leiden.
 */

#ifndef __ASCII_H
#define __ASCII_H

#define ASCII_SYMBOLS 128

#include <stdbool.h>

static const bool ascii_char_is_whitespace[ASCII_SYMBOLS] = {
    [0 ...(ASCII_SYMBOLS - 1)] = false,
    [' '] = true,
    ['\n'] = true,
    ['\t'] = true,
    ['\r'] = true,
};

static const bool ascii_char_is_number[ASCII_SYMBOLS] = {
    [0 ...(ASCII_SYMBOLS - 1)] = false,
    ['0' ... '9'] = true,
};

#endif  // __ASCII_H
