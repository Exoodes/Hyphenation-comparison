
#ifndef COMPARE_H
#define COMPARE_H

#include "patricia_trie.h"
#include "patterns.h"

#include <Judy.h>
#include <stdbool.h>
#include <cprops/trie.h>

/**
 * Insert all patterns stored in Pattern_wrapper variable into judy data
 * structure pointed. This function is timed for comparison.
 */
void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array);

/**
 * Insert all patterns stored in Pattern_wrapper variable into cprops patricia
 * trie data structure. This function is timed for comparison.
 */
void cprops_patricia_trie_insert_patterns(Pattern_wrapper *patterns,
                                          cp_trie *patricia_trie);
/**
 * This functions takes a word and full hyphenation code and returns an
 * allocated hyphenated word. Hyphenation character can be changed (TODO).
 */
char *hyphenate_from_code(char *word, char *code);

/**
 * Create array which contains values of sum of byte length of characters.
 * abcábč returns [0, 1, 2, 3, 5, 6, 8]
 */
char *create_utf_array(char *word);

/**
 * Add one dot before and one after word. Input must be pointer to allocated
 * memory on heap.
 */
char *add_dots_to_word(int len, char *word, char *previous_word);

/**
 * Hyphenate word using judy, cprops patricia trie or patricia trie data
 * structure based on branch variable. Returns pointer to allocated string with
 * hyphenation marks
 */
char *hyphenate_word(char *word, Pvoid_t *judy_array, cp_trie *cprops_patricia_trie,
                     const char *utf8_code, int branch);

/**
 * Load words from file_name and hyphenate them through judy, cprops patricia
 * trie and patricia trie data structure and time it.
 */
int compare(const char *file_name, Pvoid_t *judy_array, cp_trie *cprops_patricia_trie);

#endif // !COMPARE_H