#include "judy.h"
#include "patterns.h"
#include "utils.h"

#include <stdbool.h>

// Necessary Judy settings
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 100     // define maximum string length

extern bool verbose;

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
        printf("Insertion in judy data structure   of %u patterns "
               "took %8.f miliseconds (%.3f per pattern)\n",
               patterns->count, DeltaUSec, DeltaUSec / patterns->count);
}

char *judy_hyphenate(char *word, Pvoid_t *pattern_judy, const char *utf8_code)
{
    char backup;
    int len = strlen_utf8(word);

    char hyph_code[len + 1];
    memset(hyph_code, 0, (len + 1) * sizeof(char));
    const char *pattern_code = NULL;
    Word_t *find_return = NULL;

    if (verbose)
        printf("Hyphenating word '%s' with Judy:\n", word);

    for (int i = 1; i <= len; i++)
    {
        for (int j = 0; j <= len - i; j++)
        {
            backup = word[(int)utf8_code[j + i]];
            word[(int)utf8_code[j + i]] = '\0';

            JSLG(find_return, *pattern_judy, (uint8_t *)&word[(int)utf8_code[j]]);

            if (find_return != NULL)
            {
                if (verbose)
                    printf("Subword '%s'\t\t was found - pattern code: ", &word[(int)utf8_code[j]]);

                pattern_code = (char *)*find_return;

                for (int k = 0; k <= i; k++)
                {
                    if (verbose)
                        printf("%i", pattern_code[k]);

                    if (pattern_code[k] > hyph_code[j + k])
                        hyph_code[j + k] = pattern_code[k];
                }

                if (verbose)
                    putchar('\n');
            }

            word[(int)utf8_code[j + i]] = backup;
        }
    }

    char *result = hyphenate_from_code(word, hyph_code);
    if (verbose)
        printf("Hyphenation result: '%s'\n\n", result);

    return result;
}