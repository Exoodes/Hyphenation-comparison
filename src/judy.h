#include <Judy.h>

#define _GNU_SOURCE
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 1000000 // define maximum string length

void insert(Pvoid_t *ruby_array, char *key, char *value);

Word_t find(Pvoid_t *ruby_array, char *key);