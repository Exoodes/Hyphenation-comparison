#ifndef TRIE_H
#define TRIE_H

#include "patterns.h"

#include <cprops/trie.h>

/**
 * Insert all patterns stored in Pattern_wrapper variable into Cprops Trie data
 * structure. This function is timed for comparison(outputted only with -v).
 */
void trie_insert_patterns(Pattern_wrapper *patterns, cp_trie *patricia_trie);

/**
 * Hyphenate word using patterns stored in Cprops Trie. Returns pointer to
 * allocated string with hyphenation characters.
 */
char *trie_hyphenate(char *word, cp_trie *cprops_patricia_trie, const char *utf8_code);

#endif // !TRIE_H