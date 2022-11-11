#include "patricia_trie.h"
#include "judy.h"

#define _GNU_SOURCE
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 1000000 // define maximum string length

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

// Start of timing routines ========================================
struct timeval TBeg, TEnd;
#define STARTTm gettimeofday(&TBeg, NULL)
#define ENDTm gettimeofday(&TEnd, NULL)
#define DeltaUSec \
    (((double)TEnd.tv_sec * 1000000.0 + (double)TEnd.tv_usec) - ((double)TBeg.tv_sec * 1000000.0 + (double)TBeg.tv_usec))
// End of timing routines ========================================

bool verbose = false;

/**
 * Data structure for holding 1 pattern
 * Ex.: pattern z5a2b is stored like this
 * word = zab
 * code = [0, 5, 2, 0]
 */
typedef struct
{
    char *word;
    char *code;
} Pattern;

// Data structure for holding all loaded patterns and count of loaded patterns
typedef struct
{
    Pattern *patterns;
    int count;
} Pattern_wrapper;

/* --------------------------- PATTERNS FUNCTIONS ---------------------------- */
// Helper function for showing loaded pattern in better form
void patterns_show(Pattern_wrapper *patterns, int index)
{
    printf("%s - ", patterns->patterns[index].word);

    for (int i = 0; i <= strlen(patterns->patterns[index].word); i++)
        printf("%i", patterns->patterns[index].code[i]);

    printf("\n");
}

/**
 * Function for loading patterns. This function allocate patterns data
 * structure, which must be freed later. Returns 0 if everything went ok,
 * return 1 if file was not opened correctly or allocation failed.
 */
int patterns_load(Pattern_wrapper *patterns, const char *file_name)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int allocated_count;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        printf("Cannot open file %s", file_name);
        return 1;
    }

    allocated_count = 1;
    patterns->patterns = realloc(patterns->patterns, allocated_count * sizeof(Pattern));
    if (patterns->patterns == NULL)
    {
        printf("Allocation errro\n");
        return 1;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (allocated_count == patterns->count)
        {
            allocated_count *= 2;
            patterns->patterns = realloc(patterns->patterns, allocated_count * sizeof(Pattern));
            if (patterns->patterns == NULL)
            {
                printf("Allocation errro\n");
                return 1;
            }
        }

        patterns->patterns[patterns->count].word = calloc(read, sizeof(char));
        patterns->patterns[patterns->count].code = calloc(read + 1, sizeof(char));
        if (patterns->patterns[patterns->count].word == NULL ||
            patterns->patterns[patterns->count].code == NULL)
        {
            printf("Allocation errro\n");
            return 1;
        }

        int index = 0;
        for (int i = 0; i < read; i++)
        {
            if (isdigit(line[i]))
            {
                patterns->patterns[patterns->count].code[index] = (line[i] - '0');
            }

            else if (line[i] != '\n')
            {
                patterns->patterns[patterns->count].word[index] = line[i];
                index++;
            }
        }

        patterns->count++;
    }

    fclose(fp);

    if (line)
        free(line);

    return 0;
}

// Functions for freeing all patterns and freeing pattern wrapper
void patterns_free(Pattern_wrapper *patterns)
{
    for (int i = 0; i < patterns->count; i++)
    {
        free(patterns->patterns[i].word);
        free(patterns->patterns[i].code);
    }

    free(patterns->patterns);
}

/* -------------------------- HYPHENATION FUNCTIONS ------------------------- */

char *hyphenate_from_code(char *word, char *code)
{
    int len = strlen(word);
    int hyphen_count = 0;

    for (int i = 0; i < len - 1; i++)
        if (code[i] % 2 == 1)
            hyphen_count++;

    char *result = calloc(len + hyphen_count - 1, sizeof(char));

    int r_p = 0;
    for (int i = 1; i < len - 1; i++)
    {
        if (code[i - 1] % 2 == 1 && i != 1)
        {

            result[r_p] = '-';
            r_p++;
        }

        result[r_p] = word[i];
        r_p++;
    }

    return result;
}

/* ----------------------------- JUDY FUNCTIONS ----------------------------- */

/**
 * Insert all patterns stored in Pattern_wrapper patterns into judy data
 * structure pointed by Pvoid_t. This function is timed for comparison
 */
void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array)
{
    Word_t *PValue;

    STARTTm;
    for (int i = 0; i < patterns->count; i++)
    {
        JSLI(PValue, *judy_array, patterns->patterns[i].word);
        *PValue = (long unsigned int)patterns->patterns[i].code;
    }
    ENDTm;

    if (verbose)
        printf("Insertion in judy data structure of %u indexes took %f "
               "microseconds per index\n",
               patterns->count, DeltaUSec / patterns->count);
}
// supercalifragilisticexpialidocious
/**
 * Hyphenate word using judy data structure. Returns pointer to allocated string
 * with hyphenation marks
 */
char *judy_hyphenate_word(char *word, Pvoid_t *judy_array)
{
    char backup;
    int len = strlen(word);

    char hyph_code[len - 1];
    memset(hyph_code, 0, (len - 1) * sizeof(char));

    for (int i = 1; i <= len; i++)
    {
        for (int j = 0; j <= len - i; j++)
        {
            backup = word[j + i];
            word[j + i] = '\0';

            Word_t *find_return;
            JSLG(find_return, *judy_array, &word[j]);

            if (find_return != NULL)
            {
                char *pattern_list = (char *)*find_return;

                for (int k = 0; k <= i; k++)
                {
                    if (pattern_list[k] > hyph_code[j + k - 1])
                        hyph_code[j + k - 1] = pattern_list[k];
                }
            }

            word[j + i] = backup;
        }
    }

    char *result = hyphenate_from_code(word, hyph_code);

    return result;
}

/* ------------------------- PATRICIA TRIE FUNCTIONS ------------------------- */

/**
 * Insert all patterns stored in Pattern_wrapper patterns into patricia trie
 * data structure. This function is timed for comparison
 */
void patricia_trie_insert_patterns(Pattern_wrapper *patterns,
                                   patricia *patricia_trie)
{
    STARTTm;
    for (int i = 0; i < patterns->count; i++)
    {
        patricia_insert(patricia_trie, patterns->patterns[i].word,
                        strlen(patterns->patterns[i].word),
                        patterns->patterns[i].code);
    }
    ENDTm;

    if (verbose)
        printf("Insertion in patricia of %u indexes took %6.3f microseconds "
               "per index\n",
               patterns->count, DeltaUSec / patterns->count);
}

/**
 * Hyphenate word using patricia trie data structure. Returns pointer to
 * allocated string with hyphenation marks
 */
char *patricia_trie_hyphenate_word(char *word, patricia *patricia_trie)
{
    char backup;
    int len = strlen(word);

    char hyph_code[len - 1];
    memset(hyph_code, 0, (len - 1) * sizeof(char));

    for (int i = 1; i <= len; i++)
    {
        for (int j = 0; j <= len - i; j++)
        {
            backup = word[j + i];
            word[j + i] = '\0';

            const char *pattern_list = patricia_lookup(patricia_trie, &word[j], i);

            if (pattern_list != NULL)
            {
                for (int k = 0; k <= i; k++)
                {
                    if (pattern_list[k] > hyph_code[j + k - 1])
                        hyph_code[j + k - 1] = pattern_list[k];
                }
            }

            word[j + i] = backup;
        }
    }

    char *result = hyphenate_from_code(word, hyph_code);

    return result;
}

/* ------------------------- HYPHENATION FUNCTIONS  -------------------------- */

int compare(const char *file_name, Pvoid_t *judy_array, patricia *patricia_trie)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *word = NULL;

    long start, end;
    struct timeval timecheck;
    double time_judy = 0;
    double time_trie = 0;

    char *judy_hyphenated;
    char *trie_hyphenated;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        printf("Cannot open file %s", file_name);
        return 1;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (line[read - 1] == '\n')
        {
            line[read - 1] = '\0';
            read--;
        }

        if (read == 0)
            continue;

        word = realloc(word, (read + 3) * sizeof(char));
        if (word == NULL)
        {
            printf("Allocation error\n");
            return 1;
        }

        memset(word, 0, read + 3);
        word[0] = '.';
        strcat(word, line);
        word[read + 1] = '.';

        STARTTm;
        judy_hyphenated = judy_hyphenate_word(word, judy_array);
        ENDTm;
        time_judy += DeltaUSec;

        STARTTm;
        trie_hyphenated = patricia_trie_hyphenate_word(word, patricia_trie);
        ENDTm;
        time_trie += DeltaUSec;

        if (strcmp(judy_hyphenated, trie_hyphenated) != 0)
        {
            printf("%s != %s\n", judy_hyphenated, trie_hyphenated);
        }
    }

    fclose(fp);
    if (line)
        free(line);

    if (word)
        free(word);

    printf("Judy - %f\n", time_judy);
    printf("Trie - %f\n", time_trie);
    printf("Judy was %.2f%% faster\n", (time_judy / time_trie) * 100);
    return 0;
}

int main(int argc, char const *argv[])
{
    // Check for valid size of judy's internal type
    assert(sizeof(Word_t) == sizeof(char *));

    // Creating data structure and loading patterns from external file into it
    Pattern_wrapper patterns;
    patterns.patterns = NULL;
    patterns.count = 0;

    patterns_load(&patterns, argv[1]);

    // Creating judy data structure
    Pvoid_t judy_array = (Pvoid_t)NULL;
    Pvoid_t *judy_array_p = &judy_array;

    // Creating patricia trie data structure
    patricia *patricia_trie = NULL;
    patricia_create(NULL, NULL, NULL, &patricia_trie);

    // Inserting patterns into data structures
    judy_insert_patterns(&patterns, judy_array_p);
    patricia_trie_insert_patterns(&patterns, patricia_trie);

    // Comparing how both data structure do in hyphenation
    compare(argv[2], &judy_array, patricia_trie);

    // Destroying all data structures and freeing all of its memory
    Word_t freed_count;
    JSLFA(freed_count, judy_array);
    patricia_destroy(patricia_trie);
    patterns_free(&patterns);

    return 0;
}
