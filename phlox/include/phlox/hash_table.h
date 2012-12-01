/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_HASH_TABLE_H
#define _PHLOX_HASH_TABLE_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Hash table types definition */
typedef void* hash_node_t;
typedef void* hash_table_iterator_t;
typedef void* hash_table_t;

/* Compare function should compare the element with
 * the key, returning 0 if equal, other if not.
*/
typedef int (hash_table_compare_t)(const void *elem, const void *key);

/* Hasher function should calculate hash on either e or key,
 * depending on which one is not NULL.
*/
typedef uint32 (hash_table_hasher_t)(const void *elem, const void *key, uint range);

/* Foreach callback */
typedef void (hash_table_foreach_t)(void *elem);



/** Internally used types it is not recommended to use it directly **/

/* Hash table */
struct hash_table {
    hash_node_t *table;                   /* array of hash table buckets */
    uintptr_t node_offset;                /* hash node offset within element */
    uint table_size;                      /* total buckets count in table */
    uint num_elems;                       /* number of elements in table */
    uint num_buckets;                     /* number of used buckets */
    uint flags;                           /* flags */
    hash_table_compare_t *compare_func;   /* ptr to compare function */
    hash_table_hasher_t *hash_func;       /* ptr to hash function */
};

/* Hash table iterator */
struct hash_table_iterator {
    struct hash_table *table;  /* iterated table */
    int bucket;                /* current bucket */
    hash_node_t node;          /* current node */
};
/**  End of internal types definition **/


/*
 * Special macro for area size used in static table creation.
 * a - table size.
 * Macro result is a size of area required for table.
*/
#define HASH_TABLE_STATIC_SIZE(a) (sizeof(struct hash_table)+(a)*sizeof(hash_node_t))


/*
 * Init new hash table.
 * table_size - count of buckets in hash table;
 * node_offset - offset of hash table node within element;
 * compare_func - pointer to compare function;
 * hash_func - pointer to hasher function.
*/
hash_table_t hash_table_init(uint table_size, uintptr_t node_offset,
    hash_table_compare_t compare_func, hash_table_hasher_t hash_func);

/*
 * Init new hash table statically in predefined area.
 * area - pointer to area;
 * area_size - size of area wich can be determined with special macro.
*/
hash_table_t hash_table_init_static(void *area, size_t area_size, uintptr_t node_offset,
    hash_table_compare_t compare_func, hash_table_hasher_t hash_func);

/*
 * Deinit hash table.
*/
int hash_table_uninit(hash_table_t table);

/*
 * Resize hash table.
 * Static hash tables cannot be resized.
*/
int hash_table_resize(hash_table_t table, uint new_size);

/*
 * Insert new element into hash table.
 * Safe version of insert search for element before insert
 * to avoid duplicates.
*/
int hash_table_insert(hash_table_t table, void *elem);
int hash_table_insert_safe(hash_table_t table, void *elem);

/*
 * Remove element from hash table.
*/
int hash_table_remove(hash_table_t table, void *elem);

/*
 * Search in hash table for specified element.
*/
void *hash_table_find(hash_table_t table, const void *elem);

/*
 * Search in hash table for element with specified key.
*/
void *hash_table_lookup(hash_table_t table, const void *key);

/*
 * Routines for iterating hash table
*/
hash_table_iterator_t hash_table_open(hash_table_t table);
void hash_table_close(hash_table_iterator_t itr);
void hash_table_rewind(hash_table_iterator_t itr);
void *hash_table_next(hash_table_iterator_t itr);

/*
 * Call callback for each element in table.
*/
void hash_table_foreach(hash_table_t table, hash_table_foreach_t callback);

/*
 * Count of buckets in table.
*/
uint hash_table_size(hash_table_t table);

/*
 * Count of elements stored in table.
*/
uint hash_table_count(hash_table_t table);

/*
 * Count of used buckets.
*/
uint hash_table_buckets(hash_table_t table);


/*
 * Different hash functions that can be useful in
 * writing a hasher function for hash table.
*/


/*** String Hash Functions ***/


/* RS Hash Function */
uint32 hash_func_str_rs(const char* str);

/* JS Hash Function */
uint32 hash_func_str_js(const char* str);

/* P. J. Weinberger Hash Function */
uint32 hash_func_str_pjw(const char* str);

/* ELF Hash Function */
uint32 hash_func_str_elf(const char* str);

/* BKDR Hash Function */
uint32 hash_func_str_bkdr(const char* str);

/* SDBM Hash Function */
uint32 hash_func_str_sdbm(const char* str);

/* DJB Hash Function */
uint32 hash_func_str_djb(const char* str);

/* DEK Hash Function */
uint32 hash_func_str_dek(const char* str);

/* BP Hash Function */
uint32 hash_func_str_bp(const char* str);

/* FNV Hash Function */
uint32 hash_func_str_fnv(const char* str);

/* AP Hash Function */
uint32 hash_func_str_ap(const char* str);

/* Hash function stolen from NewOS */
uint32 hash_func_str_newos(const char *str);


/*** Integer Hash Functions ***/


/* Thomas Wang' hash functions */
uint32 hash_func_int_32shift(uint32 key);
uint32 hash_func_int_32shiftmult(uint32 key);

/* Robert Jenkins' 32 bit integer hash function */
uint32 hash_func_int_rj(uint32 key);

/* Hash function used in Java HashMap */
uint32 hash_func_int_java(uint32 key);


#ifdef __cplusplus
}
#endif

#endif /* _PHLOX_HASH_TABLE_H */
