/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/heap.h>
#include <phlox/hash_table.h>


/*** Useful macro definitions ***/

/* returns address of hash node */
#define NEXT_ADDR(t, e) ((void *)(((uintptr_t)(e)) + (t)->node_offset))
/* returns hash node */
#define NEXT(t, e) ((void *)(*(uintptr_t *)NEXT_ADDR(t, e)))
/* add link to next hash node */
#define PUT_IN_NEXT(t, e, val) (*(uintptr_t *)NEXT_ADDR(t, e) = (uintptr_t)(val))
/* test flags */
#define TEST_FLAGS(a, flags) (((a)&(flags))==(flags))


/* Hash table flags */
enum {
    HASH_TABLE_FLAG_NONE   = 0x00,  /* No flags defined */
    HASH_TABLE_FLAG_STATIC = 0x01   /* Table is statically inited */
};


/* init new hash table */
hash_table_t hash_table_init(uint table_size, uintptr_t node_offset,
    hash_table_compare_t compare_func, hash_table_hasher_t hash_func)
{
    struct hash_table *ht;
    uint i;

    /* allocate memory for hash table structure */
    ht = (struct hash_table *)kmalloc(sizeof(struct hash_table));
    if(ht == NULL)
        return NULL;

    /* allocate hash table */
    ht->table = (hash_node_t *)kmalloc(sizeof(hash_node_t) * table_size);
    if(ht->table == NULL) {
        kfree(ht);
        return NULL;
    }

    /* init table control fields */
    for(i = 0; i < table_size; ++i)
        ht->table[i] = NULL;
    ht->table_size = table_size;
    ht->node_offset = node_offset;
    ht->num_elems = 0;
    ht->num_buckets = 0;
    ht->flags = 0;
    ht->compare_func = compare_func;
    ht->hash_func = hash_func;

    /* return table to caller */
    return ht;
}

/* init new table statically */
hash_table_t hash_table_init_static(void *area, size_t area_size, uintptr_t node_offset,
    hash_table_compare_t compare_func, hash_table_hasher_t hash_func)
{
    struct hash_table *ht = area;
    uint i;

    /* check size argument. size must be enought at least for 1 bucket table. */
    if(area_size < HASH_TABLE_STATIC_SIZE(1))
        return NULL;

    /* init table pointers */
    ht->table = (hash_node_t *)((uintptr_t)area + sizeof(struct hash_table));

    /* init table control fields */
    ht->table_size = (area_size - sizeof(struct hash_table)) / sizeof(hash_node_t);
    for(i = 0; i < ht->table_size; ++i)
        ht->table[i] = NULL;
    ht->node_offset = node_offset;
    ht->num_elems = 0;
    ht->num_buckets = 0;
    ht->flags = HASH_TABLE_FLAG_STATIC;
    ht->compare_func = compare_func;
    ht->hash_func = hash_func;

    /* return table to caller */
    return ht;
}

/* deinit hash table */
int hash_table_uninit(hash_table_t table)
{
    struct hash_table *ht = table;

    /* static hash tables cannot be deinited */
    if(TEST_FLAGS(ht->flags, HASH_TABLE_FLAG_STATIC))
        return 1;

    /* release memory */
    kfree(ht->table);
    kfree(ht);

    return 0;
}

/* resize hash table */
int hash_table_resize(hash_table_t table, uint new_size)
{
    struct hash_table *ht = table;
    struct hash_table tmp;
    struct hash_table_iterator itr;
    uint i;
    void *elem;

    /* check arguments */
    if(!new_size || ht->table_size == new_size ||
       TEST_FLAGS(ht->flags, HASH_TABLE_FLAG_STATIC))
        return 1;

    /* init temporary table */
    tmp = *ht;
    tmp.num_elems = 0;
    tmp.num_buckets = 0;
    tmp.table_size = new_size;
    tmp.table = (hash_node_t *)kmalloc(sizeof(hash_node_t) * new_size);
    if(tmp.table == NULL)
        return 1;

    for(i = 0; i<new_size; ++i)
        tmp.table[i] = NULL;

    /* iterate through the old table and copy
     * all elements to new table
     */
    itr.table = ht;
    hash_table_rewind(&itr);
    while( (elem = hash_table_next(&itr)) != NULL)
        hash_table_insert(&tmp, elem);

    /* free old hash table and set new control fields */
    kfree(ht->table);
    *ht = tmp;

    return 0;
}

/* insert element into table */
int hash_table_insert(hash_table_t table, void *elem)
{
    struct hash_table *ht = table;
    uint32 hash;

    /* compute hash */
    hash = ht->hash_func(elem, NULL, ht->table_size);

    /* prepend chain */
    PUT_IN_NEXT(ht, elem, ht->table[hash]);

    /* update bucket */
    if(ht->table[hash] == NULL)
        ht->num_buckets++;
    ht->table[hash] = elem;

    ht->num_elems++;

    return 0;
}

/* safe element insert */
int hash_table_insert_safe(hash_table_t table, void *elem)
{
    struct hash_table *ht = table;
    hash_node_t i;
    uint32 hash;

    /* compute hash */
    hash = ht->hash_func(elem, NULL, ht->table_size);

    /* search for element */
    for(i = ht->table[hash]; i != NULL; i = NEXT(ht, i))
        if(i == elem)
            return 1;

    /* prepend chain */
    PUT_IN_NEXT(ht, elem, ht->table[hash]);

    /* update hash table bucket */
    if(ht->table[hash] == NULL)
        ht->num_buckets++;
    ht->table[hash] = elem;

    ht->num_elems++;

    return 0;
}

/* remove element */
int hash_table_remove(hash_table_t table, void *elem)
{
    struct hash_table *ht = table;
    hash_node_t i, last_i;
    uint32 hash;

    /* compute element hash */
    hash = ht->hash_func(elem, NULL, ht->table_size);

    /* search for element and remove it */
    last_i = NULL;
    for(i = ht->table[hash]; i != NULL; last_i = i, i = NEXT(ht, i)) {
        if(i == elem) { /* Bingo! */
            /* update chain and bucket */
            if(last_i != NULL)
                PUT_IN_NEXT(ht, last_i, NEXT(ht, i));
            else
                ht->table[hash] = NEXT(ht, i);

            /* bookkeeping */
            if(ht->table[hash] == NULL)
                ht->num_buckets--;

            ht->num_elems--;

            return 0; /* ok */
        }
    }

    return 1; /* element not removed */
}

/* find an element by its address */
void *hash_table_find(hash_table_t table, const void *elem)
{
    struct hash_table *ht = table;
    hash_node_t i;
    uint32 hash;

    /* compute hash of element */
    hash = ht->hash_func(elem, NULL, ht->table_size);
    /* search for element within chain */
    for(i = ht->table[hash]; i != NULL; i = NEXT(ht, i)) {
        if(i == elem)
            return i;
    }

    return NULL; /* nothing was found */
}

/* look for element by its key */
void *hash_table_lookup(hash_table_t table, const void *key)
{
    struct hash_table *ht = table;
    hash_node_t i;
    uint32 hash;

    /* if no compare function specified - we can not proceed */
    if(ht->compare_func == NULL)
        return NULL;

    /* compute hash for key */
    hash = ht->hash_func(NULL, key, ht->table_size);
    /* search for element by its key */
    for(i = ht->table[hash]; i != NULL; i = NEXT(ht, i)) {
        if(ht->compare_func(i, key) == 0)
            return i;
    }

    return NULL; /* nothing was found */
}

/* open hash table for iterate operation */
hash_table_iterator_t hash_table_open(hash_table_t table)
{
    struct hash_table *ht = table;
    struct hash_table_iterator *i;

    /* allocate iterator struct */
    i = (struct hash_table_iterator *)kmalloc(sizeof(struct hash_table_iterator));
    if(i == NULL)
        return NULL;

    /* init iterator */
    i->table = ht;
    hash_table_rewind(i);

    return i;
}

/* close hash table iterator */
void hash_table_close(hash_table_iterator_t itr)
{
    kfree(itr);
}

/* rewind iterator to initial state */
void hash_table_rewind(hash_table_iterator_t itr)
{
    struct hash_table_iterator *i = itr;
    i->node = NULL;
    i->bucket = -1;
}

/* iterate to next element */
void *hash_table_next(hash_table_iterator_t itr)
{
    struct hash_table_iterator *i = itr;
    struct hash_table *ht = i->table;
    uint index;

    do {
        if(!i->node) { /* initial iterator state or end of chain */
            /* search for used bucket in table */
            for(index = (uint)(i->bucket + 1); index < ht->table_size; index++) {
                if(ht->table[index]) {
                    i->node = ht->table[index];
                    break;
                }
            }
            i->bucket = index; /* update current bucket */
        } else  /* step to next node in chain */
            i->node = NEXT(ht, i->node);
    } while(!i->node && (uint)i->bucket < ht->table_size);

    return i->node; /* return pointer to element */
}

/* iterate table using callback */
void hash_table_foreach(hash_table_t table, hash_table_foreach_t callback)
{
    struct hash_table_iterator i;
    void *elem;

    /* init local iterator*/
    i.table = table;
    hash_table_rewind(&i);

    /* call callback function for each element */
    while( (elem = hash_table_next(&i)) != NULL)
        callback(elem);
}

/* return table size */
uint hash_table_size(hash_table_t table)
{
    struct hash_table *ht = table;
    return ht->table_size;
}

/* return elements count */
uint hash_table_count(hash_table_t table)
{
    struct hash_table *ht = table;
    return ht->num_elems;
}

/* return used buckets count */
uint hash_table_buckets(hash_table_t table)
{
    struct hash_table *ht = table;
    return ht->num_buckets;
}


/*** String Hash Functions ***/


/* RS Hash Function */
uint32 hash_func_str_rs(const char* str)
{
    uint32 b    = 378551;
    uint32 a    = 63689;
    uint32 hash = 0;

    for( ; *str; ++str) {
        hash = hash * a + (*str);
        a    = a * b;
    }

    return hash;
}

/* JS Hash Function */
uint32 hash_func_str_js(const char* str)
{
    uint32 hash = 1315423911;

    for( ; *str; ++str)
        hash ^= ((hash << 5) + (*str) + (hash >> 2));

    return hash;
}

/* P. J. Weinberger Hash Function */
uint32 hash_func_str_pjw(const char* str)
{
    const uint32 BitsInUnsignedInt = (uint32)(sizeof(uint32) * 8);
    const uint32 ThreeQuarters     = (uint32)((BitsInUnsignedInt  * 3) / 4);
    const uint32 OneEighth         = (uint32)(BitsInUnsignedInt / 8);
    const uint32 HighBits          = (uint32)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
    uint32 hash              = 0;
    uint32 test              = 0;

    for( ; *str; ++str) {
        hash = (hash << OneEighth) + (*str);

        if((test = hash & HighBits) != 0)
            hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
    }

    return hash;
}

/* ELF Hash Function */
uint32 hash_func_str_elf(const char* str)
{
    uint32 hash = 0;
    uint32 x    = 0;

    for( ; *str; ++str) {
       hash = (hash << 4) + (*str);
       if((x = hash & 0xF0000000L) != 0)
           hash ^= (x >> 24);
       hash &= ~x;
    }

    return hash;
}

/* BKDR Hash Function */
uint32 hash_func_str_bkdr(const char* str)
{
    uint32 seed = 131; /* 31 131 1313 13131 131313 etc.. */
    uint32 hash = 0;

    for( ; *str; ++str)
        hash = (hash * seed) + (*str);

    return hash;
}

/* SDBM Hash Function */
uint32 hash_func_str_sdbm(const char* str)
{
    uint32 hash = 0;

    for( ; *str; ++str)
        hash = (*str) + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/* DJB Hash Function */
uint32 hash_func_str_djb(const char* str)
{
    uint32 hash = 5381;

    for( ; *str; ++str)
        hash = ((hash << 5) + hash) + (*str);

    return hash;
}

/* DEK Hash Function */
uint32 hash_func_str_dek(const char* str)
{
    uint32 hash = strlen(str);

    for( ; *str; ++str)
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);

    return hash;
}

/* BP Hash Function */
uint32 hash_func_str_bp(const char* str)
{
    uint32 hash = 0;

    for( ; *str; ++str)
        hash = hash << 7 ^ (*str);

    return hash;
}

/* FNV Hash Function */
uint32 hash_func_str_fnv(const char* str)
{
    const uint32 fnv_prime = 0x811C9DC5;
    uint32 hash      = 0;

    for( ; *str; ++str) {
        hash *= fnv_prime;
        hash ^= (*str);
    }

    return hash;
}

/* AP Hash Function */
uint32 hash_func_str_ap(const char* str)
{
    uint32 hash = 0xAAAAAAAA;
    uint32 i;

    for(i = 0 ; *str; ++str, ++i) {
      hash ^= ((i & 1) == 0) ? ( (hash <<  7) ^ ((*str) * (hash >> 3)) ) :
                               (~(((hash << 11) + (*str)) ^ (hash >> 5)) );
    }

    return hash;
}

/* Hash function stolen from NewOS */
uint32 hash_func_str_newos(const char *str)
{
    uint32 hash = 0;

    for( ; *str; ++str) {
        hash ^= hash >> 28;
        hash <<= 4;
        hash ^= *str;
    }

    return hash;
}


/*** Integer Hash Functions ***/


/* Thomas Wang' hash function */
uint32 hash_func_int_32shift(uint32 key)
{
    key = ~key + (key << 15);
    key =  key ^ (key >> 12);
    key =  key + (key << 2);
    key =  key ^ (key >> 4);
    key =  key * 2057;
    key =  key ^ (key >> 16);
    return key;
}

/* Thomas Wang' hash function */
uint32 hash_func_int_32shiftmult(uint32 key)
{
    uint32 c2 = 0x27d4eb2d; /* a prime or an odd constant */
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * c2;
    key = key ^ (key >> 15);
    return key;
}

/* Robert Jenkins' 32 bit integer hash function */
uint32 hash_func_int_rj(uint32 key)
{
    key = (key+0x7ed55d16) + (key<<12);
    key = (key^0xc761c23c) ^ (key>>19);
    key = (key+0x165667b1) + (key<<5);
    key = (key+0xd3a2646c) ^ (key<<9);
    key = (key+0xfd7046c5) + (key<<3);
    key = (key^0xb55a4f09) ^ (key>>16);
    return key;
}

/*
 * Hash function used in Java HashMap.
 *
 * This function ensures that hashCodes that differ only by
 * constant multiples at each bit position have a bounded
 * number of collisions (approximately 8 at default load factor).
 *
*/
uint32 hash_func_int_java(uint32 key)
{
    key ^= (key >> 20) ^ (key >> 12);
    return key ^ (key >> 7) ^ (key >> 4);
}
