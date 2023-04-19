#ifndef COMPARE_H
#define COMPARE_H

/**
 * Simple parser for commands written on command line when hyphenating words
 */
bool command_parser(const char *word, int read);

/**
 * Hyphenate words from file or command line with patterns stored in Judy and
 * output them to the stdout
 */
void hyphenator(const char *file_name, Pvoid_t *pattern_judy);

#endif // !COMPARE_H