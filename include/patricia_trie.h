/* --------------------------------------------------------------------------
 *    Name: patricia.h
 * Purpose: Associative array implemented as a PATRICIA trie
 * ----------------------------------------------------------------------- */

/* A PATRICIA trie is a form of trie where one-way branching is replaced with
 * a bit index and terminal nodes point back to previous nodes. */

#ifndef PATRICIA_H
#define PATRICIA_H

#include <stdio.h>

#include "errors.h"

/* ----------------------------------------------------------------------- */

/* Destroy the specified key. */
typedef void(patricia_destroy_key)(void *key);

/* Destroy the specified value. */
typedef void(patricia_destroy_value)(void *value);

/* ----------------------------------------------------------------------- */

typedef struct item
{
    /* we declare key and value const and cast it away as necessary */
    const void *key;
    size_t keylen;
    const void *value;
} item_t;

typedef struct patricia__node
{
    /* using an array here rather than separate left,right elements makes some
     * operations more convenient */
    struct patricia__node *child[2]; /* left, right children */
    int bit;                         /* critical bit */
    item_t item;
} patricia__node_t;

typedef struct patricia
{
    patricia__node_t *root;

    int count;

    const void *default_value;

    patricia_destroy_key *destroy_key;
    patricia_destroy_value *destroy_value;
} patricia;

/* ----------------------------------------------------------------------- */

error patricia_create(const void *default_value,
                      patricia_destroy_key *destroy_key,
                      patricia_destroy_value *destroy_value,
                      patricia **t);

void patricia_destroy(patricia *t);

/* ----------------------------------------------------------------------- */

const void *patricia_lookup(const patricia *t, const void *key, size_t keylen);

error patricia_insert(patricia *t,
                      const void *key,
                      size_t keylen,
                      const void *value);

/* ----------------------------------------------------------------------- */

/* Gets a byte or returns zero if it's out of range.
 * Copes with negative indices. */
// Have chosen to use size_t for 'unsigned ptrdiff_t'.
#define GET_BYTE(KEY, KEYEND, INDEX) \
    (((size_t)(INDEX) < (size_t)((KEYEND) - (KEY))) ? (KEY)[INDEX] : 0)

/* Extract the specified indexed binary direction from the key. */
#define GET_DIR(KEY, KEYEND, INDEX) \
    (KEY ? (GET_BYTE(KEY, KEYEND, INDEX >> 3) & (1 << (7 - ((INDEX)&7)))) != 0 : 0)

/* ----------------------------------------------------------------------- */

typedef unsigned int patricia_walk_flags;
#define patricia_WALK_ORDER_MASK (3u << 0)
#define patricia_WALK_IN_ORDER (0u << 0)
#define patricia_WALK_PRE_ORDER (1u << 0)
#define patricia_WALK_POST_ORDER (2u << 0)

#define patricia_WALK_LEAVES (1u << 2)
#define patricia_WALK_BRANCHES (1u << 3)
#define patricia_WALK_ALL (patricia_WALK_LEAVES | patricia_WALK_BRANCHES)

typedef error(patricia__walk_internal_callback)(patricia__node_t *n,
                                                int level,
                                                void *opaque);

/* ----------------------------------------------------------------------- */

#endif /* PATRICIA_H */
