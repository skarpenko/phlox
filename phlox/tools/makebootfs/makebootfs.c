/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
*
* Portions Copyright 2001-2004, Travis Geiselbrecht.
* Portions Copyright 1998 Brian J. Swetland.
*
* Distributed under the terms of the PhloxOS License.
*/


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <boot/bootfs_struct.h>
#define PAGE_SIZE 4096

#define DIR_ITEM_COUNT ( PAGE_SIZE / sizeof(btfs_dir_entry) )

#ifdef sparc
#define xBIG_ENDIAN 1
#endif
#ifdef i386
#define xLITTLE_ENDIAN 1
#endif
#ifdef __x86_64
#define xLITTLE_ENDIAN 1
#endif
#ifdef __ppc__
#define xBIG_ENDIAN 1
#endif
#ifdef __powerpc__
#define xBIG_ENDIAN 1
#endif

#define SWAP32(x) \
    ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#define SWAP64(x) \
    SWAP32((x) << 32) | SWAP32((x) >> 32)

#if xBIG_ENDIAN
#define HOST_TO_BENDIAN32(x) (x)
#define BENDIAN_TO_HOST32(x) (x)
#define HOST_TO_LENDIAN32(x) SWAP32(x)
#define LENDIAN_TO_HOST32(x) SWAP32(x)
#endif
#if xLITTLE_ENDIAN
#define HOST_TO_BENDIAN32(x) SWAP32(x)
#define BENDIAN_TO_HOST32(x) SWAP32(x)
#define HOST_TO_LENDIAN32(x) (x)
#define LENDIAN_TO_HOST32(x) (x)
#endif

#if !xBIG_ENDIAN && !xLITTLE_ENDIAN
#error not sure which endian the host processor is, please edit makebootfs.c
#endif


/* ELF stuff */
#define ELF_MAGIC "\x7f""ELF"
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_PAD 7
#define EI_NIDENT 16

#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* XXX not safe across all build architectures */
typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef int Elf32_Sword;
typedef unsigned int Elf32_Word;

struct Elf32_Ehdr {
    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half      e_type;
    Elf32_Half      e_machine;
    Elf32_Word      e_version;
    Elf32_Addr      e_entry;
    Elf32_Off       e_phoff;
    Elf32_Off       e_shoff;
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;
    Elf32_Half      e_phentsize;
    Elf32_Half      e_phnum;
    Elf32_Half      e_shentsize;
    Elf32_Half      e_shnum;
    Elf32_Half      e_shstrndx;
};

struct Elf32_Phdr {
    Elf32_Word      p_type;
    Elf32_Off       p_offset;
    Elf32_Addr      p_vaddr;
    Elf32_Addr      p_paddr;
    Elf32_Word      p_filesz;
    Elf32_Word      p_memsz;
    Elf32_Word      p_flags;
    Elf32_Word      p_align;
};

/* ELF64 stuff */
typedef unsigned long long Elf64_Addr;
typedef unsigned short Elf64_Quarter;
typedef unsigned int Elf64_Half;
typedef unsigned long long Elf64_Off;
typedef unsigned long long Elf64_Size;
typedef long long Elf64_Sword;
typedef unsigned long long Elf64_Word;

struct Elf64_Ehdr {
    unsigned char   e_ident[EI_NIDENT];
    Elf64_Quarter   e_type;
    Elf64_Quarter   e_machine;
    Elf64_Half      e_version;
    Elf64_Addr      e_entry;
    Elf64_Off       e_phoff;
    Elf64_Off       e_shoff;
    Elf64_Half      e_flags;
    Elf64_Quarter   e_ehsize;
    Elf64_Quarter   e_phentsize;
    Elf64_Quarter   e_phnum;
    Elf64_Quarter   e_shentsize;
    Elf64_Quarter   e_shnum;
    Elf64_Quarter   e_shstrndx;
};

struct Elf64_Phdr {
    Elf64_Half      p_type;
    Elf64_Half      p_flags;
    Elf64_Off       p_offset;
    Elf64_Addr      p_vaddr;
    Elf64_Addr      p_paddr;
    Elf64_Size      p_filesz;
    Elf64_Size      p_memsz;
    Elf64_Size      p_align;
};

#define LE 0
#define BE 1

static int target_endian = LE;

#define fix(x) ((target_endian == BE) ? HOST_TO_BENDIAN32(x) : HOST_TO_LENDIAN32(x))

/* Script parameter */
typedef struct _scr_param {
    struct _scr_param  *next;
    char               *name;
    char               *value;
} scr_param;

/* Script section */
typedef struct _scr_section {
    struct _scr_section  *next;
    char                 *name;
    scr_param            *plist;
} scr_section;

/* Directory page */
typedef struct _dir_page {
    struct _dir_page  *next;
    unsigned int      num;             /* page num */
    char              data[PAGE_SIZE]; /* page data */
} dir_page;

char *scr_data = NULL;
scr_section *scr_first = NULL;
scr_section *scr_last  = NULL;

btfs_id *btfs;
dir_page *dir_first = NULL;
dir_page *dir_last  = NULL;

void usage(char *module_name);
void error(char *fmt, ...);
void *loadfile(char *file, int *size, int *npages);
scr_section *loadscript(char *file);
char *scr_getval(scr_section *s, char *name);
char *scr_getvaldef(scr_section *s, char *name, char *def);
void extract_params(scr_section *section, char **name, unsigned int *reserv,
           unsigned int *type, char **file, unsigned int *vaddr, unsigned int *ventry);
void free_sections();
dir_page *alloc_dir_page(unsigned int num, unsigned int parent);
dir_page *dir_page_by_num(unsigned int num);
void free_pages();
void *alloc_buf(int size, int *newsize);
unsigned int path_itms_count(char *path);
unsigned int path_get_item(char *path, unsigned int idx, char *out, unsigned int size);
btfs_dir_entry *fs_alloc_file_entry(char *path, unsigned int *page_counter);
btfs_dir_entry *dir_alloc_entry(char *name, dir_page *page);
void mkbootfs(char *image);
Elf32_Off elf32_find_entry(void *buf, int size);
Elf64_Off elf64_find_entry(void *buf, int size);

/**********************************************/

int main(int argc, char **argv)
{
    char *script = NULL;
    char *image  = NULL;
    char *module = argv[0];

    if(argc < 3)
       usage(module);

    /* parse command line */
    argc--;
    argv++;
    while(argc) {
        if(!strcmp(*argv, "--littleendian")) {
            target_endian = LE;
        } else
        if(!strcmp(*argv, "--bigendian")) {
            target_endian = BE;
        } else
        if(!script) {
            script = *argv;
        } else
        if(!image) {
            image = *argv;
        } else
            usage(module);

        argc--;
        argv++;
    }

    if(!loadscript(script))
        error("cannot load script \"%s\"!", script);

    mkbootfs(image);

    printf("All done.\n");

    return 0;
}

void usage(char *module_name)
{
    fprintf(stderr, "usage: %s [--littleendian (default)] [--bigendian] <scriptfile> <outfile>\n", module_name);
    exit(1);
}

void error(char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "error: ");
    va_start(args, fmt);
      vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    free_sections();
    free_pages();
    exit(1);
}

void *loadfile(char *file, int *size, int *npages)
{
    FILE *fh;
    char *data;
    int nread;
    int newsize;

    /* open file and get file size */
    *size = 0;
    fh = fopen(file, "rb");
    if(!fh)
      return NULL;
    fseek(fh, 0, SEEK_END);
    *size = ftell(fh);

    /* allocate memory */
    if(npages) data = (char *) alloc_buf(*size, &newsize);
          else data = (char *) malloc(*size);
    if(!data) {
      *size = 0;
      fclose(fh);
      return NULL;
    }
    if(npages) *npages = newsize / PAGE_SIZE;

    /* read file */
    fseek(fh, 0, SEEK_SET);
    nread = fread(data, 1, *size, fh);
    if(nread != *size) {
      free(data);
      *size = 0;
      fclose(fh);
      return NULL;
    }
    fclose(fh);

    return data;
}

scr_section *loadscript(char *file)
{
#define stNEWLINE 0
#define stSKIPLINE 1
#define stHEADER 2
#define stLHS 3
#define stRHS 4
    char *data, *end;
    int size;
    int state = stNEWLINE;
    scr_section *cur;
    char *lhs, *rhs;

    if(!(data = loadfile(file,&size,NULL))) {
        return NULL;
    }
    end = data+size;
    scr_data = data;

    /* parse script */
    while(data < end) {
        switch(state) {
        case stSKIPLINE:
            if(*data == '\n' || *data == '\r') {
                state = stNEWLINE;
            }
            data++;
            break;

        case stNEWLINE:
            if(*data == '\n' || *data == '\r') {
                data++;
                break;
            }
            if(*data == '[') {
                lhs = data+1;
                state = stHEADER;
                data++;
                break;
            }
            if(*data == '#' || *data <= ' ') {
                state = stSKIPLINE;
                data++;
                break;
            }
            lhs = data;
            data++;
            state = stLHS;
            break;
        case stHEADER:
            if(*data == ']') {
                cur = (scr_section *) malloc(sizeof(scr_section));
                cur->name = lhs;
                cur->plist = NULL;
                cur->next = NULL;
                if(scr_last) {
                    scr_last->next = cur;
                    scr_last = cur;
                } else {
                    scr_last = scr_first = cur;
                }
                *data = 0;
                state = stSKIPLINE;
            }
            data++;
            break;
        case stLHS:
            if(*data == '\n' || *data == '\r') {
                state = stNEWLINE;
            }
            if(*data == '=') {
                *data = 0;
                rhs = data+1;
                state = stRHS;
            }
            data++;
            continue;
        case stRHS:
            if(*data == '\n' || *data == '\r') {
                scr_param *p = (scr_param *) malloc(sizeof(scr_param));
                p->name = lhs;
                p->value = rhs;
                *data = 0;
                p->next = cur->plist;
                cur->plist = p;
                state = stNEWLINE;
            }
            data++;
            break;
        }
    }
    return scr_first;
}

char *scr_getval(scr_section *s, char *name)
{
    scr_param *p;
    for(p = s->plist; p; p = p->next) {
        if(!strcmp(p->name,name)) return p->value;
    }
    return NULL;
}

char *scr_getvaldef(scr_section *s, char *name, char *def)
{
    scr_param *p;
    for(p = s->plist; p; p = p->next) {
        if(!strcmp(p->name,name)) return p->value;
    }
    return def;
}

void extract_params(scr_section *section, char **name, unsigned int *reserv,
                    unsigned int *type, char **file, unsigned int *vaddr,
                    unsigned int *ventry)
{
    char *res, *typ, *fil;

    *name = section->name;
    *file = (char *)NULL;

    /* extract params */
    res = scr_getval(section, "reserve");
    typ = scr_getval(section, "type");
    fil = scr_getval(section, "file");
    *vaddr  = atoi( scr_getvaldef(section, "vaddr", "0") );
    *ventry = atoi( scr_getvaldef(section, "ventry", "0") );

    /* check for error params */
    if(res && fil)
        error("section [%s]: reserve and file params cannot be set together!", *name);
    else if(!res && !fil)
        error("section [%s]: reserve or file param is not set!", *name);
    else if(res) {
        *reserv = atoi(res);
        if(!(*reserv))
          error("section [%s]: invalid reserv param set!", *name);
    } else
       *file = (char *)fil;
    if(!typ)
       error("section [%s]: type param is not set!", *name);

    /* detect type */
    if(!strcmp(typ, "bootstrap"))  *type = OBJ_TYPE_BOOTSTRAP;
    else if(!strcmp(typ, "code"))  *type = OBJ_TYPE_CODE;
    else if(!strcmp(typ, "data"))  *type = OBJ_TYPE_DATA;
    else if(!strcmp(typ, "elf32")) *type = OBJ_TYPE_ELF32;
    else if(!strcmp(typ, "elf64")) *type = OBJ_TYPE_ELF64;
    else error("section [%s]: object type is unknown!", *name);
}

void free_sections()
{
    scr_section *p, *pb;
    scr_param   *p2, *pb2;
    
    for(p = scr_first; p; p = pb) {
       for(p2 = p->plist; p2; p2 = pb2) {
           pb2 = p2->next;
           free(p2);
       }
       pb = p->next;
       free(p);
    }
    scr_first = scr_last = NULL;

    if(scr_data) {
       free(scr_data);
       scr_data = NULL;
    }
}

dir_page *alloc_dir_page(unsigned int num, unsigned int parent)
{
    dir_page *page;
    btfs_dir_entry *en;

    page = (dir_page *)malloc(sizeof(dir_page));
    memset(page->data, 0, PAGE_SIZE);
    page->num = num;
    page->next = NULL;

    /* set special data for page 0 */
    if(!num) {
       btfs = (btfs_id *)page->data;
       memcpy(&btfs->magic, BOOTFS_MAGIC, sizeof(BOOTFS_MAGIC));
       btfs->type  = fix(BOOTFS_TYPE);
       btfs->psize = fix(PAGE_SIZE);
       btfs->rsize = fix(1);
    } else { /* for other pages */
       en = (btfs_dir_entry *)page->data;
       strcpy(en->name, "..");
       en->offset = fix(parent);
       en->size   = fix(1);
       en->vsize  = fix(PAGE_SIZE);
       en->type   = fix(OBJ_TYPE_DIRECTORY);
    }

    /* add to list */
    if(dir_last) {
        dir_last->next = page;
        dir_last = page;
    } else {
        dir_last = dir_first = page;
    }

    return page;
}

dir_page *dir_page_by_num(unsigned int num)
{
    dir_page *p;

    for(p = dir_first; p; p = p->next)
      if(p->num == num) return p;

    return NULL;
}

void free_pages()
{
    dir_page *p, *pb;

    for(p = dir_first; p; p = pb) {
        pb = p->next;
        free(p);
    }
    dir_first = dir_last = NULL;
}

void *alloc_buf(int size, int *newsize)
{
    char *data;

    /* set newsize multiple by page size */
    *newsize = ( size | (PAGE_SIZE - 1) ) + 1;
    data = (char *)malloc(*newsize);
    if(!data) {
       *newsize = 0;
       return NULL;
    }
    memset(data, 'U', *newsize);

    return data;
}

unsigned int path_itms_count(char *path)
{
    unsigned int cnt = 1;
    unsigned int n;
    int i;

    if(path[0] == '/') path = &path[1];
    n = strlen(path);
    if(path[n-1] == '/') n--;

    for(i=0; i<n; i++)
      if(path[i] == '/') cnt++;

    return cnt;
}

unsigned int path_get_item(char *path, unsigned int idx, char *out, unsigned int size)
{
    unsigned int cnt = 0;
    unsigned int n;
    int i, j;

    if(path[0] == '/') path = &path[1];
    n = strlen(path);
    if(path[n-1] == '/') n--;

    for(i=0; i<n; i++) {
       if(cnt == idx) {
          cnt = 0;
          for(j=i; j<n && path[j] != '/' && cnt < size;  j++)
             out[cnt++] = path[j];
          if(cnt==size) { out[0] = 0; return 0; }
          out[cnt] = 0;
          return cnt;
       }
       if(path[i] == '/') cnt++;
    }

    out[0] = 0;
    return 0;
}

btfs_dir_entry *fs_alloc_file_entry(char *path, unsigned int *page_counter)
{
    btfs_dir_entry *en;
    dir_page *page;
    char name[BOOTFS_NAMELEN];
    unsigned int npi;
    int i;

    if(path[strlen(path)-1] == '/')
        error("\"%s\" must be file!", path);

    if(!dir_first) alloc_dir_page((*page_counter)++, 0);
    page = dir_first;

    npi = path_itms_count(path);

    for(i=0; i<npi; i++) {
        if(!path_get_item(path, i, name, BOOTFS_NAMELEN))
            error("path \"%s\" is invalid!", path);
        if(i == npi-1) {
            en = dir_alloc_entry(name, page);
            return en;
        }
        en = dir_alloc_entry(name, page);
        if(!en->type) {
            en->offset = fix(*page_counter);
            en->size   = fix(1);
            en->vsize  = fix(PAGE_SIZE);
            en->type   = fix(OBJ_TYPE_DIRECTORY);
            page = alloc_dir_page((*page_counter)++, page->num);
        } else {
            page = dir_page_by_num(en->offset);
            if(!page)
               error("fatal error!");
        }
    }
    error("fatal error!");
}

btfs_dir_entry *dir_alloc_entry(char *name, dir_page *page)
{
    btfs_dir_entry *dir;
    int i, n;

    if(strlen(name)>BOOTFS_NAMELEN-1)
        error("name \"%s\" is too long!", name);

    dir = (btfs_dir_entry *)page->data;

    if(!page->num) {
        dir++;
        n = DIR_ITEM_COUNT - 1;
    } else {
        n = DIR_ITEM_COUNT;
    }

    /* try to locate */
    for(i=0; i<n; i++)
       if(!strcmp(dir[i].name,name)) return &dir[i];

    /* try to allocate */
    for(i=0; i<n; i++)
       if(dir[i].type == OBJ_TYPE_NONE) {
           strcpy(dir[i].name, name);
           return &dir[i];
       }
    error("no free entries for \"%s\"!", name);
}

void mkbootfs(char *image)
{
    FILE *fh;
    char *data;
    unsigned int size;
    unsigned int npages;
    unsigned int nwrite;
    unsigned int next_free_page = 0;
    scr_section *p;
    dir_page    *d;
    btfs_dir_entry *entr;
    /* params */
    char *name;
    char *file;
    unsigned int reserv;
    unsigned int type;
    unsigned int vaddr;
    unsigned int ventry;

    fh = fopen(image, "wb");
    if(!fh) error("cannot open file \"%s\"!", image);

    for(p = scr_first; p; p = p->next) {
        printf("Processing section %s...\n", p->name);
        extract_params(p, &name, &reserv, &type, &file, &vaddr, &ventry);
        entr = fs_alloc_file_entry(name, &next_free_page);
        entr->offset = fix(next_free_page);
        entr->type   = fix(type);

        if(!file) {
           nwrite = reserv * PAGE_SIZE;
           npages = reserv;

           entr->size   = fix(reserv);
           entr->vsize  = fix(nwrite);

           data = (char *) alloc_buf(nwrite, &size);
           if(!data)
              error("fatal error. not enought memory!");
        } else {
           data = loadfile(file, &size, &npages);
           if(!data)
              error("cannot load file \"%s\"!", file);
           nwrite = npages * PAGE_SIZE;
           entr->size   = fix(npages);
           entr->vsize  = fix(size);

           switch(type) {
             case OBJ_TYPE_BOOTSTRAP:
             case OBJ_TYPE_CODE:
                 entr->code_vaddr = fix(vaddr);
                 entr->code_ventr = fix(ventry);
               break;
             case OBJ_TYPE_ELF32:
                entr->code_vaddr = 0;
                entr->code_ventr = fix( elf32_find_entry(data, size) );
               break;
             case OBJ_TYPE_ELF64:
                entr->code_vaddr = 0;
                entr->code_ventr = fix( elf64_find_entry(data, size) );
               break;
             case OBJ_TYPE_DATA:
               break;
             default:
                error("unrecognized section type \"%s\"!", scr_getval(p, "type"));
           }

        }

        fseek(fh, PAGE_SIZE * next_free_page, SEEK_SET);
        if(fwrite(data, 1, nwrite, fh) != nwrite) {
            free(data);
            error("cannot write to file \"%s\"!", image);
        }
        next_free_page += npages;
        free(data);
    }

    /* store directory pages */
    btfs->fsize = fix( next_free_page ); /* FS size */
    nwrite = PAGE_SIZE;
    for(d = dir_first; d; d = d->next) {
       fseek(fh, PAGE_SIZE * d->num, SEEK_SET);
       if(fwrite(d->data, 1, nwrite, fh) != nwrite)
            error("cannot write to file \"%s\"!", image);
    }
    fclose(fh);

    printf("Page size = %d, Pages written = %d, Bytes Written = %d\n", PAGE_SIZE,
                       next_free_page, PAGE_SIZE*next_free_page);
}

Elf32_Off elf32_find_entry(void *buf, int size)
{
    struct Elf32_Ehdr *header;
    struct Elf32_Phdr *pheader;
    char *cbuf = buf;
    int byte_swap;
    int index;

#define SWAPIT(x) ((byte_swap) ? SWAP32(x) : (x))

    if(memcmp(cbuf, ELF_MAGIC, sizeof(ELF_MAGIC)-1) != 0) {
        fprintf(stderr, "file does not have proper magic value\n");
        return 0;
    }

    if(cbuf[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "wrong elf class (%d)\n", cbuf[EI_CLASS]);
        return 0;
    }

    if(cbuf[EI_VERSION] != 1) {
        fprintf(stderr, "elf file has unsupported version (%d)\n", cbuf[EI_VERSION]);
        return 0;
    }

    byte_swap = 0;
#if xBIG_ENDIAN
    if(cbuf[EI_DATA] == ELFDATA2LSB) {
        byte_swap = 1;
    }
#else
    if(cbuf[EI_DATA] == ELFDATA2MSB) {
        byte_swap = 1;
    }
#endif

    header = (struct Elf32_Ehdr *)cbuf;
    pheader = (struct Elf32_Phdr *)&cbuf[SWAPIT(header->e_phoff)];

    /* XXX only looking at the first program header. Should be ok */
    return SWAPIT(pheader->p_offset);
#undef SWAPIT
}

Elf64_Off elf64_find_entry(void *buf, int size)
{
    struct Elf64_Ehdr *header;
    struct Elf64_Phdr *pheader;
    char *cbuf = buf;
    int byte_swap;
    int index;

#define SWAPIT(x) ((byte_swap) ? SWAP64(x) : (x))

    if(memcmp(cbuf, ELF_MAGIC, sizeof(ELF_MAGIC)-1) != 0) {
        fprintf(stderr, "file does not have proper magic value\n");
        return 0;
    }

    if(cbuf[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "wrong elf class (%d)\n", cbuf[EI_CLASS]);
        return 0;
    }

    if(cbuf[EI_VERSION] != 1) {
        fprintf(stderr, "elf file has unsupported version (%d)\n", cbuf[EI_VERSION]);
        return 0;
    }

    byte_swap = 0;
#if xBIG_ENDIAN
    if(cbuf[EI_DATA] == ELFDATA2LSB) {
        byte_swap = 1;
    }
#else
    if(cbuf[EI_DATA] == ELFDATA2MSB) {
        byte_swap = 1;
    }
#endif

    header = (struct Elf64_Ehdr *)cbuf;
    pheader = (struct Elf64_Phdr *)&cbuf[SWAPIT(header->e_phoff)];

    /* XXX only looking at the first program header. Should be ok */
    return SWAPIT(pheader->p_offset);
#undef SWAPIT
}
