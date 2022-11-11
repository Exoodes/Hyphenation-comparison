#include "compare.h"
#include "patricia_trie.h"
#include "patterns.h"

#define _GNU_SOURCE
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 1000000 // define maximum string length

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

// Start of timing routines ========================================
struct timeval TBeg, TEnd;
#define STARTTm gettimeofday(&TBeg, NULL)
#define ENDTm gettimeofday(&TEnd, NULL)
#define DeltaUSec \
    (((double)TEnd.tv_sec * 1000000.0 + (double)TEnd.tv_usec) - ((double)TBeg.tv_sec * 1000000.0 + (double)TBeg.tv_usec))
// End of timing routines ========================================

bool verbose = false;

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

void judy_insert_patterns(Pattern_wrapper *patterns, Pvoid_t *judy_array)
{
    Word_t *PValue;

    STARTTm;
    for (int i = 0; i < patterns->count; i++)
    {
        JSLI(PValue, *judy_array, (uint8_t *) patterns->patterns[i].word);
        *PValue = (long unsigned int)patterns->patterns[i].code;
    }
    ENDTm;

    if (verbose)
        printf("Insertion in judy data structure of %u indexes took %f "
               "microseconds per index\n",
               patterns->count, DeltaUSec / patterns->count);
}

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
            JSLG(find_return, *judy_array, (uint8_t *) &word[j]);

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

int compare(const char *file_name, Pvoid_t *judy_array, patricia *patricia_trie)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *word = NULL;

    double time_judy = 0;
    double time_trie = 0;

    char *judy_hyphenated;
    char *trie_hyphenated;

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

        word_count++;
    }

    fclose(fp);
    if (line)
        free(line);

    if (word)
        free(word);

    printf("Judy - %f\n", time_judy);
    printf("Trie - %f\n", time_trie);
    printf("Judy was %.2f%% faster hyphenating %i words\n",
           (time_judy / time_trie) * 100, word_count);
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
