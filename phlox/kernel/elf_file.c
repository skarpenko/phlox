/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <stddef.h>
#include <phlox/elf_file.h>


/* ELF File create function */
elff_err_t elff_create(elff_obj_t *elf, elff_ctor_t *ctor, elff_dtor_t *dtor,
    elff_read_t *read, void *pvt)
{
    elf->ctor = ctor;
    elf->dtor = dtor;
    elf->read = read;
    elf->pvt  = pvt;

    if(elf->ctor)
        return elf->ctor(elf->pvt);
    else
        return ELFF_NO_ERR;
}

/* ELF File destroy function */
elff_err_t elff_destroy(elff_obj_t *elf)
{
    elff_err_t r = elf->dtor ? elf->dtor(elf->pvt) : ELFF_NO_ERR;
    
    elf->ctor = NULL;
    elf->dtor = NULL;
    elf->read = NULL;
    elf->pvt  = NULL;
    
    return r;
}

/* Read ELF File identification */
elff_err_t elff_read_ident(elff_obj_t *elf, unsigned char *ident)
{
    elff_size_t rd = elf->read(elf->pvt, 0, EI_NIDENT, ident);
    return rd == EI_NIDENT ? ELFF_NO_ERR : ELFF_RD_ERR;
}

/* Check ELF File identification */
elff_err_t elff_check_ident(const unsigned char *ident)
{
    /* Magic value */
    if(ident[EI_MAG0] != ELF_MAGIC[EI_MAG0]) return ELFF_INV_REC;
    if(ident[EI_MAG1] != ELF_MAGIC[EI_MAG1]) return ELFF_INV_REC;
    if(ident[EI_MAG2] != ELF_MAGIC[EI_MAG2]) return ELFF_INV_REC;
    if(ident[EI_MAG3] != ELF_MAGIC[EI_MAG3]) return ELFF_INV_REC;

    /* ELF class */
    if(ident[EI_CLASS] != ELFCLASS32 && ident[EI_CLASS] != ELFCLASS64)
        return ELFF_INV_REC;

    /* ELF data encoding */
    if(ident[EI_DATA] != ELFDATA2LSB && ident[EI_DATA] != ELFDATA2MSB)
        return ELFF_INV_REC;

    /* ELF version */
    if(ident[EI_VERSION] != EV_CURRENT)
        return ELFF_INV_REC;

    return ELFF_NO_ERR;
}

/* Read ELF File header */
#define DEF_ELFF_READ_EHDR_XX(B)                                                                \
elff_err_t elff_read_ehdr##B(elff_obj_t *elf, Elf##B##_Ehdr_t *ehdr)                            \
{                                                                                               \
    elff_size_t rd = elf->read(elf->pvt, 0, sizeof(Elf##B##_Ehdr_t), ehdr);                     \
    return rd == sizeof(Elf##B##_Ehdr_t) ? ELFF_NO_ERR : ELFF_RD_ERR;                           \
}

/* Read ELF File program header */
#define DEF_ELFF_READ_PHDR_XX(B)                                                                \
elff_err_t elff_read_phdr##B(elff_obj_t *elf, const Elf##B##_Ehdr_t *ehdr, elff_off_t idx,      \
    Elf##B##_Phdr_t *phdr)                                                                      \
{                                                                                               \
    elff_size_t rd;                                                                             \
    if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0 || ehdr->e_phentsize < sizeof(Elf##B##_Phdr_t)) \
        return ELFF_NO_REC;                                                                     \
    rd = elf->read(elf->pvt, ehdr->e_phoff + idx * ehdr->e_phentsize,                           \
            sizeof(Elf##B##_Phdr_t), phdr);                                                     \
    return rd == sizeof(Elf##B##_Phdr_t) ? ELFF_NO_ERR : ELFF_RD_ERR;                           \
}

/* Read ELF File section header */
#define DEF_ELFF_READ_SHDR_XX(B)                                                                \
elff_err_t elff_read_shdr##B(elff_obj_t *elf, const Elf##B##_Ehdr_t *ehdr, elff_off_t idx,      \
    Elf##B##_Shdr_t *shdr)                                                                      \
{                                                                                               \
    elff_size_t rd;                                                                             \
    if(ehdr->e_shoff == 0 || ehdr->e_shnum == 0 || ehdr->e_shentsize < sizeof(Elf##B##_Shdr_t)) \
        return ELFF_NO_REC;                                                                     \
    rd = elf->read(elf->pvt, ehdr->e_shoff + idx * ehdr->e_shentsize,                           \
            sizeof(Elf##B##_Shdr_t), shdr);                                                     \
    return rd == sizeof(Elf##B##_Shdr_t) ? ELFF_NO_ERR : ELFF_RD_ERR;                           \
}

/* Read ELF File string table */
#define DEF_ELFF_READ_STRTBL_XX(B)                                                              \
elff_err_t elff_read_strtbl##B(elff_obj_t *elf, const Elf##B##_Ehdr_t *ehdr, char *str_tbl)     \
{                                                                                               \
    Elf##B##_Shdr_t shdr;                                                                       \
    elff_size_t rd;                                                                             \
                                                                                                \
    if(ehdr->e_shstrndx == SHN_UNDEF)                                                           \
        return ELFF_NO_REC;                                                                     \
                                                                                                \
    rd = elf->read(elf->pvt, ehdr->e_shoff + ehdr->e_shstrndx * ehdr->e_shentsize,              \
            sizeof(Elf##B##_Shdr_t), &shdr);                                                    \
    if(rd != sizeof(shdr))                                                                      \
        return ELFF_RD_ERR;                                                                     \
                                                                                                \
    if(shdr.sh_type != SHT_STRTAB)                                                              \
        return ELFF_INV_REC;                                                                    \
                                                                                                \
    rd = elf->read(elf->pvt, shdr.sh_offset, shdr.sh_size, str_tbl);                            \
                                                                                                \
    return rd == shdr.sh_size ? ELFF_NO_ERR : ELFF_RD_ERR;                                      \
}

/* Read ELF File program data */
#define DEF_ELFF_READ_PDATA_XX(B)                                                               \
elff_size_t elff_read_pdata##B(elff_obj_t *elf, const Elf##B##_Phdr_t *phdr, elff_off_t off,    \
    elff_size_t size, void *buf)                                                                \
{                                                                                               \
    if(phdr->p_filesz == 0 || phdr->p_filesz <= off)                                            \
        return 0;                                                                               \
                                                                                                \
    size = ( (off+size-1) >= phdr->p_filesz ) ? phdr->p_filesz-off+1 : size;                    \
                                                                                                \
    return elf->read(elf->pvt, phdr->p_offset + off, size, buf);                                \
}

/* Read ELF File section data */
#define DEF_ELFF_READ_SDATA_XX(B)                                                               \
elff_size_t elff_read_sdata##B(elff_obj_t *elf, const Elf##B##_Shdr_t *shdr, elff_off_t off,    \
    elff_size_t size, void *buf)                                                                \
{                                                                                               \
    if(shdr->sh_size == 0 || shdr->sh_size <= off)                                              \
        return 0;                                                                               \
                                                                                                \
    size = ( (off+size-1) >= shdr->sh_size ) ? shdr->sh_size-off+1 : size;                      \
                                                                                                \
    return elf->read(elf->pvt, shdr->sh_offset + off, size, buf);                               \
}


/** Define functions **/

DEF_ELFF_READ_EHDR_XX(32)
DEF_ELFF_READ_EHDR_XX(64)

DEF_ELFF_READ_PHDR_XX(32)
DEF_ELFF_READ_PHDR_XX(64)

DEF_ELFF_READ_SHDR_XX(32)
DEF_ELFF_READ_SHDR_XX(64)

DEF_ELFF_READ_STRTBL_XX(32)
DEF_ELFF_READ_STRTBL_XX(64)

DEF_ELFF_READ_PDATA_XX(32)
DEF_ELFF_READ_PDATA_XX(64)

DEF_ELFF_READ_SDATA_XX(32)
DEF_ELFF_READ_SDATA_XX(64)
