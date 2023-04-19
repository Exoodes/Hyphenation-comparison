#define _GNU_SOURCE

#include "hyphenator.h"
#include "patterns.h"
#include "judy.h"
#include "utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global constants
int left_hyphen_min = 2;
int right_hyphen_min = 2;
bool verbose = false;
char hyphenation_char = '-';
char usage[] = "\nUsage: hyphenator [options] pattern_file\n"
               "hyphenator program loads hyphenation patterns and then hyphenates words from the file or terminal input\n\n"
               "Options:\n"
               "\t-lx\t\tx can be an arbitrary number higher than 0, it sets `left_hyphen_min` for hyphenating process\n"
               "\t-rx\t\tx can be an arbitrary number higher than 0, it sets `right_hyphen_min` for hyphenating process\n"
               "\t-f file_name\tspecifies a file with words to be hyphenated, if not specified, words from terminal will be hyphenated\n";

bool command_parser(const char *word, int read)
{
    if (read < 2)
        return false;

    if (word[1] == 'q')
        return true;

    if (word[1] == 'l')
    {
        left_hyphen_min = atoi(&word[2]);
        if (left_hyphen_min == 0)
            left_hyphen_min = 2;
        printf("left_hyphen_min has been changed to: %i\n", left_hyphen_min);
    }

    if (word[1] == 'r')
    {
        right_hyphen_min = atoi(&word[2]);
        if (right_hyphen_min == 0)
            right_hyphen_min = 2;
        printf("right_hyphen_min has been changed to: %i\n", right_hyphen_min);
    }

    return false;
}

void hyphenator(const char *file_name, Pvoid_t *pattern_judy)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *word = NULL;

    if (file_name != NULL)
    {
        fp = fopen(file_name, "r");
    }
    else
    {
        printf("Write words to be hyphenated:\n");
        fp = stdin;
    }

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

        // Skip if no word was loaded
        if (read == 0)
            continue;

        // commands from command line 
        if (line[0] == ':')
        {
            if (command_parser(line, read))
                break;

            continue;
        }

        word = add_dots_to_word(read, line);
        char *utf8_code = create_utf_array(word);
        char *judy_hyphenated = judy_hyphenate(word, pattern_judy, utf8_code);

        printf("%s\n", judy_hyphenated);
        free(judy_hyphenated);
        free(utf8_code);
    }

    fclose(fp);
    if (line)
        free(line);
    if (word)
        free(word);
}

int main(int argc, char **argv)
{
    // Check for valid size of judy's internal type
    assert(sizeof(Word_t) == sizeof(char *));

    char *patterns_filepath = NULL;
    char *words_filepath = NULL;

    int c;
    while ((c = getopt(argc, argv, "hvl:r:f:")) != -1)
        switch (c)
        {
        case 'h':
            printf("%s", usage);
            return 0;
        case 'l':
            left_hyphen_min = atoi(optarg);
            break;
        case 'r':
            right_hyphen_min = atoi(optarg);
            break;
        case 'v':
            verbose = true;
            break;
        case 'f':
            words_filepath = optarg;
            break;
        default:
            fprintf(stderr, "Unknown option `\\x%x'.\n", optopt);
            return 1;
        }

    if (left_hyphen_min < 1)
        left_hyphen_min = 1;

    if (right_hyphen_min < 1)
        right_hyphen_min = 1;

    if (argc - optind != 1)
    {
        fprintf(stderr, "Missing file paths\n");
        return 1;
    }
    patterns_filepath = argv[optind];

    // Load patterns
    Pattern_wrapper pattern_list;
    patterns_load(&pattern_list, patterns_filepath);

    // Creating judy data structure and inserting patterns
    Pvoid_t pattern_judy = (Pvoid_t)NULL;
    judy_insert_patterns(&pattern_list, &pattern_judy);

    hyphenator(words_filepath, &pattern_judy);

    // Destroying all data structures and freeing all of its memory
    Word_t freed_count;
    JSLFA(freed_count, pattern_judy);
    patterns_free(&pattern_list);

    return 0;
}