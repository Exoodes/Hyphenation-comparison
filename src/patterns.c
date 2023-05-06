#define _GNU_SOURCE

#include "patterns.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

void patterns_print(Pattern_wrapper *pattern_array)
{
    for (int i = 0; i < pattern_array->count; i++)
    {
        patterns_show(pattern_array, i);
    }
}

void patterns_show(Pattern_wrapper *pattern_array, int index)
{
    printf("%s - ", pattern_array->patterns[index].word);

    for (int i = 0; i <= strlen_utf8(pattern_array->patterns[index].word); i++)
        printf("%i", pattern_array->patterns[index].code[i]);

    printf("\n");
}

int patterns_load(Pattern_wrapper *pattern_array, const char *file_name)
{
    pattern_array->patterns = NULL;
    pattern_array->count = 0;

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
    pattern_array->patterns = realloc(pattern_array->patterns, allocated_count * sizeof(Pattern));
    if (pattern_array->patterns == NULL)
    {
        printf("Allocation error\n");
        return 1;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (allocated_count == pattern_array->count)
        {
            allocated_count *= 2;
            pattern_array->patterns = realloc(pattern_array->patterns, allocated_count * sizeof(Pattern));
            if (pattern_array->patterns == NULL)
            {
                printf("Allocation error\n");
                return 1;
            }
        }

        pattern_array->patterns[pattern_array->count].word = calloc(read, sizeof(char));
        if (pattern_array->patterns[pattern_array->count].word == NULL)
        {
            printf("Allocation error\n");
            return 1;
        }

        int index = 0;
        for (int i = 0; i < read; i++)
        {
            if (!isdigit(line[i]) && line[i] != '\n')
            {
                pattern_array->patterns[pattern_array->count].word[index] = line[i];
                index++;
            }
        }

        pattern_array->patterns[pattern_array->count].code =
            calloc(strlen_utf8(pattern_array->patterns[pattern_array->count].word) + 1, sizeof(char));
        if (pattern_array->patterns[pattern_array->count].code == NULL)
        {
            printf("Allocation error\n");
            return 1;
        }

        index = 0;
        for (int i = 0; i < read; i++)
        {
            if (isdigit(line[i]))
            {
                pattern_array->patterns[pattern_array->count].code[index] = line[i] - '0';
            }

            else if ((line[i] & 0xF8) == 0xF0 || (line[i] & 0xF0) == 0xE0 ||
                     (line[i] & 0xE0) == 0xC0 || (line[i] >= 0 && line[i] <= 127))
            {
                index++;
            }
        }

        pattern_array->count++;
    }

    fclose(fp);

    if (line)
        free(line);

    return 0;
}

void patterns_free(Pattern_wrapper *pattern_array)
{
    for (int i = 0; i < pattern_array->count; i++)
    {
        free(pattern_array->patterns[i].word);
        free(pattern_array->patterns[i].code);
    }

    free(pattern_array->patterns);
}