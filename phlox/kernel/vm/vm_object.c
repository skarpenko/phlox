/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/param.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/vm_page.h>
#include <phlox/vm_private.h>
#include <phlox/vm.h>


/* Redefinition for convenience */
typedef xlist_t object_list_t;

/* Next available object id */
static vuint next_object_id;

/* Objects list */
static object_list_t objects_list;

/* AVL tree of objects for fast find by id */
static avl_tree_t objects_tree;

/* Spinlock for operations on objects list and tree */
static spinlock_t objects_lock;


/*** Locally used routines ***/

/* compare routine for objects AVL tree */
static int compare_object_id(const void *o1, const void *o2)
{
    if( ((vm_object_t *)o1)->id > ((vm_object_t *)o2)->id )
        return 1;
    if( ((vm_object_t *)o1)->id < ((vm_object_t *)o2)->id )
        return -1;
    return 0;
}

/* compare routine for universal pages tree */
static int compare_upage(const void *up1, const void *up2)
{
    if( ((vm_upage_t *)up1)->upn > ((vm_upage_t *)up2)->upn )
        return 1;
    if( ((vm_upage_t *)up1)->upn < ((vm_upage_t *)up2)->upn )
        return -1;
    return 0;
}

/* returns available object id */
static object_id get_next_object_id(void)
{
    object_id retval;

    /* atomically increment and get previous value */
    retval = (object_id)atomic_inc_ret(&next_object_id);
    if(retval == VM_INVALID_OBJECTID)
        panic("No available VM object IDs!");
    /* TODO: Implement better approach for reliability? */

    return retval;
}

/* put object to end of the list */
static void put_object_to_list(vm_object_t *object)
{
    unsigned long irqs_state;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* add item */
    xlist_add_last(&objects_list, &object->list_node);

    /* put object into tree */
    if(!avl_tree_add(&objects_tree, object))
      panic("put_object_to_list(): failed to add object into tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);
}

/* remove object from list */
static void remove_object_from_list(vm_object_t *object)
{
    unsigned long irqs_state;

    /* acquire lock before modifying list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* remove item */
    xlist_remove_unsafe(&objects_list, &object->list_node);

    /* and remove it from tree */
    if(!avl_tree_remove(&objects_tree, object))
      panic("remove_object_from_list(): failed to remove object from tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);
}

/* put upage into tree and list of the object keeping list sorted
 * (no lock acquired before)
 */
static bool put_upage_to_object(vm_object_t *object, vm_upage_t *upage)
{
    avl_tree_index_t where;
    vm_upage_t *parent;

    /* get "where" index and ensure we can add this upage */
    if(avl_tree_find(&object->upages_tree, upage, &where) != NULL)
        return false;

    /* put upage into tree */
    avl_tree_insert(&object->upages_tree, upage, where);

    /* if no other upages present - just add to list and exit */
    if(!where.node) {
        xlist_add_first(&object->upages_list, &upage->list_node);
        return true;
    }

    /* get parent node */
    parent = containerof(where.node, vm_upage_t, tree_node);

    /* add to proper position in list */
    if(!where.child) {
        xlist_insert_before(&object->upages_list, &parent->list_node,
                            &upage->list_node);
    } else {
        xlist_insert_after(&object->upages_list, &parent->list_node,
                           &upage->list_node);
    }

    return true;
}

/* adds all missing universal pages into object */
static bool add_all_upages_to_object(vm_object_t *object)
{
    vm_upage_t *dummy;
    addr_t offset;
    status_t err;

    /* walk through object and add upages */
    for(offset = 0; offset < object->size; offset += PAGE_SIZE) {
        err = vm_object_add_upage(object, offset, &dummy);
        ASSERT_MSG(err != ERR_NO_MEMORY, "add_all_upages_to_object(): no memory!");
        if(err == ERR_NO_MEMORY)
            return false;
    }

    return true;
}

/* removes universal page from memory object (no lock acquired before) */
static bool remove_upage_from_object(vm_object_t *object, vm_upage_t *upage)
{
    /* remove from the tree */
    if(avl_tree_remove(&object->upages_tree, upage) == NULL)
        return false;

    /* ... and from the list */
    if(!xlist_remove(&object->upages_list, &upage->list_node))
        return false;

    return true;
}

/* unwire single universal page
 * (no locks acquired, object must not be in use!)
*/
static void unwire_single_upage(vm_upage_t *upage)
{
    vm_page_t *ppage; /* physical page */

    ASSERT_MSG(upage->object->list_node.prev == NULL &&
        upage->object->list_node.next == NULL,
        "unwire_single_upage(): object is in use!");

    /* return if already unwired */
    if(upage->state == VM_UPAGE_STATE_UNWIRED)
        return;

    /* set free state to corresponding physical page */
    ppage = vm_page_lookup(upage->ppn);
    vm_page_set_state(ppage, VM_PAGE_STATE_FREE);
    /* set upage unwired */
    upage->ppn = 0;
    upage->state = VM_UPAGE_STATE_UNWIRED;
}

/* unwire all universal pages from object
 * (no locks acquired, object must not be in use!)
*/
static void unwire_upages_from_object(vm_object_t *object)
{
    vm_upage_t *upage;
    list_elem_t *iter;

    /* walk through list of universal pages and set them unwired */
    for(iter = object->upages_list.first; iter != NULL; iter = iter->next) {
        upage = containerof(iter, vm_upage_t, list_node);
        unwire_single_upage(upage);
    }
}

/* common routine for creating virtual memory objects */
static vm_object_t *create_object_common(const char *name, size_t size, uint protection)
{
    vm_object_t *object;

    /* object must have size */
    if(size == 0)
        return NULL;

    /* allocate object structure */
    object = (vm_object_t *)kmalloc(sizeof(vm_object_t));
    if(!object)
        return NULL;

    /* if creating named object - copy its name into structure field */
    if(name) {
        object->name = kstrdup(name);
        if(!object->name)
            goto error;
    } else {
        object->name = NULL;
    }

    /* init object structure fields */
    spin_init(&object->lock);
    object->size = PAGE_ALIGN(size);
    object->state = VM_OBJECT_STATE_NORMAL;
    object->protect = protection;
    object->flags = 0; /* currently not used */
    object->ref_count = 0;

    /* init bookkeeping structures */
    xlist_init(&object->mappings_list);
    xlist_init(&object->upages_list);
    avl_tree_create( &object->upages_tree, compare_upage,
                     sizeof(vm_upage_t),
                     offsetof(vm_upage_t, tree_node) );

    /* assign object id */
    object->id = get_next_object_id();

    /* return result to caller */
    return object;

error:
    /* release memory on error */
    if(object->name)
        kfree(object->name);
    kfree(object);

    return NULL; /* failed to create object */
}

/* common delete routine for memory objects.
 * releases all occupied memory by object structures.
 */
static void delete_object_common(vm_object_t *object)
{
    list_elem_t *item;
    vm_upage_t *upage;

    /* ensure that object is not in objects list */
    if(object->list_node.prev != NULL || object->list_node.next != NULL)
        panic("delete_object_common(): object still in list!");

    /* ensure that there is no mappings of this object */
    if(xlist_peek_first(&object->mappings_list) != NULL)
        panic("delete_object_common(): object has mappings!");

    /* check references count */
    if(object->ref_count != 0)
        panic("delete_object_common(): references count is not zero!");

    /* remove all universal pages */
    while( (item = xlist_peek_first(&object->upages_list)) != NULL ) {
        upage = containerof(item, vm_upage_t, list_node);
        if(upage->state != VM_UPAGE_STATE_UNWIRED)
            panic("delete_object_common(): upage with wired data!");
        /* remove from structures and free */
        remove_upage_from_object(object, upage);
        kfree(upage);
    }

    /* delete object structure */
    if(object->name)
        kfree(object->name);
    kfree(object);
}


/*** Public routines ***/

/* Module initialization routine */
status_t vm_objects_init(kernel_args_t *kargs)
{
   /* set initially available object id */
    next_object_id = 1;

    /* init spinlock for list access */
    spin_init(&objects_lock);

    /* init objects list */
    xlist_init(&objects_list);

    /* init objects AVL tree */
    avl_tree_create( &objects_tree, compare_object_id,
                     sizeof(vm_object_t),
                     offsetof(vm_object_t, tree_node) );

    return NO_ERROR;
}

/* create new upage at given offset and add it to object (no lock acquired before) */
status_t vm_object_add_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage)
{
    uint upn = PAGE_NUMBER(offset);

    /* check offset */
    if(offset >= object->size)
        return ERR_VM_BAD_OFFSET;

    /* allocate memory for new upage */
    *upage = (vm_upage_t *)kmalloc(sizeof(vm_upage_t));
    ASSERT_MSG(*upage != NULL, "vm_object_add_upage(): no memory!");
    if(*upage == NULL)
        return ERR_NO_MEMORY;

    /* init upage fields */
    (*upage)->upn    = upn;
    (*upage)->ppn    = 0;
    (*upage)->state  = VM_UPAGE_STATE_UNWIRED;
    (*upage)->object = object;
    xlist_elem_init(&(*upage)->list_node);

    /* try to put into object */
    if(!put_upage_to_object(object, *upage)) {
        kfree(*upage);
        return ERR_VM_UPAGE_EXISTS;
    }

    return NO_ERROR;
}

/* returns or creates upage at given offset (no lock acquired before) */
status_t vm_object_get_or_add_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage)
{
    uint upn = PAGE_NUMBER(offset);
    avl_tree_index_t where;
    vm_upage_t *look4, *parent;

    /* check offset */
    if(offset >= object->size)
        return ERR_VM_BAD_OFFSET;

    /* init search cookie */
    look4 = containerof(&upn, vm_upage_t, upn);
    /* .. and try to locate upage first */
    *upage = avl_tree_find(&object->upages_tree, look4, &where);
    if(*upage != NULL)
        return NO_ERROR;

    /* no upage here. so... allocate memory for new one */
    *upage = (vm_upage_t *)kmalloc(sizeof(vm_upage_t));
    ASSERT_MSG(*upage != NULL, "vm_object_get_or_add_upage(): no memory!");
    if(*upage == NULL)
        return ERR_NO_MEMORY;

    /* init upage fields */
    (*upage)->upn    = upn;
    (*upage)->ppn    = 0;
    (*upage)->state  = VM_UPAGE_STATE_UNWIRED;
    (*upage)->object = object;
    xlist_elem_init(&(*upage)->list_node);
    
    /* put it into tree */
    avl_tree_insert(&object->upages_tree, *upage, where);

    /* if no other upages present just add and exit */
    if(!where.node) {
        xlist_add_first(&object->upages_list, &(*upage)->list_node);
        return NO_ERROR;
    }

    /* get parent node */
    parent = containerof(where.node, vm_upage_t, tree_node);

    /* add upage to proper position in list */
    if(!where.child) {
        xlist_insert_before(&object->upages_list, &parent->list_node,
                            &(*upage)->list_node);
    } else {
        xlist_insert_after(&object->upages_list, &parent->list_node,
                           &(*upage)->list_node);
    }

    return NO_ERROR;
}

/* returns upage at given offset if exists (no lock acquired before) */
status_t vm_object_get_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage)
{
    uint upn = PAGE_NUMBER(offset);
    vm_upage_t *look4;

    /* check offset */
    if(offset >= object->size)
        return ERR_VM_BAD_OFFSET;

    /* fill cookie for search */
    look4 = containerof(&upn, vm_upage_t, upn);

    /* search for upage */
    *upage = avl_tree_find(&object->upages_tree, look4, NULL);
    if(*upage == NULL)
        return ERR_VM_NO_UPAGE;

    return NO_ERROR;
}

/* puts mapping wired with object into its mappings list (no lock acquired before) */
void vm_object_put_mapping(vm_object_t *object, vm_mapping_t *mapping)
{
    xlist_add_last(&object->mappings_list, &mapping->obj_list_node);
}

/* removes mapping wired with object from its mappings list (no lock acquired before) */
void vm_object_remove_mapping(vm_object_t *object, vm_mapping_t *mapping)
{
    if(!xlist_remove(&object->mappings_list, &mapping->obj_list_node))
        panic("vm_object_remove_mapping(): no mapping!");
}

/* create virtual memory object */
object_id vm_create_object(const char *name, size_t size, uint protection)
{
    vm_object_t *object;
    object_id id;

    /* check name length */
    if(name != NULL && strlen(name) > SYS_MAX_OS_NAME_LEN)
        return VM_INVALID_OBJECTID;

    /* check that name is unique */
    if(vm_find_object_by_name(name) != VM_INVALID_OBJECTID)
        return VM_INVALID_OBJECTID;

    /* create object */
    object = create_object_common(name, size, protection);
    if(!object)
        return VM_INVALID_OBJECTID;

    id = object->id;

    /* add to objects list */
    put_object_to_list(object);

    return id; /* return id */
}

/* create object with assigned physical memory chunk */
object_id vm_create_physmem_object(const char *name, addr_t phys_addr, size_t size, uint protection)
{
    vm_object_t *object;
    vm_page_t *pages, *page;
    uint page_num, first_page_num, n_pages;
    addr_t offset;
    vm_upage_t *upage;

    /* check name length */
    if(name != NULL && strlen(name) > SYS_MAX_OS_NAME_LEN)
        return VM_INVALID_OBJECTID;

    /* check that name is unique */
    if(vm_find_object_by_name(name) != VM_INVALID_OBJECTID)
        return VM_INVALID_OBJECTID;

    /* align address and size on page boundary */
    phys_addr = ROUNDOWN(phys_addr, PAGE_SIZE);
    size = PAGE_ALIGN(size);
    /* first physical page and pages count */
    first_page_num = PAGE_NUMBER(phys_addr);
    n_pages = PAGE_NUMBER(size);

    /* create object structure */
    object = create_object_common(name, size, protection);
    if(object == NULL)
        return VM_INVALID_OBJECTID;

    /* ... and add all upages to it */
    if(!add_all_upages_to_object(object))
        goto error;

    /* try to allocate specified physical pages range */
    pages = vm_page_alloc_specific_range(first_page_num, n_pages, VM_PAGE_STATE_FREE);
    if(pages == NULL)
        goto error;

    /* assign allocated physical pages to universal pages of the object */
    for(offset = 0, page_num = first_page_num, page = pages; offset < size;
      offset += PAGE_SIZE, page_num++, page++) {
        vm_object_get_upage(object, offset, &upage);
        ASSERT_MSG(upage != NULL, "vm_create_physmem_object(): upage = NULL!");
        /* physical page is now wired */
        vm_page_set_state(page, VM_PAGE_STATE_WIRED);
        upage->state = VM_UPAGE_STATE_RESIDENT;
        upage->ppn = page_num;
    }

    /* put object into bookkeeping structures */
    put_object_to_list(object);

    /* return id to caller */
    return object->id;

error:
    /* release memory on error */
    delete_object_common(object);

    return VM_INVALID_OBJECTID;
}

/* create object with assigned physical memory that is queried from virtual memory range */
object_id vm_create_virtmem_object(const char *name, aspace_id aid, addr_t virt_addr, size_t size, uint protection)
{
    vm_address_space_t *aspace;
    vm_object_t *object;
    vm_upage_t *upage;
    vm_page_t *page;
    list_elem_t *item;
    status_t err;
    addr_t offset;
    addr_t vaddr, paddr;
    uint flags;

    /* check name length */
    if(name != NULL && strlen(name) > SYS_MAX_OS_NAME_LEN)
        return VM_INVALID_OBJECTID;

    /* check that name is unique */
    if(vm_find_object_by_name(name) != VM_INVALID_OBJECTID)
        return VM_INVALID_OBJECTID;

    /* round to page boundary */
    virt_addr = ROUNDOWN(virt_addr, PAGE_SIZE);
    size = PAGE_ALIGN(size);

    /* get specified address space */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return VM_INVALID_OBJECTID;

    /* create object structures */
    object = create_object_common(name, size, protection);
    if(object == NULL) {
        vm_put_aspace(aspace);
        return VM_INVALID_OBJECTID;
    }

    /* lock translation map */
    aspace->tmap.ops->lock(&aspace->tmap);

    /* and start querying physical pages */
    for(offset = 0, vaddr = virt_addr; offset < size; offset += PAGE_SIZE, vaddr += PAGE_SIZE) {
        /* query physical page */
        aspace->tmap.ops->query(&aspace->tmap, vaddr, &paddr, &flags);
        /* page must present in address space */
        if( !(flags & VM_FLAG_PAGE_PRESENT) )
            goto error;
        /* add new universal page into object */
        err = vm_object_add_upage(object, offset, &upage);
        ASSERT_MSG(err != ERR_NO_MEMORY, "vm_create_virtmem_object(): no memory for upage!");
        if(err == ERR_NO_MEMORY)
            goto error;
        /* now allocate recieved physical page */
        page = vm_page_alloc_specific(PAGE_NUMBER(paddr), VM_PAGE_STATE_FREE);
        if(page == NULL)
            goto error;
        /* page is now be wired and assigned to upage of the object */
        vm_page_set_state(page, VM_PAGE_STATE_WIRED);
        upage->state = VM_UPAGE_STATE_RESIDENT;
        upage->ppn = page->ppn;
    }

    /* unlock translation map */
    aspace->tmap.ops->unlock(&aspace->tmap);
    /* ... and put address space back */
    vm_put_aspace(aspace);

    /* add object to bookkeeping structures */
    put_object_to_list(object);

    /* return its id to caller */
    return object->id;

error:
    /* releas lock and put address space back */
    aspace->tmap.ops->unlock(&aspace->tmap);
    vm_put_aspace(aspace);

    /* return allocated physical pages back */
    item = xlist_peek_first(&object->upages_list);
    while(item != NULL) {
        upage = containerof(item, vm_upage_t, list_node);
        upage->state = VM_UPAGE_STATE_UNWIRED;
        page = vm_page_lookup(upage->ppn);
        ASSERT_MSG(page != NULL, "vm_create_virtmem_object(): on error page is NULL!");
        vm_page_set_state(page, VM_PAGE_STATE_UNUSED);
        xlist_peek_next(item);
    }

    /* destroy object structures */
    delete_object_common(object);

    return VM_INVALID_OBJECTID;
}

/* delete object by its id */
status_t vm_delete_object(object_id oid)
{
    vm_object_t *object = vm_get_object_by_id(oid); /* get object */

    /* return error if object not found */
    if(object == NULL)
        return ERR_VM_INVALID_OBJECT;

    /* set deletion state */
    object->state = VM_OBJECT_STATE_DELETION;

    /* put object back, this can force actual destruction of the object */
    vm_put_object(object);

    /* return success */
    return NO_ERROR;
}

/* returns object by its id */
vm_object_t *vm_get_object_by_id(object_id oid)
{
    vm_object_t *search4, *object;
    unsigned long irqs_state;

    /* search cookie */
    search4 = containerof(&oid, vm_object_t, id);

    /* acquire lock before accessing tree */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* search tree */
    object = avl_tree_find(&objects_tree, search4, NULL);

    /* if object found and in proper state - increase references count */
    if(object && object->state == VM_OBJECT_STATE_NORMAL)
        atomic_inc(&object->ref_count);
    else
        object = NULL;

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);

    return object;
}

/* put previously taken object */
void vm_put_object(vm_object_t *object)
{
    /* decrease references count */
    atomic_dec(&object->ref_count);

    /* if state is DELETION and no more referers exists, start object
     * destruction stage or exit.
    */
    if(object->state != VM_OBJECT_STATE_DELETION || object->ref_count != 0)
        return;

    /* remove object from all control structures */
    remove_object_from_list(object);
    /* unwire all of its universal pages and free physical ones */
    unwire_upages_from_object(object);
    /* release memory occupied by object structures */
    delete_object_common(object);
}

/* returns object id by its name */
object_id vm_find_object_by_name(const char *name)
{
    unsigned long irqs_state;
    list_elem_t *item;
    vm_object_t *object;
    object_id id = VM_INVALID_OBJECTID;

    /* return invalid id if NULL passed */
    if(!name)
        return VM_INVALID_OBJECTID;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* start search from first list item */
    item = xlist_peek_first(&objects_list);
    while(item) {
        /* convert node to object structure */
        object = containerof(item, vm_object_t, list_node);

        /* we skip unnamed and in deletion state */
        if(object->state != VM_OBJECT_STATE_DELETION &&
           object->name && !strcmp(object->name, name))
        {
            id = object->id;
            break; /* search successful */
        }

        /* step to next item */
        item = xlist_peek_next(item);
    }

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);

    return id;
}
