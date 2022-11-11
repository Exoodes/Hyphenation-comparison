#include "judy.h"

#include <Judy.h>

#define _GNU_SOURCE
#define JUDYERROR_SAMPLE 1 // use default Judy error handler
#define MAXLINELEN 1000000 // define maximum string length

void insert(Pvoid_t *judy_array, char *key, char *value)
{
    Word_t *PValue;
    JSLI(PValue, *judy_array, key);
    *PValue = (long unsigned int)value;
}

Word_t find(Pvoid_t *judy_array, char *key)
{
    Word_t *PValue;
    JSLG(PValue, *judy_array, key);
    if (PValue != NULL)
    {
        return *PValue;
    }
}