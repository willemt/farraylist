/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "fixed_arraylist.h"

typedef struct
{
    int arraySize;
    int count;

//    tea_object_t obj;
//    tea_arrayqueue_t *openslotQueue;
#if HEAP_AVAILABLE
//    tea_heap_t *openslotHeap;
#endif

//    tea_hashmap_t *hash;        // special special case
//    tea_arrayqueue_t *hashslotQueue;
} alist_in_t;

#define INITIALCAPACITY 16
#define in(x) ((alist_in_t*)x->in)

/*----------------------------------------------------- OBJECT CLASSIFICATION */

#if HASHMAP_AVAILABLE
typedef struct
{
    int idx;
    void *data;
} hashslot_item_t;
#endif

#if HEAP_AVAILABLE

/**
 * for managing open slots */
typedef struct
{
    int idx;
} openslot_item_t;

static int openslot_compare(
    const void *e1,
    const void *e2
)
{
    const openslot_item_t *osi1 = e1, *osi2 = e2;

    return osi2->idx - osi1->idx;
}
#endif

arraylistf_t *arraylistf_new(
)
{
    arraylistf_t *alist;
    alist_in_t *list;

    alist = calloc(1, sizeof(arraylistf_t));
    alist->in = list = calloc(1, sizeof(alist_in_t));

    list->arraySize = INITIALCAPACITY;
    alist->array = calloc(list->arraySize, sizeof(void *));

#if HASHMAP_AVAILABLE
    list->hash = NULL;
    list->hashslotQueue = NULL;
#endif

#if HEAP_AVAILABLE
    list->openslotHeap = tea_heap_initalloc(&objOSI);
#endif

    return alist;
}

/* enlarge according to this proposed index. */
static void __ensurecap(
    arraylistf_t * alist,
    int idx
)
{
    void **array_new;

    int size_n, cnt, ii;

    assert(0 < in(alist)->arraySize);

    if (in(alist)->count < in(alist)->arraySize && idx < in(alist)->arraySize)
        return;

    size_n = in(alist)->arraySize * 2;  // new size

    /* check if requested 'size_n' is enough */
    if (idx >= size_n)
    {
        /* just barely give us enough space according to this idx. */
        size_n = idx + 1;
    }

    array_new = calloc(size_n, sizeof(void *));

    for (cnt = 0, ii = 0; ii < in(alist)->arraySize; ii++)
    {
        if (alist->array[ii])
            cnt++;
        array_new[ii] = alist->array[ii];
    }

    assert(cnt == arraylistf_count(alist));

    free(alist->array);
    alist->array = array_new;
    in(alist)->arraySize = size_n;
}

/**
 * When we hash, we might want to know that item's index on the arraylist.
 * we're using a malloc bucket for the hashslots.
 * we can decide to use hashslots or not here. */
#if HASHMAP_AVAILABLE
inline static void *__hashslot_wrap(
    arraylistf_t * alist,
    int idx,
    void *item
)
{
    /* getting at cache'd hashslot */
    if (in(alist)->hashslotQueue)
    {
        hashslot_item_t *hslot;

        hslot = tea_arrayqueue_poll(in(alist)->hashslotQueue);

        if (!hslot)
            hslot = tea_malloc(sizeof(hashslot_item_t));

        hslot->data = item;
        hslot->idx = idx;
        return hslot;
    }
    else
    {
//              assert(FALSE);
        return item;
    }
}
#endif

/**
 * Insert item at index */
void arraylistf_insert(
    arraylistf_t * alist,
    void *item,
    const int idx
)
{
    alist_in_t *list = alist->in;

    assert(item);
    assert(alist);

    __ensurecap(alist, idx);

#if HASHMAP_AVAILABLE
    /* OPTIONAL: add to HASHMAP */
    if (list->hash)
    {
        /* ONLY FROM DIRECT arraylistf_insert calls, in case we overwrite.. */
        if (alist->array[idx])
        {
            void *hslot;

            /* ..remove it since we don't have it anymore! */
            assert(tea_hashmap_containsKey(list->hash, item));

            hslot = tea_hashmap_remove(list->hash, alist->array[idx]);

            assert(hslot);
            assert(!tea_hashmap_containsKey(list->hash, item));
            assert(list->hashslotQueue);

            tea_arrayqueue_offer(list->hashslotQueue, hslot);
        }

        if (tea_hashmap_containsKey(list->hash, item))
        {
            tea_printf("ALREADY INSIDE\n");
            assert(FALSE);
        }

        tea_hashmap_put(list->hash, item, __hashslot_wrap(alist, idx, item));
    }
#endif

    if (!alist->array[idx])
    {
        list->count++;
    }

    alist->array[idx] = item;

#if HASHMAP_AVAILABLE
    if (list->hash)
    {
        assert(tea_hashmap_size(in(alist)->hash) == arraylistf_count(alist));
    }
#endif
}

/**
 * Is this probe valid?
 * @returns 1 on valid, 0 otherwise. */
inline static int __probecheck(
    arraylistf_t * alist,
    int idx
)
{
    assert(alist);

    if (in(alist)->arraySize <= idx)
        return 0;

    if (idx < 0)
        return 0;

    return 1;
}

/**
 *  @return size of memory used
 */
int arraylistf_get_arraysize(
    arraylistf_t * alist
)
{
    return in(alist)->arraySize;
}

/**
 * @return the object at this index. */
void *arraylistf_get(
    arraylistf_t * alist,
    const int idx
)
{
    assert(alist);

    if (!__probecheck(alist, idx))
        return NULL;

    return alist->array[idx];
}

/** remove and return the entry marked by idx. */
void *arraylistf_remove(
    arraylistf_t * alist,
    const int idx
)
{
    void *tmp;

    if (!__probecheck(alist, idx))
        return NULL;

    tmp = alist->array[idx];

    if (!tmp)
    {
        assert(0);
        return NULL;
    }

#if HASHMAP_AVAILABLE
    /* OPTIONAL:  remove from HASHMAP */
    if (in(alist)->hash)
    {
        hashslot_item_t *hslot;

        assert(tea_hashmap_size(in(alist)->hash) == arraylistf_count(alist));

        hslot = tea_hashmap_remove(in(alist)->hash, tmp);
        assert(hslot);
        assert(hslot->idx == idx);

        tea_arrayqueue_offer(in(alist)->hashslotQueue, hslot);
    }
#endif

#if HEAP_AVAILABLE
    openslot_item_t *ositem;

    /* goto the REMOVE-HEAP.. */
    ositem = tea_malloc(sizeof(openslot_item_t));
    /* ..idx so we can remember it */
    ositem->idx = idx;
    tea_heap_offer(in(alist)->openslotHeap, ositem);
#endif

//      tea_printf("removing %d\n", idx);

    alist->array[idx] = NULL;
    in(alist)->count--;

#if HASHMAP_AVAILABLE
    if (in(alist)->hash)
    {
        assert(tea_hashmap_size(in(alist)->hash) == arraylistf_count(alist));
    }
#endif
    return tmp;
}

/** Add this object to this alist.
 * @return the idx we added to. */
int arraylistf_add(
    arraylistf_t * alist,
    void *item
)
{
    int idx;

#if HEAP_AVAILABLE
    openslot_item_t *ositem;

    assert(item);

    /* use a remove'd openslotitem from our heap! */
    ositem = tea_heap_poll(in(alist)->openslotHeap);

    /* check if its inuse, (most probably because of _insert). */
    while (ositem && alist->array[ositem->idx])
    {
        free(ositem);

        /* try-again to find us an open slot! */

        if (tea_heap_isEmpty(in(alist)->openslotHeap))
        {
            ositem = NULL;
            break;
        }

        ositem = tea_heap_poll(in(alist)->openslotHeap);
    }

    if (ositem)
    {
        idx = ositem->idx;
        free(ositem);

        /* we've exhausted our search, add to the end */
    }
    else
    {
        idx = in(alist)->count;
        __ensurecap(alist, idx);
        assert(idx < in(alist)->arraySize);
        assert(!alist->array[idx]);
    }
#else
    for (idx = 0; idx < in(alist)->arraySize; idx++)
    {
        if (!alist->array[idx])
        {
            goto found_slot;
        }
    }
    /*  didn't find a slot, so increase space */
    idx = in(alist)->count;
    __ensurecap(alist, idx);

  found_slot:
#endif

    assert(!alist->array[idx]);

    /*
     * #if DEBUG
     * if (in(alist)->array[idx]) {
     * tea_printf("ERROR: adding to a used slot %d!\n", idx);
     * }
     * #endif
     */

    arraylistf_insert(alist, item, idx);

    return idx;
}

/** Empty this arraylist. */
void arraylistf_clear(
    arraylistf_t * alist
)
{
    int ii;

    for (ii = 0; ii < in(alist)->count; ii++)
    {
        alist->array[ii] = NULL;
    }
    in(alist)->count = 0;
}

/** Free the memory used by this alist. */
void arraylistf_free(
    arraylistf_t * alist
)
{
    alist_in_t *list = alist->in;

    arraylistf_free(alist);

#if HASHMAP_AVAILABLE
    if (list->hash)
        tea_hashmap_freeall(list->hash);
#endif

#if HEAP_AVAILABLE
    tea_heap_freeall(list->openslotHeap);
#endif
}

/**
 * @return number of items in this arraylist */
int arraylistf_count(
    arraylistf_t * alist
)
{
    assert(alist);
    return in(alist)->count;
}

/*------------------------------------------------------------------- HASHING */
#if HASHMAP_AVAILABLE
static void __alist_makehashtable(
    arraylistf_t * alist
)
{
    alist_in_t *list = alist->in;

    int ii = 0;

    if (NULL == list->hash)
    {
        list->hash = tea_hashmap_initalloc(&list->obj);
    }

    assert(tea_hashmap_isEmpty(list->hash));

    int end;

    end = arraylistf_get_arraysize(alist);
    for (ii = 0; ii < end; ii++)
    {
        void *tmp;

        tmp = alist->array[ii];

        if (tmp)
            tea_hashmap_put(list->hash, tmp, __hashslot_wrap(alist, ii, tmp));
    }
}

/** Is this item, represented by key inside this arraylist?
 * @returns 1 if it is, otherwise 0 (also 0 if the object doesn't have a hash code */
int arraylistf_contains(
    arraylistf_t * alist,
    const void *key
)
{
    alist_in_t *list = alist->in;

    if (NULL == list->obj.hash)
    {
        tea_msg(mSYS, 0, ERR_S "object needs a hash function for contains()\n");
        return 0;
    }

    /* build the hashtable */
    if (NULL == list->hash)
    {    // && list->obj.hash) {
        __alist_makehashtable(alist);
    }

    return tea_hashmap_containsKey(list->hash, key);

    /*
     * if (tea_hashmap_containsKey(list->hash,key)) {
     * return 1;
     * }
     * return 0;
     */
}

void *arraylistf_get_item(
    arraylistf_t * alist,
    const void *key
)
{
    alist_in_t *list = alist->in;

    assert(key);

    /* build the hashtable */
    if (NULL == list->hash && list->obj.hash)
    {
        __alist_makehashtable(alist);
    }

    /* we're using hashslots.. */
    if (list->hashslotQueue)
    {
        hashslot_item_t *hslot;

        hslot = tea_hashmap_get(list->hash, key);

        if (!hslot)
            return NULL;

        return hslot->data;
    }

    /* ..or not (straight hashmap) */
    return tea_hashmap_get(list->hash, key);
}

/** Init this hashslotQueue and rebuild the hash table. */
static void __hashslots_init(
    arraylistf_t * alist
)
{
    in(alist)->hashslotQueue = tea_arrayqueue_initalloc(NULL);

    if (!in(alist)->hash)
    {
        __alist_makehashtable(alist);
    }

    int size;

    size = tea_hashmap_size(in(alist)->hash);

    /* redo the hash with hashslots */
    tea_hashmap_clear(in(alist)->hash);
    __alist_makehashtable(alist);

    assert(size == tea_hashmap_size(in(alist)->hash));
}

void *arraylistf_remove_Item(
    arraylistf_t * alist,
    const void *key
)
{
    alist_in_t *list = alist->in;

    /* using a hashmap, in this case we start using hashslots */
    if (list->obj.hash)
    {
        hashslot_item_t *hslot;

        /* build the hashslot array */
        if (NULL == list->hashslotQueue)
        {
            __hashslots_init(alist);
        }

        /* do the REMOVE on the HASHMAP and ARRAYLIST */
        hslot = tea_hashmap_get(in(alist)->hash, key);

        if (NULL == hslot)
        {
            return NULL;
        }
//              assert(hslot);
//              list->array[hslot->idx] = NULL;
//              tea_arrayqueue_offer(list->hashslotQueue,hslot); // put back in hashslot bucket
//              list->count--;
//              return hslot->data;
        return arraylistf_remove(alist, hslot->idx);

        /* straight up linear compare */
    }
    else if (list->obj.compare)
    {
        int ii;

        for (ii = 0; ii < list->arraySize; ii++)
        {
            void *obj = arraylistf_get(alist, ii);

            if (obj && 0 == list->obj.compare(key, obj))
            {
                return arraylistf_remove(alist, ii);
            }
        }

        /* whoa, a linear memory address compare */
    }
    else
    {
        int ii;

        assert(FALSE);

        for (ii = 0; ii < list->arraySize; ii++)
        {
            void *obj = arraylistf_get(alist, ii);

            if (obj && key == obj)
            {
                return arraylistf_remove(alist, ii);
            }
        }
    }

    return NULL;
}

/** Whats the array index of this item? 
 * @returns -1 if item doesn't exist */
int arraylistf_item_index(
    arraylistf_t * alist,
    const void *key
)
{
    alist_in_t *list = alist->in;

    /* using a hash, in this case we start using hashslots */
    if (list->obj.hash)
    {
        hashslot_item_t *hslot;

        /* build the hashslot array */
        if (NULL == list->hashslotQueue)
        {
            __hashslots_init(alist);
        }

        hslot = tea_hashmap_get(in(alist)->hash, key);

        if (hslot)
            return hslot->idx;
    }

    return -1;
}
#endif
