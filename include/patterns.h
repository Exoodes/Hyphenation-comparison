#ifndef PATTERNS_H
#define PATTERNS_H

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

// Prints all of the patterns in format mention above
void patterns_print(Pattern_wrapper *patterns);

// Helper function for showing loaded pattern in better form
void patterns_show(Pattern_wrapper *patterns, int index);

/**
 * Function for loading patterns. This function allocate patterns data
 * structure, which must be freed later. Returns 0 if everything went ok,
 * return 1 if file was not opened correctly or allocation failed.
 */
int patterns_load(Pattern_wrapper *patterns, const char *file_name);

// Functions for freeing all patterns and freeing pattern wrapper
void patterns_free(Pattern_wrapper *patterns);

#endif // !PATTERNS_H
