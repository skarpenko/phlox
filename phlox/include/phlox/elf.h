/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001-2004, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ELF_H
#define _PHLOX_ELF_H

#include <phlox/types.h>

/*** NOTE: For more details refer to Executable and Linking Format Specification ***/

#define ELF_MAGIC  "\x7f""ELF"
#define EI_MAG0        0   /* File identification */
#define EI_MAG1        1   /* File identification */
#define EI_MAG2        2   /* File identification */
#define EI_MAG3        3   /* File identification */
#define EI_CLASS       4   /* File class */
#define EI_DATA        5   /* Data encoding */
#define EI_VERSION     6   /* File version */
#define EI_OSABI       7   /* Operating system / ABI identification */
#define EI_ABIVERSION  8   /* ABI version */
#define EI_PAD         9   /* Start of padding bytes */
#define EI_NIDENT     16

/* Required architectures */
#define EM_NONE          0  /* No machine */
#define EM_M32           1  /* AT&T WE 32100 */
#define EM_SPARC         2  /* SPARC */
#define EM_386           3  /* Intel Architecture (Intel 80386) */
#define EM_68K           4  /* Motorola 68000 */
#define EM_88K           5  /* Motorola 88000 */
#define EM_486           6  /* Intel 80486 (Perhaps disused) */
#define EM_860           7  /* Intel 80860 */
#define EM_MIPS          8  /* MIPS RS3000 Big-Endian */
#define EM_MIPS_RS4_BE  10  /* MIPS RS4000 Big-Endian */
#define EM_PARISC       15  /* Hewlett-Packard PA-RISC */
#define EM_VPP500       17  /* Fujitsu VPP500 */
#define EM_SPARC32PLUS  18  /* Enhanced instruction set SPARC */
#define EM_960          19  /* Intel 80960 */
#define EM_PPC          20  /* PowerPC */
#define EM_PPC64        21  /* PowerPC 64bit */
#define EM_S390         22  /* IBM S/390 */
#define EM_V800         36  /* NEC V800 */
#define EM_FR20         37  /* Fujitsu FR20 */
#define EM_RH32         38  /* TRW RH-32 */
#define EM_RCE          39  /* Motorola RCE */
#define EM_ARM          40  /* Advanced RISC Machines (ARM) */
#define EM_SH           42  /* Hitachi SuperH */
#define EM_SPARCV9      43  /* SPARC Version 9 64-bit */
#define EM_TRICORE      44  /* Siemens Tricore embedded processor */
#define EM_ARC          45  /* Argonaut RISC Core */
#define EM_H8_300       46  /* Hitachi H8/300 */
#define EM_H8_300H      47  /* Hitachi H8/300H */
#define EM_H8S          48  /* Hitachi H8S */
#define EM_H8_500       49  /* Hitachi H8/500 */
#define EM_IA_64        50  /* HP/Intel Merced Processor 64-bit */
#define EM_MIPS_X       51  /* Stanford MIPS-X */
#define EM_COLDFIRE     52  /* Motorola Coldfire */
#define EM_68HC12       53  /* Motorola M68HC12 */
#define EM_X86_64       62  /* AMD x86-64 */
#define EM_CRIS         76  /* Axis Communications 32-bit embedded processor */
#define EM_V850         87  /* NEC V850 */
#define EM_M32R         88  /* Renesas M32R */
/*
 * This is an interim value that we will use until the committee comes
 * up with a final number.
 */
#define EM_FRV          0x5441  /* Fujitsu FR-V */
#define EM_ALPHA        0x9026  /* Digital Alpha */
/* Bogus old V850 magic number, used by old tools.  */
#define EM_CYGNUS_V850  0x9080
/* Bogus old m32r magic number, used by old tools.  */
#define EM_CYGNUS_M32R  0x9041
/*
 * This is the old interim value for S/390 architecture
 */
#define EM_S390_OLD     0xA390


#define ELFCLASSNONE  0  /* Invalid class */
#define ELFCLASS32    1  /* 32-bit objects */
#define ELFCLASS64    2  /* 64-bit objects */
#define ELFDATANONE   0  /* Invalid data encoding */
#define ELFDATA2LSB   1  /* LSB first */
#define ELFDATA2MSB   2  /* MSB first */
#define EV_NONE       0  /* Invalid version */
#define EV_CURRENT    1  /* Current version */

/* Object file type */
#define ET_NONE 0   /* No file type */
#define ET_REL  1   /* Relocatable file */
#define ET_EXEC 2   /* Executable file */
#define ET_DYN  3   /* Shared object file */
#define ET_CORE 4   /* Core file */
#define ET_LOPROC  0xff00  /* processor-specific */
#define ET_HIPROC  0xffff  /* processor-specific */

/* Section types */
#define SHT_NULL     0  /* Inactive section */
#define SHT_PROGBITS 1  /* Holds information defined by the programm */
#define SHT_SYMTAB   2  /* Holds symbol table */
#define SHT_STRTAB   3  /* Holds string table */
#define SHT_RELA     4  /* Relocation entries with explicit addends */
#define SHT_HASH     5  /* Symbol hash table */
#define SHT_DYNAMIC  6  /* Holds information for dynamic linking */
#define SHT_NOTE     7  /* Holds information that marks a file in some way */
#define SHT_NOBITS   8  /* This section occupies no space in file but
                         * otherwise resembles SHT_PROGBITS
                         */
#define SHT_REL      9  /* Relocation entries without explicit addends */
#define SHT_SHLIB   10  /* Reserved */
#define SHT_DYNSYM  11  /* Holds symbol table */
#define SHT_LOOS    0x60000000  /* Operating system-specific */
#define SHT_HIOS    0x6fffffff  /* Operating system-specific */
#define SHT_LOPROC  0x70000000  /* Reserved for processor-specific semantics */
#define SHT_HIPROC  0x7fffffff  /* Reserved for processor-specific semantics */
#define SHT_LOUSER  0x80000000  /* Lower bound of the range of indexes reserved
                                 * for application programms
                                 */
#define SHT_HIUSER  0xffffffff  /* Upper bound of the range of indexes reserved
                                 * for application programms
                                 */

/* Section attribute flags */
#define SHF_WRITE      0x1         /* Section contains data that should be writable */
#define SHF_ALLOC      0x2         /* Section occupies memory during process execution */
#define SHF_EXECINSTR  0x4         /* Section contains executable machine instructions */
#define SHF_TLS        0x400       /* Thread local storage segment */
#define SHF_MASKPROC   0xf0000000  /* Bits included in mask are reserved for
                                    * processor-specific semantics
                                    */

/* Segment flag bits */
#define PF_X           0x1         /* Execute */
#define PF_W           0x2         /* Write */
#define PF_R           0x4         /* Read */
#define PF_MASKPROC    0xf0000000  /* Unspecified */

/* Segment types */
#define PT_NULL     0           /* The array element is unused */
#define PT_LOAD     1           /* The array element specifies loadable segment */
#define PT_DYNAMIC  2           /* The array element specifies dynamic linking information */
#define PT_INTERP   3           /* The array element specifies the location and size of
                                 * null-terminated path name to invoke as an interpreter
                                 */
#define PT_NOTE     4           /* The array element specifies the location and size of
                                 * auxilary information
                                 */
#define PT_SHLIB    5           /* Segment type is reserved */
#define PT_PHDR     6           /* The array element, if present, specifies the location and
                                 * size of the program header table itself
                                 */
#define PT_TLS      7           /* Thread local storage segment */
#define PT_LOOS     0x60000000  /* Operating system-specific */
#define PT_HIOS     0x6fffffff  /* Operating system-specific */
#define PT_LOPROC   0x70000000  /* Reserved for processor-specific semantics */
#define PT_HIPROC   0x7fffffff  /* Reserved for processor-specific semantics */

/* Special sections indexes */
#define SHN_UNDEF      0       /* Undefined section reference */
#define SHN_LORESERVE  0xff00  /* Lower bound of the range of reserved indexes */
#define SHN_LOPROC     0xff00  /* Range reserved for processor-specific semantics */
#define SHN_HIPROC     0xff1f  /* Range reserved for processor-specific semantics */
#define SHN_ABS        0xfff1  /* Abs values for the corresponding reference */
#define SHN_COMMON     0xfff2  /* Section of common symbols */
#define SHN_HIRESERVE  0xffff  /* Upper bound of the range of reserved indexes */

/* Symbol types */
#define STT_NOTYPE   0  /* The symbol's type not specified */
#define STT_OBJECT   1  /* The symbol is associated with a data object
                         * (variable, array, and so on)
                         */
#define STT_FUNC     2  /* The symbol is associated with a function or other executable code */
#define STT_SECTION  3  /* The symbol is associated with a section */
#define STT_FILE     4  /* A file symbol has STB_LOCAL binding, its section index is SHN_ABS,
                         * and it precedes the other STB_LOCAL symbols for the file,
                         * if it ispresent
                         */
#define STT_TLS      6  /* Symbol is associated with thread local storage segment */
#define STT_LOPROC  13  /* Reserved for processor-specific semantics */
#define STT_HIPROC  15  /* Reserved for processor-specific semantics */

/* Symbol binding */
#define STB_LOCAL    0  /* Local symbols are not visible outside the object file */
#define STB_GLOBAL   1  /* Global symbols visible to all object files to be combined */
#define STB_WEAK     2  /* Weak symbols resemble global symbols, but their definitions
                         * have lower precedence
                         */
#define STB_LOPROC  13  /* Reserved for processor-specific semantics */
#define STB_HIPROC  15  /* Reserved for processor-specific semantics */

#define STN_UNDEF  0    /* Undefined symbol index */

/* Dynamic array tags */
#define DT_NULL              0  /* Marks end of dynamic array */
#define DT_NEEDED            1  /* This element holds null-terminated string, giving the name
                                 * of a needed library
                                 */
#define DT_PLTRELSZ          2  /* Total size, in bytes, of the relocation entries associated
                                 * with the procedure linkage table
                                 */
#define DT_PLTGOT            3  /* Holds address associated with the procedure linkage table
                                 * and/or the global offset table
                                 */
#define DT_HASH              4  /* Holds the address of the symbol hash table */
#define DT_STRTAB            5  /* Holds the address of the string table */
#define DT_SYMTAB            6  /* Holds the address of the symbol table */
#define DT_RELA              7  /* Holds the address of the relocation table */
#define DT_RELASZ            8  /* Total size, in bytes, of the DT_RELA relocation table */
#define DT_RELAENT           9  /* Holds the size, in bytes, of the DT_RELA relocation entry */
#define DT_STRSZ            10  /* Holds the size, in bytes, of the string table */
#define DT_SYMENT           11  /* Holds the size, in bytes, of the symbol table entry */
#define DT_INIT             12  /* Holds the address of the initialization function */
#define DT_FINI             13  /* Holds the address of the termination function */
#define DT_SONAME           14  /* Holds the string table offset of a null-terminated string,
                                 * giving the name of the shared object.
                                 */
#define DT_RPATH            15  /* Holds the string table offset of a null-terminated search
                                 * library search path string
                                 */
#define DT_SYMBOLIC         16  /* This element's presence in a shared object library alters the
                                 * dynamic linker's symbol resolution algorithm for references within
                                 * the library. Instead of starting a symbol search with the executable
                                 * file, the dynamic linker starts from the shared object itself.
                                 */
#define DT_REL              17  /* Similar DT_RELA, except its table has implicit addends */
#define DT_RELSZ            18  /* Total size, in bytes, of the DT_REL relocation table */
#define DT_RELENT           19  /* Holds the size, in bytes, of the DT_REL relocation entry */
#define DT_PLTREL           20  /* Specifies the type of relocation entry to which procedure
                                 * linkage table refers
                                 */
#define DT_DEBUG            21  /* Used for debugging */
#define DT_TEXTREL          22  /* This member's absence signifies that no relocation entry should
                                 * cause a modification to non-writable segment, as a specified
                                 * by the segment permissions in the program header table.
                                 */
#define DT_JMPREL           23  /* If present, this entries d_ptr member holds the address of
                                 * relocation entries associated solely with the procedure
                                 * linkage table.
                                 */
#define DT_BIND_NOW         24  /* If present in a shared object or executable, this entry instructs
                                 * the dynamic linker to process all relocations for the object
                                 * containing this entry before transferring control to the programm.
                                 */
#define DT_INIT_ARRAY       25  /* The address of an array of pointers to initialization functions. */
#define DT_FINI_ARRAY       26  /* The address of an array of pointers to termination functions. */
#define DT_INIT_ARRAYSZ     27  /* Size in bytes of DT_INIT_ARRAY */
#define DT_FINI_ARRAYSZ     28  /* Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH          29  /* null-terminated library search path string */
#define DT_FLAGS            30
#define DT_ENCODING         32
#define DT_PREINIT_ARRAY    32
#define DT_PREINIT_ARRAYSZ  33
#define DT_LOOS             0x6000000d  /* Operating system-specific */
#define DT_HIOS             0x6fff0000  /* Operating system-specific */
#define DT_LOPROC           0x70000000  /* Reserved for processor-specific semantics */
#define DT_HIPROC           0x7fffffff  /* Reserved for processor-specific semantics */

/*
 * i386 relocation types
 */
#define R_386_NONE      0
#define R_386_32        1
#define R_386_PC32      2
#define R_386_GOT32     3  /* This relocation type computes the distance from the base of the
                            * global offset table to the symbol's global offset table entry. It
                            * additionally instructs the link editor to build a global offset
                            * table.
                            */
#define R_386_PLT32     4  /* This relocation type computes the address of the symbol's procedure
                            * linkage table entry and additionally instructs the link editor to
                            * build a procedure linkage table.
                            */
#define R_386_COPY      5  /* The link editor creates this relocation type for dynamic linking.
                            * Its offset member refers to a location in a writable segment. The
                            * symbol table index specifies a symbol that should exist both in the
                            * current object file and in a shared object. During execution, the
                            * dynamic linker copies data associated with shared object's symbol to
                            * the location specified by the offset.
                            */
#define R_386_GLOB_DAT  6  /* This relocation type used to set a global offset table entry to the
                            * address of the specified symbol
                            */
#define R_386_JMP_SLOT  7  /* The link editor creates this relocation type for dynamic linking. Its
                            * offset member gives the location of a procedure linkage table entry.
                            */
#define R_386_RELATIVE  8  /* The link editor creates this relocation type for dynamic linking. Its
                            * offset member gives the location within a shared object that contains
                            * a value representing a relative address.
                            */
#define R_386_GOTOFF    9  /* This relocation type computes the difference between a symbol's value
                            * and the address of the global offset table.
                            */
#define R_386_GOTPC    10  /* This relocation type resembles R_386_PC32, except it uses the address
                            * of the global offset table in its calculation.
                            */

/*
 * x86-64 relocation types
 */
#define R_X86_64_NONE       0
#define R_X86_64_64         1
#define R_X86_64_PC32       2
#define R_X86_64_GOT32      3
#define R_X86_64_PLT32      4
#define R_X86_64_COPY       5
#define R_X86_64_GLOB_DAT   6
#define R_X86_64_JUMP_SLOT  7
#define R_X86_64_RELATIVE   8
#define R_X86_64_GOTPCREL   9
#define R_X86_64_32        10
#define R_X86_64_32S       11
#define R_X86_64_16        12
#define R_X86_64_PC16      13
#define R_X86_64_8         14
#define R_X86_64_PC8       15
#define R_X86_64_DPTMOD64  16
#define R_X86_64_DTPOFF64  17
#define R_X86_64_TPOFF64   18
#define R_X86_64_TLSGD     19
#define R_X86_64_TLSLD     20
#define R_X86_64_DTPOFF32  21
#define R_X86_64_GOTTPOFF  22
#define R_X86_64_TPOFF32   23

/*
 * sh4 relocation types
 */
#define R_SH_NONE                  0
#define R_SH_DIR32                 1
#define R_SH_REL32                 2
#define R_SH_DIR8WPN               3
#define R_SH_IND12W                4
#define R_SH_DIR8WPL               5
#define R_SH_DIR8WPZ               6
#define R_SH_DIR8BP                7
#define R_SH_DIR8W                 8
#define R_SH_DIR8L                 9
#define R_SH_SWITCH16             25
#define R_SH_SWITCH32             26
#define R_SH_USES                 27
#define R_SH_COUNT                28
#define R_SH_ALIGN                29

#define R_SH_CODE                 30
#define R_SH_DATA                 31
#define R_SH_LABEL                32
#define R_SH_SWITCH8              33
#define R_SH_GNU_VTINHERIT        34
#define R_SH_GNU_VTENTRY          35
#define R_SH_LOOP_START           36
#define R_SH_LOOP_END             37
#define R_SH_DIR5U                45
#define R_SH_DIR6U                46
#define R_SH_DIR6S                47
#define R_SH_DIR10S               48
#define R_SH_DIR10SW              49
#define R_SH_DIR10SL              50
#define R_SH_DIR10SQ              51
#define R_SH_GOT32               160
#define R_SH_PLT32               161
#define R_SH_COPY                162
#define R_SH_GLOB_DAT            163
#define R_SH_JMP_SLOT            164
#define R_SH_RELATIVE            165
#define R_SH_GOTOFF              166
#define R_SH_GOTPC               167
#define R_SH_GOTPLT32            168
#define R_SH_GOT_LOW16           169
#define R_SH_GOT_MEDLOW16        170
#define R_SH_GOT_MEDHI16         171
#define R_SH_GOT_HI16            172
#define R_SH_GOTPLT_LOW16        173
#define R_SH_GOTPLT_MEDLOW16     174
#define R_SH_GOTPLT_MEDHI16      175
#define R_SH_GOTPLT_HI16         176
#define R_SH_PLT_LOW16           177
#define R_SH_PLT_MEDLOW16        178
#define R_SH_PLT_MEDHI16         179
#define R_SH_PLT_HI16            180
#define R_SH_GOTOFF_LOW16        181
#define R_SH_GOTOFF_MEDLOW16     182
#define R_SH_GOTOFF_MEDHI16      183
#define R_SH_GOTOFF_HI16         184
#define R_SH_GOTPC_LOW16         185
#define R_SH_GOTPC_MEDLOW16      186
#define R_SH_GOTPC_MEDHI16       187
#define R_SH_GOTPC_HI16          188
#define R_SH_GOT10BY4            189
#define R_SH_GOTPLT10BY4         190
#define R_SH_GOT10BY8            191
#define R_SH_GOTPLT10BY8         192
#define R_SH_COPY64              193
#define R_SH_GLOB_DAT64          194
#define R_SH_JMP_SLOT64          195
#define R_SH_RELATIVE64          196
#define R_SH_SHMEDIA_CODE        242
#define R_SH_PT_16               243
#define R_SH_IMMS16              244
#define R_SH_IMMU16              245
#define R_SH_IMM_LOW16           246
#define R_SH_IMM_LOW16_PCREL     247
#define R_SH_IMM_MEDLOW16        248
#define R_SH_IMM_MEDLOW16_PCREL  249
#define R_SH_IMM_MEDHI16         250
#define R_SH_IMM_MEDHI16_PCREL   251
#define R_SH_IMM_HI16            252
#define R_SH_IMM_HI16_PCREL      253
#define R_SH_64                  254
#define R_SH_64_PCREL            255

/* PowerPC specific declarations */

/*
 * ppc relocation types
 */
#define R_PPC_NONE              0
#define R_PPC_ADDR32            1   /* 32bit absolute address */
#define R_PPC_ADDR24            2   /* 26bit address, 2 bits ignored.  */
#define R_PPC_ADDR16            3   /* 16bit absolute address */
#define R_PPC_ADDR16_LO         4   /* lower 16bit of absolute address */
#define R_PPC_ADDR16_HI         5   /* high 16bit of absolute address */
#define R_PPC_ADDR16_HA         6   /* adjusted high 16bit */
#define R_PPC_ADDR14            7   /* 16bit address, 2 bits ignored */
#define R_PPC_ADDR14_BRTAKEN    8
#define R_PPC_ADDR14_BRNTAKEN   9
#define R_PPC_REL24            10   /* PC relative 26 bit */
#define R_PPC_REL14            11   /* PC relative 16 bit */
#define R_PPC_REL14_BRTAKEN    12
#define R_PPC_REL14_BRNTAKEN   13
#define R_PPC_GOT16            14
#define R_PPC_GOT16_LO         15
#define R_PPC_GOT16_HI         16
#define R_PPC_GOT16_HA         17
#define R_PPC_PLTREL24         18
#define R_PPC_COPY             19
#define R_PPC_GLOB_DAT         20
#define R_PPC_JMP_SLOT         21
#define R_PPC_RELATIVE         22
#define R_PPC_LOCAL24PC        23
#define R_PPC_UADDR32          24
#define R_PPC_UADDR16          25
#define R_PPC_REL32            26
#define R_PPC_PLT32            27
#define R_PPC_PLTREL32         28
#define R_PPC_PLT16_LO         29
#define R_PPC_PLT16_HI         30
#define R_PPC_PLT16_HA         31
#define R_PPC_SDAREL16         32
#define R_PPC_SECTOFF          33
#define R_PPC_SECTOFF_LO       34
#define R_PPC_SECTOFF_HI       35
#define R_PPC_SECTOFF_HA       36
#define R_PPC_NUM              37

#endif
