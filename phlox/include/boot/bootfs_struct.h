/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _BOOTFS_STRUCT_H_
#define _BOOTFS_STRUCT_H_


#define BOOTFS_MAGIC   "btfs"     /* Magic */
#define BOOTFS_TYPE    0x00010000 /* version=1, type=0 */
#define BOOTFS_NAMELEN 32

/* fs info block, first item of root directory */
typedef struct {
    unsigned int  magic;  /* Magic = "btfs"           */
    unsigned int  type;   /* FS type                  */
    unsigned int  psize;  /* Page size                */
    unsigned int  fsize;  /* FS size in pages         */
    unsigned int  rsize;  /* Root dir size in pages   */
    unsigned int  extra0;
    unsigned int  extra1;
    unsigned int  extra2;
    unsigned int  extra3;
    unsigned int  extra4;
    unsigned int  extra5;
    unsigned int  extra6;
    unsigned int  extra7;
    unsigned int  extra8;
    unsigned int  extra9;
    unsigned int  extraA;
} btfs_id;

/* directory item */
typedef struct {
    char          name[BOOTFS_NAMELEN]; /* name of loaded object, zero terminated              */
    unsigned int  offset;               /* offset of object relative to the start of bootfs    */
    unsigned int  type;                 /* object type designator                              */
    unsigned int  size;                 /* size of loaded object (pages)                       */
    unsigned int  vsize;                /* size loaded object should occupy when mapped in     */
    unsigned int  extra0;
    unsigned int  extra1;
    unsigned int  extra2;
    unsigned int  extra3;
} btfs_dir_entry;

#define OBJ_TYPE_NONE         0  /* empty entry                              */
#define OBJ_TYPE_DIRECTORY    1  /* directory                                */
#define OBJ_TYPE_BOOTSTRAP    2  /* bootstrap code object                    */
#define OBJ_TYPE_CODE         3  /* executable code object                   */
#define OBJ_TYPE_DATA         4  /* raw data object                          */
#define OBJ_TYPE_ELF32        5  /* 32bit ELF object                         */
#define OBJ_TYPE_ELF64        6  /* 64bit ELF object                         */

/* for OBJ_TYPE_BOOTSTRAP, OBJ_TYPE_CODE, OBJ_TYPE_ELF32 and OBJ_TYPE_ELF64  */
#define code_vaddr extra0 /* virtual address (rel offset 0)           */
#define code_ventr extra1 /* virtual entry point (rel offset 0)       */

#endif
