#include "utils.h"
#include "patterns.h"

#include <stdio.h>
#include <stdbool.h>

extern int left_hyphen_min;
extern int right_hyphen_min;
extern bool verbose;
extern char hyphenation_char;

char *add_dots_to_word(int len, char *word)
{
    char *result = calloc(len + 3, sizeof(char));
    if (result == NULL)
        return NULL;

    memset(result, 0, len + 3);
    result[0] = '.';
    strcat(result, word);
    result[len + 1] = '.';

    return result;
}

int strlen_utf8(const char *str)
{
    int c, i, ix, q;
    for (q = 0, i = 0, ix = strlen(str); i < ix; i++, q++)
    {
        c = (unsigned char)str[i];
        if (c >= 0 && c <= 127)
            i += 0;
        else if ((c & 0xE0) == 0xC0)
            i += 1;
        else if ((c & 0xF0) == 0xE0)
            i += 2;
        else if ((c & 0xF8) == 0xF0)
            i += 3;
        else
            return 0; // invalid utf8
    }
    return q;
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

int min(int a, int b)
{
    return (a > b) ? b : a;
}

char *hyphenate_from_code(char *word, char *code)
{
    int len = strlen(word);
    int len_utf = strlen_utf8(word);
    int hyphen_count = 0;

    // left_hyphen_min
    for (int i = 0; i < min(left_hyphen_min, len_utf) + 1; i++)
    {
        code[i] = 0;
    }

    // right_hyphen_min
    for (int i = 0; i < min(right_hyphen_min, len_utf) + 1; i++)
    {
        code[len_utf - i] = 0;
    }

    if (verbose)
        printf("Final hyphenation code: ");

    for (int i = 1; i < len_utf; i++)
    {
        if (verbose)
            printf("%i", code[i]);

        if (code[i] % 2 == 1)
            hyphen_count++;
    }

    if (verbose)
        printf("\n");

    char *result = calloc(len + hyphen_count + 1, sizeof(char));

    int result_index = 0;
    int code_index = 1;
    for (int i = 1; i < len - 1; i++)
    {
        if ((word[i] & 0xF8) == 0xF0 || (word[i] & 0xF0) == 0xE0 ||
            (word[i] & 0xE0) == 0xC0 || (word[i] >= 0 && word[i] <= 127))
        {
            if (code[code_index] % 2 == 1)
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