#define _GNU_SOURCE

#include "compare.h"
#include "patterns.h"
#include "judy.h"
#include "trie.h"
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

// Global constants
int left_hyphen_min = 2;
int right_hyphen_min = 2;
bool verbose = false;
char hyphenation_char = '-';

// Private compare.c function to print out results of time testing
void print_results(double time_judy, double time_trie, int word_count)
{
    printf("Hyphenation results\n");
    printf("Hyphenating %i words with patterns stored in Judy        took %8.0f"
           " microseconds total, %4.3f microseconds per word\n",
           word_count, time_judy, time_judy / word_count);
    printf("Hyphenating %i words with patterns stored in cprops Trie took %8.0f"
           " microseconds total, %4.3f microseconds per word\n",
           word_count, time_trie, time_trie / word_count);
}

void space_test_judy(Pattern_wrapper *pattern_list)
{
    Pvoid_t pattern_judy = (Pvoid_t)NULL;
    judy_insert_patterns(pattern_list, &pattern_judy);

    Word_t freed_count;
    JSLFA(freed_count, pattern_judy);

    patterns_free(pattern_list);
}

void space_test_trie(Pattern_wrapper *pattern_list)
{
    cp_trie *pattern_trie = cp_trie_create(COLLECTION_MODE_NOSYNC);

    trie_insert_patterns(pattern_list, pattern_trie);

    cp_trie_destroy(pattern_trie);
    patterns_free(pattern_list);
}

void compare(const char *file_name, Pvoid_t *pattern_judy, cp_trie *pattern_trie)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    double time_judy = 0;
    double time_trie = 0;
    char *word = NULL;
    int word_count = 0;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        printf("Cannot open file %s\n", file_name);
        return;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        // Remove new line character
        if (line[read - 1] == '\n')
        {
            line[read - 1] = '\0';
            read--;
        }

        // Skip if now word was loaded
        if (read == 0)
            continue;

        word = add_dots_to_word(read, line);
        char *utf8_code = create_utf_array(word);

        STARTTm;
        char *judy_hyphenated = judy_hyphenate(word, pattern_judy, utf8_code);
        ENDTm;
        time_judy += DeltaUSec;

        STARTTm;
        char *trie_hyphenated = trie_hyphenate(word, pattern_trie, utf8_code);
        ENDTm;
        time_trie += DeltaUSec;

        word_count++;
        free(judy_hyphenated);
        free(trie_hyphenated);
        free(utf8_code);
    }

    fclose(fp);
    if (line)
        free(line);
    if (word)
        free(word);

    print_results(time_judy, time_trie, word_count);
}

int main(int argc, char **argv)
{

    // Check for valid size of judy's internal type
    assert(sizeof(Word_t) == sizeof(char *));

    // Parse command line arguments
    bool time_test_insert_flag = false;
    bool memory_test_Judy_flag = false;
    bool memory_test_Trie_flag = false;
    bool memory_test_only_patterns_flag = false;
    char *patterns_filepath = NULL;
    char *words_filepath = NULL;
    int c;

    while ((c = getopt(argc, argv, "jtpvi")) != -1)
        switch (c)
        {
        case 'j':
            memory_test_Judy_flag = true;
            break;
        case 't':
            memory_test_Trie_flag = true;
            break;
        case 'p':
            memory_test_only_patterns_flag = true;
            break;
        case 'i':
            time_test_insert_flag = true;
            break;
        case 'v':
            verbose = true;
            break;
        default:
            fprintf(stderr, "Unknown option `\\x%x'.\n", optopt);
            return 1;
        }

    if (argc - optind != 2)
    {
        fprintf(stderr, "Missing file paths\n");
    }
    patterns_filepath = argv[optind];
    words_filepath = argv[optind + 1];

    // Load patterns
    Pattern_wrapper pattern_list;
    patterns_load(&pattern_list, patterns_filepath);

    // Memory testing
    if (memory_test_only_patterns_flag)
    {
        patterns_free(&pattern_list);
        return 0;
    }

    if (memory_test_Judy_flag)
    {
        space_test_judy(&pattern_list);
        return 0;
    }

    if (memory_test_Trie_flag)
    {
        space_test_trie(&pattern_list);
        return 0;
    }

    // Creating judy data structure
    Pvoid_t pattern_judy = (Pvoid_t)NULL;

    // Creating patricia trie data structure
    cp_trie *pattern_trie = cp_trie_create(COLLECTION_MODE_NOSYNC);

    // Inserting patterns into data structures
    judy_insert_patterns(&pattern_list, &pattern_judy);
    trie_insert_patterns(&pattern_list, pattern_trie);

    // Comparing how both data structure do in hyphenation
    if (!time_test_insert_flag)
        compare(words_filepath, &pattern_judy, pattern_trie);

    // Destroying all data structures and freeing all of its memory
    Word_t freed_count;
    JSLFA(freed_count, pattern_judy);
    cp_trie_destroy(pattern_trie);
    patterns_free(&pattern_list);

    return 0;
}