/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001-2004, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ELF64_H
#define _PHLOX_ELF64_H

#include <phlox/elf.h>

/*** NOTE: For more details refer to Executable and Linking Format Specification ***/

typedef uint64  Elf64_Addr;     /* Unsigned program address */
typedef uint32  Elf64_Half;     /* Unsigned medium integer */
typedef uint16  Elf64_Quarter;
typedef uint64  Elf64_Off;      /* Unsigned file offset */
typedef uint64  Elf64_Size;
typedef int64   Elf64_Sword;    /* Signed large integer */
typedef uint64  Elf64_Word;     /* Unsigned large integer */

/* ELF header */
struct Elf64_Ehdr {
   /* The initial bytes mark the file as an object file and provide
    * machine-independent data with which to decode and interpret the
    * file's contents.
    */
    unsigned char   e_ident[EI_NIDENT];
    
   /* This member identifies the object file type.
    * Although the core file contents are unspecified, type ET_CORE is
    * reserved to mark the file. Values from ET_LOPROC through ET_HIPROC
    * (inclusive) are reserved for processor-specific semantics. Other
    * values are reserved and will be assigned to new object file types as
    * necessary.
    */
    Elf64_Quarter   e_type;

   /* This member's value specifies the required architecture for an
    * individual file.
    * Other values are reserved and will be assigned to new machines as
    * necessary. Processor-specific ELF names use the machine name to
    * distinguish them. For example, the flags mentioned below use the
    * prefix EF_; a flag named WIDGET for the EM_XYZ machine would be
    * called EF_XYZ_WIDGET.
    */
    Elf64_Quarter   e_machine;
    
   /* This member identifies the object file version.
    * The value 1 signifies the original file format; extensions will
    * create new versions with higher numbers. The value of EV_CURRENT,
    * though given as 1 above, will change as necessary to reflect the
    * current version number.
    */
    Elf64_Half      e_version;

   /*
    * This member gives the virtual address to which the system first
    * transfers control, thus starting the process. If the file has no
    * associated entry point, this member holds zero.
    */
    Elf64_Addr      e_entry;

   /*
    * This member holds the program header table's file offset in bytes.
    * If the file has no program header table, this member holds zero.
    */
    Elf64_Off       e_phoff;

   /*
    * This member holds the section header table's file offset in bytes.
    * If the file has no section header table, this member holds zero.
    */
    Elf64_Off       e_shoff;

   /*
    * This member holds processor-specific flags associated with the file.
    * Flag names take the form EF_<machine>_<flag>.
    */
    Elf64_Half      e_flags;

   /*
    * This member holds the ELF header's size in bytes.
    */
    Elf64_Quarter   e_ehsize;

   /*
    * This member holds the size in bytes of one entry in the file's
    * program header table; all entries are the same size.
    */
    Elf64_Quarter   e_phentsize;

   /*
    * This member holds the number of entries in the program header
    * table. Thus the product of e_phentsize and e_phnum gives the table's
    * size in bytes. If a file has no program header table, e_phnum holds
    * the value zero.
    */
    Elf64_Quarter   e_phnum;

   /*
    * This member holds a section header's size in bytes. A section header
    * is one entry in the section header table; all entries are the same
    * size.
    */
    Elf64_Quarter   e_shentsize;

   /*
    * This member holds the number of entries in the section header table.
    * Thus the product of e_shentsize and e_shnum gives the section header
    * table's size in bytes. If a file has no section header table,
    * e_shnum holds the value zero.
    */
    Elf64_Quarter   e_shnum;

   /*
    * This member holds the section header table index of the entry
    * associated with the section name string table. If the file has no
    * section name string table, this member holds the value SHN_UNDEF.
    */
    Elf64_Quarter   e_shstrndx;
};

/* Section header */
struct Elf64_Shdr {
   /*
    * This member specifies the name of the section. Its value is an index
    * into the section header string table section, giving the location
    * of a null-terminated string.
    */
    Elf64_Half      sh_name;

   /*
    * This member categorizes the section's contents and semantics.
    */
    Elf64_Half      sh_type;

   /*
    * Sections support 1-bit flags that describe miscellaneous attributes.
    */
    Elf64_Size      sh_flags;

   /*
    * If the section will appear in the memory image of a process, this
    * member gives the address at which the section's first byte should
    * reside. Otherwise, the member contains 0.
    */
    Elf64_Addr      sh_addr;

   /*
    * This member's value gives the byte offset from the beginning of the
    * file to the first byte in the section. One section type, SHT_NOBITS
    * described below, occupies no space in the file, and its sh_offset
    * member locates the conceptual placement in the file.
    */
    Elf64_Off       sh_offset;

   /*
    * This member gives the section's size in bytes.  Unless the section
    * type is SHT_NOBITS, the section occupies sh_size bytes in the file.
    * A section of type SHT_NOBITS may have a non-zero size, but it
    * occupies no space in the file.
    */
    Elf64_Size      sh_size;

   /*
    * This member holds a section header table index link, whose
    * interpretation depends on the section type.
    */
    Elf64_Half      sh_link;

   /*
    * This member holds extra information, whose interpretation depends on
    * the section type.
    */
    Elf64_Half      sh_info;

   /*
    * Some sections have address alignment constraints. For example, if a
    * section holds a doubleword, the system must ensure doubleword
    * alignment for the entire section. That is, the value of sh_addr must
    * be congruent to 0, modulo the value of sh_addralign. Currently, only
    * 0 and positive integral powers of two are allowed. Values 0 and 1
    * mean the section has no alignment constraints.
    */
    Elf64_Size      sh_addralign;

   /*
    * Some sections hold a table of fixed-size entries, such as a symbol
    * table. For such a section, this member gives the size in bytes of
    * each entry. The member contains 0 if the section does not hold a
    * table of fixed-size entries.
    */
    Elf64_Size      sh_entsize;
};

/* Program header */
struct Elf64_Phdr {
   /*
    * This member tells what kind of segment this array element describes
    * or how to interpret the array element's information.
    */
    Elf64_Half      p_type;

   /*
    * This member gives flags relevant to the segment.
    */
    Elf64_Half      p_flags;

   /*
    * This member gives the offset from the beginning of the file at which
    * the first byte of the segment resides.
    */
    Elf64_Off       p_offset;

   /*
    * This member gives the virtual address at which the first byte of the
    * segment resides in memory.
    */
    Elf64_Addr      p_vaddr;

   /*
    * On systems for which physical addressing is relevant, this member is
    * reserved for the segment's physical address. Because System V
    * ignores physical addressing for application programs, this member
    * has unspecified contents for executable files and shared objects.
    */
    Elf64_Addr      p_paddr;

   /*
    * This member gives the number of bytes in the file image of the
    * segment; it may be zero.
    */
    Elf64_Size      p_filesz;

   /*
    * This member gives the number of bytes in the memory image of the
    * segment; it may be zero.
    */
    Elf64_Size      p_memsz;

   /*
    * As ``Program Loading'' later in this part describes, loadable
    * process segments must have congruent values for p_vaddr and
    * p_offset, modulo the page size. This member gives the value to which
    * the segments are aligned in memory and in the file. Values 0 and 1
    * mean no alignment is required. Otherwise, p_align should be a
    * positive, integral power of 2, and p_vaddr should equal p_offset,
    * modulo p_align.
    */
    Elf64_Size      p_align;
};

/* Symbol table entry */
struct Elf64_Sym {
   /*
    * This member holds an index into the object file's symbol string
    * table, which holds the character representations of the symbol
    * names. If the value is non-zero, it represents a string table index
    * that gives the symbol name. Otherwise, the symbol table entry has no
    * name.
    * NOTE: External C symbols have the same names in C and object files'
    * symbol tables.
    */
    Elf64_Half      st_name;

   /*
    * This member specifies the symbol's type and binding attributes.  A
    * list of the values and meanings appears below. The following code
    * shows how to manipulate the values.
    */
    unsigned char   st_info;

   /*
    * This member currently holds 0 and has no defined meaning.
    */
    unsigned char   st_other;

   /*
    * Every symbol table entry is ``defined'' in relation to some section;
    * this member holds the relevant section header table index.
    */
    Elf64_Quarter   st_shndx;

   /*
    * This member gives the value of the associated symbol. Depending on
    * the context, this may be an absolute value, an address, etc.
    */
    Elf64_Addr      st_value;

   /*
    * Many symbols have associated sizes. For example, a data object's
    * size is the number of bytes contained in the object. This member
    * holds 0 if the symbol has no size or an unknown size.
    */
    Elf64_Size      st_size;
};

/*
 * A symbol's binding determines the linkage visibility and behavior.
*/

#define ELF64_ST_BIND(i)     ((i) >> 4)
#define ELF64_ST_TYPE(i)     ((i) & 0xf)
#define ELF64_ST_INFO(b, t)  (((b) << 4) + ((t) & 0xf))

/* Relocation entries */
struct Elf64_Rel {
   /*
    * This member gives the location at which to apply the relocation
    * action. For a relocatable file, the value is the byte offset from
    * the beginning of the section to the storage unit affected by the
    * relocation. For an executable file or a shared object, the value is
    * the virtual address of the storage unit affected by the relocation.
    */
    Elf64_Addr  r_offset;

   /*
    * This member gives both the symbol table index with respect to which
    * the relocation must be made, and the type of relocation to apply.
    * For example, a call instruction's relocation entry would hold the
    * symbol table index of the function being called. If the index is
    * STN_UNDEF, the undefined symbol index, the relocation uses 0 as the
    * ``symbol value.'' Relocation types are processor-specific. When the
    * text refers to a relocation entry's relocation type or symbol table
    * index, it means the result of applying ELF64_R_TYPE or ELF64_R_SYM,
    * respectively, to the entry's r_info member.
    */
    Elf64_Size  r_info;
};

struct Elf64_Rela {
    Elf64_Addr  r_offset;
    Elf64_Size  r_info;

   /*
    * This member specifies a constant addend used to compute the value to
    * be stored into the relocatable field.
    */
    Elf64_Off   r_addend;
};

#define ELF64_R_SYM(i)      ((i) >> 32)
#define ELF64_R_TYPE(i)     ((unsigned char)(i))
#define ELF32_R_INFO(s, t)  (((s) << 32) + (unsigned char)(t))

/* Dynamic structure */
struct Elf64_Dyn {
   /* For each object with this type, d_tag controls the interpretation of
    * d_un.
    */
    Elf64_Size      d_tag;

    union {

       /*
        * These Elf64_Word objects represent integer values with various
        * interpretations.
        */
        Elf64_Size  d_val;

       /*
        * These Elf64_Addr objects represent program virtual addresses. As
        * mentioned previously, a file's virtual addresses might not match the
        * memory virtual addresses during execution. When interpreting
        * addresses contained in the dynamic structure, the dynamic linker
        * computes actual addresses, based on the original file value and the
        * memory base address. For consistency, files do not contain
        * relocation entries to ``correct'' addresses in the dynamic
        * structure.
        */
        Elf64_Addr  d_ptr;
    } d_un;
};

/* ELF specific types definitions */
typedef struct Elf64_Ehdr  Elf64_Ehdr_t;
typedef struct Elf64_Shdr  Elf64_Shdr_t;
typedef struct Elf64_Phdr  Elf64_Phdr_t;
typedef struct Elf64_Sym   Elf64_Sym_t;
typedef struct Elf64_Rel   Elf64_Rel_t;
typedef struct Elf64_Rela  Elf64_Rela_t;
typedef struct Elf64_Dyn   Elf64_Dyn_t;

#endif
