#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

// Timing routines
struct timeval TBeg, TEnd;

#define STARTTm gettimeofday(&TBeg, NULL)
#define ENDTm gettimeofday(&TEnd, NULL)
#define DeltaUSec (((double)TEnd.tv_sec * 1000000.0 + (double)TEnd.tv_usec) - \
                   ((double)TBeg.tv_sec * 1000000.0 + (double)TBeg.tv_usec))

/**
 * Add one dot before and one after word. Input must be pointer to allocated
 * memory on heap.
 */
char *add_dots_to_word(int len, char *word);

// Returns the number of utf8 characters in string
int strlen_utf8(const char *str);

/**
 * Create array which contains values of sum of byte length of characters.
 * abcábč returns [0, 1, 2, 3, 5, 6, 8]
 */
char *create_utf_array(char *word);

/**
 * This functions takes a word and full hyphenation code and returns an
 * allocated hyphenated word.
 */
char *hyphenate_from_code(char *word, char *code);

#endif // !UTILS_H
