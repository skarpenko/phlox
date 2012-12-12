/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ELFFILE_H
#define _PHLOX_ELFFILE_H

#include <phlox/elf.h>
#include <phlox/elf32.h>
#include <phlox/elf64.h>


/* ELF library basic types */
typedef unsigned int elff_err_t;   /* Error code */
typedef unsigned int elff_size_t;  /* Size */
typedef unsigned int elff_off_t;   /* Offset */

/* ELF object constructor */
typedef elff_err_t  (elff_ctor_t)(void *pvt);
/* ELF object destructor */
typedef elff_err_t  (elff_dtor_t)(void *pvt);
/* ELF file read function */
typedef elff_size_t (elff_read_t)(void *pvt, elff_off_t off, elff_size_t size,
     void *buf);

/* Error codes */
enum elff_err_codes {
    ELFF_NO_ERR,   /* No error */
    ELFF_RD_ERR,   /* Read error */
    ELFF_NO_REC,   /* No record */
    ELFF_INV_REC   /* Invalid record */
};

/* ELF object structure */
typedef struct {
    elff_ctor_t  *ctor;  /* Pointer to constructor */
    elff_dtor_t  *dtor;  /* Pointer to destructor */
    elff_read_t  *read;  /* Pointer to read function */
    void         *pvt;   /* Private data */
} elff_obj_t;


/*
 * Create ELF File object
 *
 * Params:
 *   elf   - pointer to constructed object structure;
 *   ctor  - pointer to constructor (can be NULL);
 *   dtor  - pointer to destructor (can be NULL);
 *   read  - pointer to read function;
 *   pvt   - pointer to custom data (can be NULL).
*/
elff_err_t elff_create(elff_obj_t *elf, elff_ctor_t *ctor, elff_dtor_t *dtor,
     elff_read_t *read, void *pvt);

/*
 * Destroy ELF File object
 *
 * Params:
 *   elf  - pointer to elf object structure;
*/
elff_err_t elff_destroy(elff_obj_t *elf);

/*
 * Read ELF File identification
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   ident - array of length EI_NIDENT bytes.
*/
elff_err_t elff_read_ident(elff_obj_t *elf, unsigned char *ident);

/*
 * Check ELF File identification is valid
 *
 * Params:
 *   ident - array of length EI_NIDENT bytes.
*/
elff_err_t elff_check_ident(const unsigned char *ident);

/*
 * Read ELF File header
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   ehdr  - pointer to destination structure.
 */
elff_err_t elff_read_ehdr32(elff_obj_t *elf, Elf32_Ehdr_t *ehdr);
elff_err_t elff_read_ehdr64(elff_obj_t *elf, Elf64_Ehdr_t *ehdr);

/*
 * Read ELF File program header
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   idx   - program header index;
 *   phdr  - pointer to destination structure.
*/
elff_err_t elff_read_phdr32(elff_obj_t *elf, const Elf32_Ehdr_t *ehdr, elff_off_t idx,
     Elf32_Phdr_t *phdr);
elff_err_t elff_read_phdr64(elff_obj_t *elf, const Elf64_Ehdr_t *ehdr, elff_off_t idx,
     Elf64_Phdr_t *phdr);

/*
 * Read ELF File section header
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   idx   - program header index;
 *   shdr  - pointer to destination structure.
*/
elff_err_t elff_read_shdr32(elff_obj_t *elf, const Elf32_Ehdr_t *ehdr, elff_off_t idx,
     Elf32_Shdr_t *shdr);
elff_err_t elff_read_shdr64(elff_obj_t *elf, const Elf64_Ehdr_t *ehdr, elff_off_t idx,
     Elf64_Shdr_t *shdr);

/*
 * Read ELF File string table
 *
 * Params:
 *   elf     - pointer to elf file object;
 *   ehdr    - pointer to elf header;
 *   str_tbl - destination buffer.
*/
elff_err_t elff_read_strtbl32(elff_obj_t *elf, const Elf32_Ehdr_t *ehdr, char *str_tbl);
elff_err_t elff_read_strtbl64(elff_obj_t *elf, const Elf64_Ehdr_t *ehdr, char *str_tbl);

/*
 * Read ELF File program data
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   phdr  - pointer to elf program header;
 *   off   - offset within program data section;
 *   size  - bytes count to read;
 *   buf   - destination buffer.
*/
elff_size_t elff_read_pdata32(elff_obj_t *elf, const Elf32_Phdr_t *phdr, elff_off_t off,
    elff_size_t size, void *buf);
elff_size_t elff_read_pdata64(elff_obj_t *elf, const Elf64_Phdr_t *phdr, elff_off_t off,
    elff_size_t size, void *buf);

/*
 * Read ELF File section data
 *
 * Params:
 *   elf   - pointer to elf file object;
 *   shdr  - pointer to elf section header;
 *   off   - offset within program data section;
 *   size  - bytes count to read;
 *   buf   - destination buffer.
*/
elff_size_t elff_read_sdata32(elff_obj_t *elf, const Elf32_Shdr_t *shdr, elff_off_t off,
    elff_size_t size, void *buf);
elff_size_t elff_read_sdata64(elff_obj_t *elf, const Elf64_Shdr_t *shdr, elff_off_t off,
    elff_size_t size, void *buf);


#endif
