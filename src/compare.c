#define _GNU_SOURCE

#include "compare.h"
#include "patricia_trie.h"
#include "patterns.h"

#include <Judy.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <cprops/trie.h>
#include <unistd.h>

// Judy settings
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 100     // define maximum string length

// Timing routines
struct timeval TBeg, TEnd;

#define STARTTm gettimeofday(&TBeg, NULL)
#define ENDTm gettimeofday(&TEnd, NULL)
#define DeltaUSec (((double)TEnd.tv_sec * 1000000.0 + (double)TEnd.tv_usec) - \
                   ((double)TBeg.tv_sec * 1000000.0 + (double)TBeg.tv_usec))

// Global constants
bool verbose = false;
bool log_hyphenation = false;
FILE *log_file;
char hyphenation_char = '-';

void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array)
{
    Word_t *PValue;

    STARTTm;
    for (int i = 0; i < patterns->count; i++)
    {
        JSLI(PValue, *judy_array, (uint8_t *)patterns->patterns[i].word);
        *PValue = (long unsigned int)patterns->patterns[i].code;
    }
    ENDTm;

    if (verbose)
        printf("Insertion in judy data structure            of %u patterns took"
               " %.3f microseconds per pattern\n",
               patterns->count, DeltaUSec / patterns->count);
}

void cprops_patricia_trie_insert_patterns(Pattern_wrapper *patterns,
                                          cp_trie *patricia_trie)
{
    STARTTm;
    for (int i = 0; i < patterns->count; i++)
    {
        cp_trie_add(patricia_trie, patterns->patterns[i].word,
                    patterns->patterns[i].code);
    }
    ENDTm;

    if (verbose)
        printf("Insertion in cprops patricia trie structure of %u patterns "
               "took %.3f microseconds per pattern\n",
               patterns->count, DeltaUSec / patterns->count);
}

char *hyphenate_from_code(char *word, char *code)
{
    int len = strlen(word);
    int len_utf = strlen_utf8(word);
    int hyphen_count = 0;

    if (log_hyphenation)
    {
        fprintf(log_file, "Final hyphenation code: ");
    }

    for (int i = 1; i < len_utf; i++)
    {
        if (log_hyphenation)
        {
            fprintf(log_file, "%i", code[i]);
        }
        if (code[i] % 2 == 1)
            hyphen_count++;
    }

    if (log_hyphenation)
    {
        fprintf(log_file, "\n");
    }
    char *result = calloc(len + hyphen_count + 1, sizeof(char));

    int result_index = 0;
    int code_index = 1;
    for (int i = 1; i < len - 1; i++)
    {
        if ((word[i] & 0xF8) == 0xF0 || (word[i] & 0xF0) == 0xE0 ||
            (word[i] & 0xE0) == 0xC0 || (word[i] >= 0 && word[i] <= 127))
        {
            if (code[code_index] % 2 == 1 && i != 1)
            {
                result[result_index] = hyphenation_char;
                result_index++;
            }

            code_index++;
        }

        result[result_index] = word[i];
        result_index++;
    }

    return result;
}

// compare.c private function
const char *_get_pattern(Pvoid_t *judy_array,
                         cp_trie *cprops_patricia_trie, int branch,
                         char *pattern_word, int len)
{
    Word_t *find_return;

    switch (branch)
    {
    case 0:
        JSLG(find_return, *judy_array, (uint8_t *)pattern_word);
        if (find_return != NULL)
            return (char *)*find_return;

        return NULL;
        break;

    case 1:
        return cp_trie_exact_match(cprops_patricia_trie, pattern_word);
        break;

    default:
        return NULL;
    }
}

char *hyphenate_word(char *word, Pvoid_t *judy_array,
                     cp_trie *cprops_patricia_trie, const char *utf8_code,
                     int branch)
{
    char backup;
    int len = strlen_utf8(word);

    char hyph_code[len + 1];
    memset(hyph_code, 0, (len + 1) * sizeof(char));
    const char *pattern_code = NULL;

    if (log_hyphenation)
    {
        fprintf(log_file, "Hyphenating word '%s':\n", word);
    }

    for (int i = 1; i <= len; i++)
    {
        for (int j = 0; j <= len - i; j++)
        {
            backup = word[(int)utf8_code[j + i]];
            word[(int)utf8_code[j + i]] = '\0';

            pattern_code = _get_pattern(judy_array, cprops_patricia_trie, branch,
                                        &word[(int)utf8_code[j]],
                                        utf8_code[j + i] - utf8_code[j]);

            if (log_hyphenation)
            {
                if (pattern_code == NULL)
                {
                    fprintf(log_file, "Subword was not found: '%s'\n", &word[(int)utf8_code[j]]);
                }
                else
                {
                    fprintf(log_file, "Subword was found    : '%s' - pattern: ", &word[(int)utf8_code[j]]);
                }
            }

            if (pattern_code != NULL)
            {
                for (int k = 0; k <= i; k++)
                {
                    if (log_hyphenation)
                    {
                        fprintf(log_file, "%i", pattern_code[k]);
                    }

                    if (pattern_code[k] > hyph_code[j + k])
                        hyph_code[j + k] = pattern_code[k];
                }

                if (log_hyphenation)
                {
                    fputc('\n', log_file);
                }
            }

            word[(int)utf8_code[j + i]] = backup;
        }
    }

    char *result = hyphenate_from_code(word, hyph_code);
    if (log_hyphenation)
    {
        fprintf(log_file, "Hyphenating result: '%s'\n\n", result);
    }
    return result;
}

char *create_utf_array(char *word)
{
    char *code = calloc((strlen_utf8(word) + 2), sizeof(char));
    int array_index = 1;
    char c;

    for (int i = 0, ix = strlen(word); i < ix; i++)
    {
        c = word[i];
        if (c >= 0 && c <= 127)
        {
            i += 0;
            code[array_index] = code[array_index - 1] + 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            i += 1;
            code[array_index] = code[array_index - 1] + 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            i += 2;
            code[array_index] = code[array_index - 1] + 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            i += 3;
            code[array_index] = code[array_index - 1] + 4;
        }

        array_index++;
    }

    return code;
}

// compare.c private function
void print_results(double time_judy, double time_cprops_trie, int word_count)
{
    printf("Judy        - Hyphenating %i words took %.2f microseconds "
           "total, %.2f microseconds per word\n",
           word_count, time_judy, time_judy / word_count);
    printf("cprops Trie - Hyphenating %i words took %.2f microseconds "
           "total, %.2f microseconds per word\n",
           word_count, time_cprops_trie, time_cprops_trie / word_count);
}

char *add_dots_to_word(int len, char *word, char *previous_word)
{
    char *result = realloc(previous_word, (len + 3) * sizeof(char));
    if (result == NULL)
        return NULL;

    memset(result, 0, len + 3);
    result[0] = '.';
    strcat(result, word);
    result[len + 1] = '.';

    return result;
}

int compare(const char *file_name, Pvoid_t *judy_array, cp_trie *cprops_patricia_trie)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *word = NULL;

    double time_judy = 0;
    double time_cprops_trie = 0;
    int word_count = 0;

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

        word = add_dots_to_word(read, line, word);
        if (word == NULL)
        {
            return 1;
        }

        char *utf8_code = create_utf_array(word);

        STARTTm;
        char *judy_hyphenated = hyphenate_word(word, judy_array,
                                               cprops_patricia_trie, utf8_code,
                                               0);
        ENDTm;
        time_judy += DeltaUSec;

        STARTTm;
        char *cprops_trie_hyphenated = hyphenate_word(word, judy_array,
                                                      cprops_patricia_trie,
                                                      utf8_code, 1);
        ENDTm;
        time_cprops_trie += DeltaUSec;

        if (strcmp(judy_hyphenated, cprops_trie_hyphenated) != 0)
        {
            printf("Error: %s != %s\n", judy_hyphenated,
                   cprops_trie_hyphenated);
        }

        word_count++;
        free(judy_hyphenated);
        free(cprops_trie_hyphenated);
        free(utf8_code);
    }

    fclose(fp);
    if (line)
        free(line);
    if (word)
        free(word);

    print_results(time_judy, time_cprops_trie, word_count);
    return 0;
}

void space_test_judy(Pattern_wrapper *patterns)
{
    Pvoid_t judy_array = (Pvoid_t)NULL;
    Pvoid_t *judy_array_p = &judy_array;

    judy_insert_patterns(patterns, judy_array_p);

    Word_t freed_count;
    JSLFA(freed_count, judy_array);
    patterns_free(patterns);
}

void space_test_trie(Pattern_wrapper *patterns)
{
    cp_trie *cprops_patricia_trie = cp_trie_create(COLLECTION_MODE_NOSYNC);

    cprops_patricia_trie_insert_patterns(patterns, cprops_patricia_trie);

    cp_trie_destroy(cprops_patricia_trie);
    patterns_free(patterns);
}

int main(int argc, char const *argv[])
{
    // Check for valid size of judy's internal type
    assert(sizeof(Word_t) == sizeof(char *));

    // Creating data structure for patterns and loading them from external file
    Pattern_wrapper patterns;
    patterns.patterns = NULL;
    patterns.count = 0;
    patterns_load(&patterns, argv[1]);

    // Space testing
    if (argc > 3 && strcmp(argv[3], "-J") == 0)
    {
        space_test_judy(&patterns);
        return 0;
    }

    if (argc > 3 && strcmp(argv[3], "-T") == 0)
    {
        space_test_trie(&patterns);
        return 0;
    }

    // Creating judy data structure
    Pvoid_t judy_array = (Pvoid_t)NULL;
    Pvoid_t *judy_array_p = &judy_array;

    // Creating patricia trie data structure
    cp_trie *cprops_patricia_trie = cp_trie_create(COLLECTION_MODE_NOSYNC);

    // Create log file
    if (log_hyphenation)
    {
        log_file = fopen("hyphenation.log", "w");
    }

    // Inserting patterns into data structures
    judy_insert_patterns(&patterns, judy_array_p);
    cprops_patricia_trie_insert_patterns(&patterns, cprops_patricia_trie);

    // Comparing how both data structure do in hyphenation
    compare(argv[2], &judy_array, cprops_patricia_trie);

    // Destroying all data structures and freeing all of its memory
    Word_t freed_count;
    JSLFA(freed_count, judy_array);
    cp_trie_destroy(cprops_patricia_trie);
    patterns_free(&patterns);
    return 0;
}