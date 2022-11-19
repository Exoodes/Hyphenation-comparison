#ifndef COMPARE_H
#define COMPARE_H

#include "patricia_trie.h"
#include "patterns.h"

#include <Judy.h>
#include <stdbool.h>

/**
 * Insert all patterns stored in Pattern_wrapper patterns into judy data
 * structure pointed by Pvoid_t. This function is timed for comparison
 */
void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array);

/**
 * Insert all patterns stored in Pattern_wrapper patterns into patricia trie
 * data structure. This function is timed for comparison
 */
void patricia_trie_insert_patterns(Pattern_wrapper *patterns,
                                   patricia *patricia_trie);

/**
 * This functions takes a word and full hyphenation code and returns a new
 * hyphenated word. Hyphenation character can be changed.
 */
char *hyphenate_from_code(char *word, char *code);

/**
 *
 */
char *create_array_utf(char *word);

/**
 *
 */
char *add_dots_to_word(int len, char *word, char *previous_word);

/**
 * Hyphenate word using judy or patricia trie data structure based on branch
 * variable. Returns pointer to allocated string with hyphenation marks
 */
char *hyphenate_word(char *word, Pvoid_t *judy_array, patricia *patricia_trie,
                     const char *utf8_code, bool branch);

/**
 * Load words from file_name and hyphenate them through judy and patricia trie
 * data structure and time it.
 */
int compare(const char *file_name, Pvoid_t *judy_array, patricia *patricia_trie);

#endif // !COMPARE_H