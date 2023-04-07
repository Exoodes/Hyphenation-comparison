#ifndef JUDY_H
#define JUDY_H

#include <Judy.h>
#include "patterns.h"

/**
 * Insert all patterns stored in Pattern_wrapper variable into judy data
 * structure. This function is timed for comparison(outputted only with -v).
 */
void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array);

/**
 * Hyphenate word using patterns stored in judy. Returns pointer to allocated
 * string with hyphenation characters.
 */
char *judy_hyphenate(char *word, Pvoid_t *judy_array, const char *utf8_code);

#endif // !JUDY_H