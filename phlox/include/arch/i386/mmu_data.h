/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_MMU_DATA_H_
#define _PHLOX_ARCH_I386_MMU_DATA_H_

#include <phlox/compiler.h>
#include <phlox/types.h>

/*
* Page Directory entry format
*
* 31                12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |                   | Avail-|G|P|0|A|P|P|U|R|P|
* |PAGE TABLE ADDRESS | able  | |S| | |C|W|/|/| |
* |                   |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    PCD - Page Cache Disable (486+)
* R/W - Readable/Writable (0/1)    A   - Accessed
* U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
* PWT - Page Write Throught (486+) G   - Global (P6+)
*/
typedef struct _PACKED {
    /* attributes */
    uint32 p:     1;
    uint32 rw:    1;
    uint32 us:    1;
    uint32 pwt:   1;
    uint32 pcd:   1;
    uint32 a:     1;
    uint32 zero0: 1;
    uint32 ps:    1;
    uint32 g:     1;
    uint32 avl:   3;
    /**************/
    uint32 base: 20;
} mmu_pde_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
} mmu_pde_raw;

/* union */
typedef union {
    mmu_pde_struc stru;
    mmu_pde_raw   raw;
} mmu_pde;


/*
* Page Table entry format
*
* 31                12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |                   | Avail-|G|0|D|A|P|P|U|R|P|
* |PAGE FRAME ADDRESS | able  | | | | |C|W|/|/| |
* |                   |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    PCD - Page Cache Disable (486+)
* R/W - Readable/Writable (0/1)    A   - Accessed
* U/S - User/Supervisor (1/0)      D   - Dirty
* PWT - Page Write Throught (486+) G   - Global (P6+)
*/
typedef struct _PACKED {
    /* attributes */
    uint32 p:     1;
    uint32 rw:    1;
    uint32 us:    1;
    uint32 pwt:   1;
    uint32 pcd:   1;
    uint32 a:     1;
    uint32 d:     1;
    uint32 zero0: 1;
    uint32 g:     1;
    uint32 avl:   3;
    /**************/
    uint32 base: 20;
} mmu_pte_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
} mmu_pte_raw;

/* union */
typedef union {
    mmu_pte_struc stru;
    mmu_pte_raw   raw;
} mmu_pte;


/*
* Page Directory entry format for 4Mb pages (P5+)
*
* 31       22 21    12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |PAGE FRAME|        | Avail-|G|P|D|A|P|P|U|R|P|
* | ADDRESS  |Reserved| able  | |S| | |C|W|/|/| |
* |          |        |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    A   - Accessed
* R/W - Readable/Writable (0/1)    D   - Dirty
* U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
* PWT - Page Write Throught (486+) G   - Global (P6+)
* PCD - Page Cache Disable (486+)
*/
typedef struct _PACKED {
    /* attributes      */
    uint32 p:          1;
    uint32 rw:         1;
    uint32 us:         1;
    uint32 pwt:        1;
    uint32 pcd:        1;
    uint32 a:          1;
    uint32 d:          1;
    uint32 ps:         1;   /* = 1 for page size = 4Mb, else see mmu_pde */
    uint32 g:          1;
    uint32 avl:        3;
    uint32 reserved0: 10;
    /*******************/
    uint32 base:      10;
} mmu_pse_pde_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
} mmu_pse_pde_raw;

/* union */
typedef union {
    mmu_pse_pde_struc stru;
    mmu_pse_pde_raw   raw;
} mmu_pse_pde;


/** Paging with Physical Address Extension (P6+) **/

/*
* Page Directory Pointer Table Entry (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P| | | |
* | of page dir.|AVAIL|  res  |C|W|res|1| +0
* |  (0..19)    |     | | | | |D|T| | | |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
* res   - Reserved
*/
typedef struct _PACKED {
    /* attributes      */
    uint32 p:          1; /* present bit is always set to 1 */
    uint32 reserved0:  2;
    uint32 pwt:        1;
    uint32 pcd:        1;
    uint32 reserved1:  4;
    uint32 avl:        3;
    /*******************/
    uint32 base:      24;
    uint32 reserved2: 28;
} mmu_pae_pdpte_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} mmu_pae_pdpte_raw;

/* union */
typedef union {
    mmu_pae_pdpte_struc stru;
    mmu_pae_pdpte_raw   raw;
} mmu_pae_pdpte;


/*
* Page Directory Entry for 4K Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* | of page tab.|AVAIL|0|0|0|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*
*
* Page Directory Entry for 2M Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* |   of page   |AVAIL|G|1|D|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*/
typedef struct _PACKED {
    /* attributes      */
    uint32 p:          1;
    uint32 rw:         1;
    uint32 us:         1;
    uint32 pwt:        1;
    uint32 pcd:        1;
    uint32 a:          1;
    uint32 d:          1; /* always = 0 if ps = 0 */
    uint32 ps:         1;
    uint32 g:          1; /* always = 0 if ps = 0 */
    uint32 avl:        3;
    /*******************/
    uint32 base:      24;
    uint32 reserved0: 28;
} mmu_pae_pde_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} mmu_pae_pde_raw;

/* union */
typedef union {
    mmu_pae_pde_struc stru;
    mmu_pae_pde_raw   raw;
} mmu_pae_pde;


/*
* Page Table Entry for 4K Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* |   of page   |AVAIL|G|0|D|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*/
typedef struct _PACKED {
    /* attributes      */
    uint32 p:          1;
    uint32 rw:         1;
    uint32 us:         1;
    uint32 pwt:        1;
    uint32 pcd:        1;
    uint32 a:          1;
    uint32 d:          1;
    uint32 zero0:      1;
    uint32 g:          1;
    uint32 avl:        3;
    /*******************/
    uint32 base:      24;
    uint32 reserved0: 28;
} mmu_pae_pte_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} mmu_pae_pte_raw;

/* union */
typedef union {
    mmu_pae_pte_struc stru;
    mmu_pae_pte_raw   raw;
} mmu_pae_pte;

#endif
