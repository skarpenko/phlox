/*
* Portions Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_AVL_TREE_H_
#define _PHLOX_AVL_TREE_H_


/*
 * This is a generic implemenatation of AVL trees for use in the kernel.
 * The interfaces provide an efficient way of implementing an ordered set of
 * data structures.
 *
 * AVL trees provide an alternative to using an ordered linked list. Using AVL
 * trees will usually be faster, however they requires more storage. An ordered
 * linked list in general requires 2 pointers in each data structure. The
 * AVL tree implementation uses 3 pointers. The following chart gives the
 * approximate performance of operations with the different approaches:
 *
 *  Operation    Link List  AVL tree
 *  ---------    --------   --------
 *  lookup         O(n)     O(log(n))
 *
 *  insert 1 node    constant   constant
 *
 *  delete 1 node    constant   between constant and O(log(n))
 *
 *  delete all nodes   O(n)     O(n)
 *
 *  visit the next
 *  or prev node     constant   between constant and O(log(n))
 *
 *
 * The data structure nodes are anchored at an "avl_tree_t" (the equivalent
 * of a list header) and the individual nodes will have a field of
 * type "avl_tree_node_t" (corresponding to list pointers).
 *
 * The type "avl_tree_index_t" is used to indicate a position in the list for
 * certain calls.
 *
 * The usage scenario is generally:
 *
 * 1. Create the list/tree with: avl_tree_create()
 *
 * followed by any mixture of:
 *
 * 2a. Insert nodes with: avl_tree_add(), or avl_tree_find() and
 *   avl_tree_insert()
 *
 * 2b. Visited elements with:
 *   avl_tree_first() - returns the lowest valued node
 *   avl_tree_last() - returns the highest valued node
 *   AVL_TREE_NEXT() - given a node go to next higher one
 *   AVL_TREE_PREV() - given a node go to previous lower one
 *
 * 2c.  Find the node with the closest value either less than or greater
 *  than a given value with avl_tree_nearest().
 *
 * 2d. Remove individual nodes from the list/tree with avl_tree_remove().
 *
 * and finally when the list is being destroyed
 *
 * 3. Use avl_tree_destroy_nodes() to quickly process/free up any remaining
 *  nodes.
 *    Note that once you use avl_tree_destroy_nodes(), you can no longer
 *    use any routine except avl_tree_destroy_nodes() and avl_tree_destoy().
 *
 * 4. Use avl_tree_destroy() to destroy the AVL tree itself.
 *
 * Any locking for multiple thread access is up to the user to provide, just
 * as is needed for any linked list implementation.
 */

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * The tree structure. The fields avl_root, avl_compare, and avl_offset come
 * first since they are needed for avl_tree_find().  We want them to fit into
 * a single 64 byte cache line to make avl_tree_find() as fast as possible.
 */
struct avl_tree {
    struct  avl_tree_node *avl_root;  /* root node in tree */
    int     (*avl_compare)(const void *, const void *); /* compare function */
    size_t  avl_offset;      /* offsetof(type, avl_tree_node_t field) */
    ulong_t avl_numnodes;    /* number of nodes in the tree */
    size_t  avl_size;        /* sizeof user type struct */
};

/*
 * There are 5 pieces of information stored for each node in an AVL tree
 *
 *  pointer to less than child
 *  pointer to greater than child
 *  a pointer to the parent of this node
 *  an indication  [0/1]  of which child I am of my parent
 *  a "balance" (-1, 0, +1)  indicating which child tree is taller
 */
struct avl_tree_node {
    struct avl_tree_node *avl_child[2];  /* left/right children */
    struct avl_tree_node *avl_parent;    /* this node's parent */
    unsigned short avl_child_index;      /* my index in parent's avl_child[] */
    short  avl_balance;                  /* balance value: -1, 0, +1 */
};

/*
 * An opaque structure used to locate a position in the tree.
 */
struct avl_tree_index {
    struct avl_tree_node *node;
    unsigned child;
};

/*
 * Type used for the root of the AVL tree.
 */
typedef struct avl_tree  avl_tree_t;

/*
 * The data nodes in the AVL tree must have a field of this type.
 */
typedef struct avl_tree_node  avl_tree_node_t;

/*
 * An opaque type used to locate a position in the tree where a node
 * would be inserted.
 */
typedef struct avl_tree_index  avl_tree_index_t;

/*
 * The type used in avl_tree_destroy_nodes()
 */
typedef struct avl_tree_index  avl_tree_cookie_t;

/*
 * Macro for first avl_tree_cookie_t initialization
 */
#define AVL_TREE_COOKIE_INIT(cookie)  {(cookie).node=NULL;(cookie).child=0;}

/*
 * Direction constants used for avl_tree_nearest().
 */
#define AVL_TREE_BEFORE  (0)
#define AVL_TREE_AFTER   (1)



/*
 * Prototypes
 *
 * Where not otherwise mentioned, "void *" arguments are a pointer to the
 * user data structure which must contain a field of type avl_tree_node_t.
 *
 * Also assume the user data structures looks like:
 *  stuct my_type {
 *      ...
 *      avl_tree_node_t  my_link;
 *      ...
 *  };
 */

/*
 * Initialize an AVL tree. Arguments are:
 *
 * tree    - the tree to be initialized
 * compare - function to compare two nodes, it must return exactly: -1, 0, or +1
 *          -1 for <, 0 for ==, and +1 for >
 * size   - the value of sizeof(struct my_type)
 * offset - the value of OFFSETOF(struct my_type, my_link)
 */
void avl_tree_create(avl_tree_t *tree, int (*compare) (const void *, const void *),
                     size_t size, size_t offset);


/*
 * Find a node with a matching value in the tree. Returns the matching node
 * found. If not found, it returns NULL and then if "where" is not NULL it sets
 * "where" for use with avl_tree_insert() or avl_tree_nearest().
 *
 * node   - node that has the value being looked for
 * where  - position for use with avl_tree_nearest() or avl_tree_insert(), may be NULL
 */
void *avl_tree_find(avl_tree_t *tree, void *node, avl_tree_index_t *where);


/*
 * Insert a node into the tree.
 *
 * node   - the node to insert
 * where  - position as returned from avl_tree_find()
 */
void avl_tree_insert(avl_tree_t *tree, void *node, avl_tree_index_t where);


/*
 * Insert "new_data" in "tree" in the given "direction" either after
 * or before the data "here".
 *
 * This might be usefull for avl clients caching recently accessed
 * data to avoid doing avl_tree_find() again for insertion.
 *
 * new_data - new data to insert
 * here     - existing node in "tree"
 * direction    - either AVL_TREE_AFTER or AVL_TREE_BEFORE the data "here".
 */
void avl_tree_insert_here(avl_tree_t *tree, void *new_data, void *here,
                          int direction);


/*
 * Return the first or last valued node in the tree. Will return NULL
 * if the tree is empty.
 *
 */
void *avl_tree_first(avl_tree_t *tree);
void *avl_tree_last(avl_tree_t *tree);


/*
 * This will only by used via AVL_TREE_NEXT() or AVL_TREE_PREV()
 * defined below.
 */
void *avl_tree_walk(avl_tree_t *tree, void *node, int direction);


/*
 * Return the next or previous valued node in the tree.
 * AVL_TREE_NEXT() will return NULL if at the last node.
 * AVL_TREE_PREV() will return NULL if at the first node.
 *
 * node   - the node from which the next or previous node is found
 */
#define AVL_TREE_NEXT(tree, node)  avl_tree_walk(tree, node, AVL_AFTER)
#define AVL_TREE_PREV(tree, node)  avl_tree_walk(tree, node, AVL_BEFORE)


/*
 * Find the node with the nearest value either greater or less than
 * the value from a previous avl_tree_find(). Returns the node or NULL if
 * there isn't a matching one.
 *
 * where     - position as returned from avl_tree_find()
 * direction - either AVL_TREE_BEFORE or AVL_TREE_AFTER
 *
 * EXAMPLE get the greatest node that is less than a given value:
 *
 *  avl_tree_t *tree;
 *  struct my_data look_for_value = {....};
 *  struct my_data *node;
 *  struct my_data *less;
 *  avl_tree_index_t where;
 *
 *  node = avl_tree_find(tree, &look_for_value, &where);
 *  if (node != NULL)
 *      less = AVL_TREE_PREV(tree, node);
 *  else
 *      less = avl_tree_nearest(tree, where, AVL_TREE_BEFORE);
 */
void *avl_tree_nearest(avl_tree_t *tree, avl_tree_index_t where, int direction);


/*
 * Add a single node to the tree.
 * The node must not be in the tree, and it must not
 * compare equal to any other node already in the tree.
 * Return the node added or NULL if failed.
 *
 * node   - the node to add
 */
void *avl_tree_add(avl_tree_t *tree, void *node);


/*
 * Remove a single node from the tree.  The node must be in the tree.
 * Return the node removed or NULL if failed
 *
 * node   - the node to remove
 */
void *avl_tree_remove(avl_tree_t *tree, void *node);


/*
 * Return the number of nodes in the tree
 */
ulong_t avl_tree_numnodes(avl_tree_t *tree);


/*
 * Used to destroy any remaining nodes in a tree. The cookie argument should
 * be initialized to NULL before the first call. Returns a node that has been
 * removed from the tree and may be free()'d. Returns NULL when the tree is
 * empty.
 *
 * Once you call avl_tree_destroy_nodes(), you can only continuing calling it
 * and finally avl_tree_destroy(). No other AVL routines will be valid.
 *
 * cookie - a "void *" used to save state between calls to avl_tree_destroy_nodes()
 *
 * EXAMPLE:
 *  avl_tree_t *tree;
 *  struct my_data *node;
 *  avl_tree_cookie_t *cookie;
 *
 *  AVL_TREE_INIT_COOKIE(cookie);
 *  while ((node = avl_tree_destroy_nodes(tree, &cookie)) != NULL)
 *      free(node);
 *  avl_tree_destroy(tree);
 */
void *avl_tree_destroy_nodes(avl_tree_t *tree, avl_tree_cookie_t *cookie);


/*
 * Final destroy of an AVL tree. Return the number of nodes in tree,
 * on success return value is 0.
 * Arguments are:
 *
 * tree   - the empty tree to destroy
 */
ulong_t avl_tree_destroy(avl_tree_t *tree);


#ifdef __cplusplus
}
#endif

#endif  /* _PHLOX_AVL_TREE_H_ */
