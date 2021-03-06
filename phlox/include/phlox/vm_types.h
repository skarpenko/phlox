/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_TYPES_H_
#define _PHLOX_VM_TYPES_H_

#include <phlox/ktypes.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/spinlock.h>
#include <phlox/arch/vm_translation_map.h>


/* Page of physical memory */
typedef struct vm_page {
    list_elem_t list_node;  /* Pages list node */
    uint ppn;               /* Physical number of the page */
    vuint wire_count;       /* Count of maps refered to page */
    uint type  : 2;         /* Page type */
    uint state : 4;         /* Page state */
} vm_page_t;

/* Page types */
enum {
    VM_PAGE_TYPE_PHYSICAL = 0 /* Physical page */
};

/* Page states */
enum {
    VM_PAGE_STATE_ACTIVE = 0,  /* Active page */
    VM_PAGE_STATE_INACTIVE,    /* Inactive page */
    VM_PAGE_STATE_FREE,        /* Free page */
    VM_PAGE_STATE_CLEAR,       /* Free and clear page */
    VM_PAGE_STATE_WIRED,       /* Wired page */
    VM_PAGE_STATE_BUSY,        /* Busy page */
    VM_PAGE_STATE_UNUSED       /* Unused or reserved page */
};

/* Virtual memory map */
typedef struct vm_memory_map {
    addr_t                   base;           /* Map base address */
    size_t                   size;           /* Map size */
    struct vm_address_space *aspace;         /* Parent address space */
    xlist_t                  mappings_list;  /* Mappings list */
    avl_tree_t               mappings_tree;  /* Mappings AVL tree */
} vm_memory_map_t;

/* Mapping */
typedef struct vm_mapping {
    addr_t                start;          /* Start address */
    addr_t                end;            /* End address */
    struct vm_memory_map *mmap;           /* Parent memory map */
    struct vm_object     *object;         /* Mapped object */
    addr_t                offset;         /* Offset into object */
    uint                  type;           /* Mapping type */
    uint                  protect;        /* Memory protection attributes */
    list_elem_t           list_node;      /* Node of mappings list */
    avl_tree_node_t       tree_node;      /* Node of mappings AVL tree */
    list_elem_t           obj_list_node;  /* Node of mappings list in VM Object */
} vm_mapping_t;

/* Mapping types */
enum {
    VM_MAPPING_TYPE_HOLE = 0,  /* Memory hole (nothing is mapped) */
    VM_MAPPING_TYPE_OBJECT,    /* Mapped memory object */
    VM_MAPPING_TYPE_SUBMAP     /* Submap */
};

/* Address space */
typedef struct vm_address_space {
    aspace_id             id;            /* Address space ID */
    spinlock_t            lock;          /* Access lock */
    char                 *name;          /* Name (can be NULL) */
    int                   state;         /* Address space state */
    vuint                 ref_count;     /* Reference count */
    vuint                 faults_count;  /* Page faults count */
    vm_translation_map_t  tmap;          /* Translation map */
    struct vm_memory_map  mmap;          /* Memory map */
    list_elem_t           list_node;     /* Node of address spaces list */
    avl_tree_node_t       tree_node;     /* Node of address spaces AVL tree */
} vm_address_space_t;

/* Address space states */
enum {
    VM_ASPACE_STATE_NORMAL = 0,  /* Normal state */
    VM_ASPACE_STATE_DELETION     /* Deleting address space */
};

/* Memory object */
typedef struct vm_object {
    object_id        id;             /* Object ID */
    spinlock_t       lock;           /* Access lock */
    char            *name;           /* Name (can be NULL) */
    size_t           size;           /* Size */
    int              state;          /* Object state */
    uint             protect;        /* Protection */
    uint             flags;          /* Flags */
    vuint            ref_count;      /* Reference count */
    xlist_t          upages_list;    /* Universal pages list */
    avl_tree_t       upages_tree;    /* Universal pages AVL tree */
    xlist_t          mappings_list;  /* Mappings list */
    list_elem_t      list_node;      /* Node of objects list */
    avl_tree_node_t  tree_node;      /* Node of objects AVL tree */
} vm_object_t;

/* Object states */
enum {
    VM_OBJECT_STATE_NORMAL = 0,  /* Normal state */
    VM_OBJECT_STATE_DELETION     /* Deleting object */
};

/* Object protection flags */
#define VM_OBJECT_PROTECT_NONE     0x00
#define VM_OBJECT_PROTECT_READ     0x01
#define VM_OBJECT_PROTECT_WRITE    0x02
#define VM_OBJECT_PROTECT_EXECUTE  0x04
#define VM_OBJECT_PROTECT_ALL      ( VM_OBJECT_PROTECT_READ | \
                                     VM_OBJECT_PROTECT_WRITE | \
                                     VM_OBJECT_PROTECT_EXECUTE )
#define VM_OBJECT_PROTECT_MASK     0x07

/* Universal page */
typedef struct vm_upage {
    uint              upn;        /* Universal page number within object */
    uint              ppn;        /* Refered physical page number */
    uint              state : 2;  /* Universal page state */
    struct vm_object *object;     /* Parent object */
    list_elem_t       list_node;  /* Node of universal pages list */
    avl_tree_node_t   tree_node;  /* Node of universal pages tree */
} vm_upage_t;

/* Universal page states */
enum {
    VM_UPAGE_STATE_UNWIRED = 0,   /* No data wired to page (Initial state) */
    VM_UPAGE_STATE_RESIDENT       /* Page data is resident */
};

#endif
