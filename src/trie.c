#include "trie.h"
#include "patterns.h"
#include "utils.h"

#include <stdbool.h>

extern bool verbose;

void trie_insert_patterns(Pattern_wrapper *patterns, cp_trie *patricia_trie)
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

char *trie_hyphenate(char *word, cp_trie *cprops_patricia_trie, const char *utf8_code)
{
    char backup;
    int len = strlen_utf8(word);

    char hyph_code[len + 1];
    memset(hyph_code, 0, (len + 1) * sizeof(char));
    const char *pattern_code = NULL;

    if (verbose)
        printf("Hyphenating word '%s' with Trie:\n", word);

    for (int i = 1; i <= len; i++)
    {
        for (int j = 0; j <= len - i; j++)
        {
            backup = word[(int)utf8_code[j + i]];
            word[(int)utf8_code[j + i]] = '\0';

            pattern_code = cp_trie_exact_match(cprops_patricia_trie, &word[(int)utf8_code[j]]);

            if (pattern_code != NULL)
            {
                if (verbose)
                    printf("Subword '%s'\t\t was found - pattern code: ", &word[(int)utf8_code[j]]);

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