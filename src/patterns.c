#include "patterns.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

void patterns_show(Pattern_wrapper *patterns, int index)
{
    printf("%s - ", patterns->patterns[index].word);

    for (int i = 0; i <= strlen(patterns->patterns[index].word); i++)
        printf("%i", patterns->patterns[index].code[i]);

    printf("\n");
}

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

void patterns_free(Pattern_wrapper *patterns)
{
    for (int i = 0; i < patterns->count; i++)
    {
        free(patterns->patterns[i].word);
        free(patterns->patterns[i].code);
    }

    free(patterns->patterns);
}