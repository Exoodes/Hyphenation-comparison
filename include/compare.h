#ifndef COMPARE_H
#define COMPARE_H

#include "patterns.h"

#include <Judy.h>
#include <stdbool.h>
#include <cprops/trie.h>

/**
 * This function inserts all patterns from pattern_list to Judy and then free
 * all of its memory. Should be run with Valgrind or other memory measuring
 * software
 */
void space_test_judy(Pattern_wrapper *pattern_list);

/**
 * This function inserts all patterns from pattern_list to Cprops Trie and then
 * free all of its memory. Should be run with Valgrind or other memory measuring
 * software
 */
void space_test_trie(Pattern_wrapper *pattern_list);

/**
 * Load words from file_name and hyphenate them with patterns stored in judy and
 * in cprops trie. This proccess is timed.
 */
void compare(const char *file_name, Pvoid_t *judy_array,
             cp_trie *cprops_patricia_trie);

#endif // !COMPARE_H