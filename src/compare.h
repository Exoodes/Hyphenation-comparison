#include "patricia_trie.h"
#include "patterns.h"
#include "judy.h"

/* -------------------------- HYPHENATION FUNCTIONS ------------------------- */

char *hyphenate_from_code(char *word, char *code);

/* ----------------------------- JUDY FUNCTIONS ----------------------------- */

/**
 * Insert all patterns stored in Pattern_wrapper patterns into judy data
 * structure pointed by Pvoid_t. This function is timed for comparison
 */
void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array);
// supercalifragilisticexpialidocious
/**
 * Hyphenate word using judy data structure. Returns pointer to allocated string
 * with hyphenation marks
 */
char *judy_hyphenate_word(char *word, Pvoid_t *judy_array);

/* ------------------------- PATRICIA TRIE FUNCTIONS ------------------------- */

/**
 * Insert all patterns stored in Pattern_wrapper patterns into patricia trie
 * data structure. This function is timed for comparison
 */
void patricia_trie_insert_patterns(Pattern_wrapper *patterns,
                                   patricia *patricia_trie);

/**
 * Hyphenate word using patricia trie data structure. Returns pointer to
 * allocated string with hyphenation marks
 */
char *patricia_trie_hyphenate_word(char *word, patricia *patricia_trie);

/* ------------------------- HYPHENATION FUNCTIONS  -------------------------- */

int compare(const char *file_name, Pvoid_t *judy_array, patricia *patricia_trie);
