#include "patricia_trie.h"
#include "utils.h"
#include "types.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

patricia__node_t *patricia__node_create(patricia *t,
                                        const void *key,
                                        size_t keylen,
                                        const void *value)
{
    patricia__node_t *n;

    n = malloc(sizeof(*n));
    if (n == NULL)
        return NULL;

    n->child[0] = NULL;
    n->child[1] = NULL;
    n->item.key = key;
    n->item.keylen = keylen;
    n->item.value = value;
    n->bit = 0xdeadbeef; /* expected to be overwritten subsequently */

    t->count++;

    return n;
}

error patricia_create(const void *default_value,
                      patricia_destroy_key *destroy_key,
                      patricia_destroy_value *destroy_value,
                      patricia **pt)
{
    patricia *t;

    *pt = NULL;

    t = malloc(sizeof(*t));
    if (t == NULL)
        return error_OOM;

    /* the root node is only used for an all-zero-bits key */
    t->root = patricia__node_create(t, NULL, 0, NULL);
    if (t->root == NULL)
        return error_OOM;

    t->root->child[0] = t->root; /* left child points to self (for root node) */
    t->root->bit = -1;

    t->default_value = default_value;
    t->destroy_key = destroy_key;
    t->destroy_value = destroy_value;

    t->count = 0;

    *pt = t;

    return error_OK;
}

void patricia__node_clear(patricia *t, patricia__node_t *n)
{
    if (t->destroy_key && n->item.key)
        t->destroy_key((void *)n->item.key); /* must cast away const */
    if (t->destroy_value && n->item.value)
        t->destroy_value((void *)n->item.value);
}

void patricia__node_destroy(patricia *t, patricia__node_t *n)
{
    patricia__node_clear(t, n);

    free(n);

    t->count--;
}

static error patricia__destroy_node(patricia__node_t *n,
                                    int level,
                                    void *opaque)
{
    NOT_USED(level);

    patricia__node_destroy(opaque, n);

    return error_OK;
}

static error patricia__walk_internal_in_order(patricia__node_t *n,
                                              patricia_walk_flags flags,
                                              int level,
                                              patricia__walk_internal_callback *cb,
                                              void *opaque)
{
    error err;
    int i;

    if (n == NULL)
        return error_OK;

    for (i = 0; i < 2; i++)
    {
        if (n->child[i] == NULL)
            continue;

        /* does this child point to a leaf? */
        if (n->child[i]->bit <= n->bit)
        {
            if ((flags & patricia_WALK_LEAVES) != 0)
            {
                err = cb(n->child[i], level, opaque);
                if (err)
                    return err;
            }
        }
        else
        {
            err = patricia__walk_internal_in_order(n->child[i], flags, level + 1, cb, opaque);
            if (err)
                return err;
        }

        /* self */
        if (i == 0 && (flags & patricia_WALK_BRANCHES) != 0) /* inbetween */
        {
            err = cb(n, level, opaque);
            if (err)
                return err;
        }
    }

    return error_OK;
}

static error patricia__walk_internal_pre_order(patricia__node_t *n,
                                               patricia_walk_flags flags,
                                               int level,
                                               patricia__walk_internal_callback *cb,
                                               void *opaque)
{
    error err;
    int i;

    if (n == NULL)
        return error_OK;

    /* self */
    if ((flags & patricia_WALK_BRANCHES) != 0)
    {
        err = cb(n, level, opaque);
        if (err)
            return err;
    }

    for (i = 0; i < 2; i++)
    {
        if (n->child[i] == NULL)
            continue;

        /* does this child point to a leaf? */
        if (n->child[i]->bit <= n->bit)
        {
            if ((flags & patricia_WALK_LEAVES) != 0)
            {
                err = cb(n->child[i], level, opaque);
                if (err)
                    return err;
            }
        }
        else
        {
            err = patricia__walk_internal_pre_order(n->child[i], flags, level + 1, cb, opaque);
            if (err)
                return err;
        }
    }

    return error_OK;
}

static error patricia__walk_internal_post_order(patricia__node_t *n,
                                                patricia_walk_flags flags,
                                                int level,
                                                patricia__walk_internal_callback *cb,
                                                void *opaque)
{
    error err;
    int i;

    if (n == NULL)
        return error_OK;

    for (i = 0; i < 2; i++)
    {
        if (n->child[i] == NULL)
            continue;

        /* does this child point to a leaf? */
        if (n->child[i]->bit <= n->bit)
        {
            if ((flags & patricia_WALK_LEAVES) != 0)
            {
                err = cb(n->child[i], level, opaque);
                if (err)
                    return err;
            }
        }
        else
        {
            err = patricia__walk_internal_post_order(n->child[i], flags, level + 1, cb, opaque);
            if (err)
                return err;
        }
    }

    /* self */
    if ((flags & patricia_WALK_BRANCHES) != 0)
    {
        err = cb(n, level, opaque);
        if (err)
            return err;
    }

    return error_OK;
}

error patricia__walk_internal(patricia *t,
                              patricia_walk_flags flags,
                              patricia__walk_internal_callback *cb,
                              void *opaque)
{
    error (*walker)(patricia__node_t * n,
                    patricia_walk_flags flags,
                    int level,
                    patricia__walk_internal_callback *cb,
                    void *opaque);

    if (t == NULL)
        return error_OK;

    switch (flags & patricia_WALK_ORDER_MASK)
    {
    default:
    case patricia_WALK_IN_ORDER:
        walker = patricia__walk_internal_in_order;
        break;

    case patricia_WALK_PRE_ORDER:
        walker = patricia__walk_internal_pre_order;
        break;

    case patricia_WALK_POST_ORDER:
        walker = patricia__walk_internal_post_order;
        break;
    }

    return walker(t->root, flags, 0, cb, opaque);
}

void patricia_destroy(patricia *t)
{
    (void)patricia__walk_internal(t,
                                  patricia_WALK_POST_ORDER |
                                      patricia_WALK_BRANCHES,
                                  patricia__destroy_node,
                                  t);

    free(t);
}

/* ----------------------------------------------------------------------- */

const patricia__node_t *patricia__lookup(const patricia__node_t *n,
                                         const void *key,
                                         size_t keylen)
{
    const unsigned char *ukey = key;
    const unsigned char *ukeyend = ukey + keylen;
    int i;

    /* We follow nodes until we hit a lower bit value than previously
     * encountered. Sedgewick has values increasing as we progress through the
     * tree. Other texts describe it the other way around with the values
     * descending but that requires known key lengths. */

    do
    {
        i = n->bit;
        n = n->child[GET_DIR(ukey, ukeyend, i)];
        assert(n != NULL);
    } while (n->bit > i); /* we encounter ascending bit indices */

    return n;
}

const void *patricia_lookup(const patricia *t,
                            const void *key,
                            size_t keylen)
{
    const patricia__node_t *n;

    assert(key != NULL);
    assert(keylen > 0);

    /* test for empty tree */
    n = t->root;
    if (n == NULL)
        return NULL;

    /* keys consisting of all zero bits always live in the root node */
    if (unlikely(iszero(key, keylen)))
        return n->item.value; /* found */

    n = patricia__lookup(n, key, keylen);

    assert(n != NULL);
    if (n->item.keylen == keylen && memcmp(n->item.key, key, keylen) == 0)
        return n->item.value; /* found */
    else
        return t->default_value; /* not found */
}

error patricia_insert(patricia *t,
                      const void *key,
                      size_t keylen,
                      const void *value)
{
    const unsigned char *ukey;
    const unsigned char *ukeyend;
    int bit;

    {
        patricia__node_t *q;
        const unsigned char *qkey;
        const unsigned char *qkeyend;

        q = t->root;
        assert(q != NULL);

        /* keys consisting of all zero bits always live in the root node */
        if (unlikely(iszero(key, keylen)))
            goto update;

        /* find closest node */
        q = (patricia__node_t *)patricia__lookup(q, key, keylen); /* we cast away const */
        assert(q != NULL);

        if (q->item.keylen == keylen && memcmp(q->item.key, key, keylen) == 0)
        {
        update:
            if (q->item.key == key)
            {
                /* existing key - just update the value */
                q->item.value = value;
            }
            else
            {
                patricia__node_clear(t, q);

                q->item.key = key;
                q->item.keylen = keylen;
                q->item.value = value;
            }

            return error_OK;
        }

        /* we've found a node which differs */

        qkey = q->item.key;
        qkeyend = qkey + q->item.keylen;

        ukey = key;
        ukeyend = ukey + keylen;

        /* locate the critical bit */

        bit = keydiffbit(qkey, qkeyend - qkey, ukey, ukeyend - ukey);
        // do i need the if (bit == -1) test?
    }

    /* insert new item */

    {
        patricia__node_t *newnode;
        patricia__node_t *n;
        int nbit;
        int parbit;
        patricia__node_t **pn;
        int newdir;

        /* allocate new node */

        newnode = patricia__node_create(t, key, keylen, value);
        if (newnode == NULL)
            return error_OOM;

        n = t->root;
        nbit = n->bit;
        do
        {
            parbit = nbit;
            pn = &n->child[GET_DIR(ukey, ukeyend, nbit)];
            n = *pn;
            nbit = n->bit;
        }
        /* while (we've not hit the bit we want && encountered ascending) */
        while (nbit < bit && nbit > parbit);

        /* 'n' now points to a node with a bit index greater than our intended
         * index, or to the head node */

        newnode->bit = bit;

        /* the set bit indexes the direction we take to arrive at ourselves */
        newdir = GET_DIR(ukey, ukeyend, bit);
        assert(newdir == 0 || newdir == 1);

        newnode->child[newdir] = newnode;
        newnode->child[!newdir] = n;

        *pn = newnode;
    }

    return error_OK;
}