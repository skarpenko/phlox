/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

/* define to use libstring */
#define USE_STRING
#include <boot/bootfs.h>

#ifdef USE_STRING
#include <string.h>
#endif

static bootfs_t *_cur_fs = NULL;   /* current FS */

#ifndef USE_STRING
/* returns string length */
static unsigned int _str_len(const char *str)
{
    unsigned int i = 0;

    while(str[i]) i++;

    return i;
}

/* string compare. returns 0 if equal */
static unsigned int _str_cmp(const char *s1, const char *s2)
{
    unsigned int i = 0;

    while(s1[i] || s2[i]) {
      if(s1[i] != s2[i])
           return 1;
      i++;
    }

    return 0;
}

/* memory copy */
static void *_memncpy(void *dst, void *src, unsigned int n)
{
    unsigned int i;

    for(i=0; i<n; i++)
      ((char *)dst)[i] = ((char *)src)[i];
   return dst;
}
#else
#define _str_len strlen
#define _str_cmp strcmp
#define _memncpy memcpy
#endif

/* returns items count in path */
static unsigned int _path_itms_count(const char *path)
{
    unsigned int cnt = 1;
    unsigned int n;
    unsigned int i;

    if(path[0] == '/') path = &path[1];
    n = _str_len(path);
    if(path[n-1] == '/') n--;

    for(i=0; i<n; i++)
      if(path[i] == '/') cnt++;

    return cnt;
}

/* returns path item by its index */
static unsigned int _path_get_item(const char *path, unsigned int idx, char *out, unsigned int size)
{
    unsigned int cnt = 0;
    unsigned int n;
    unsigned int i, j;

    if(path[0] == '/') path = &path[1];
    n = _str_len(path);
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

/* locate directory entry by name */
static btfs_dir_entry *_ent_locate(btfs_dir_entry *first, unsigned int count, const char *name)
{
    unsigned int i;

    for(i=0; i<count; i++)
      if(!_str_cmp(first[i].name, name))
          return &first[i];

    return NULL;
}


int btfs_mount(void *addr, bootfs_t *fsinfo)
{
    btfs_id *id = (btfs_id *)addr;
    char *magic = BOOTFS_MAGIC;
    unsigned int i;

    /* check fs magic value */
    for(i=0; i < 4; i++)
      if( ((char *)&id->magic)[i] != magic[i] )
        return 1;

    /* check other fields */
    if( (id->type != BOOTFS_TYPE) || !id->psize || !id->fsize || !id->rsize)
       return 1;

    /* all tests passed store info */
    fsinfo->addr = addr;
    fsinfo->hdr  = id;
    fsinfo->cwd  = 0;
    fsinfo->scwd = id->rsize;

    _cur_fs = fsinfo; /* set as current FS */

    return 0;
}

int btfs_umount(bootfs_t *fsinfo)
{
    fsinfo->addr = NULL;
    fsinfo->hdr  = NULL;
    fsinfo->cwd  = 0;
    fsinfo->scwd = 0;

    _cur_fs = NULL;

    return 0;
}

int btfs_set_cur_fs(bootfs_t *fs)
{
    if(fs->addr) {
       _cur_fs = fs;
       return 0;
    }
    return 1;
}

btfs_dir_entry *btfs_locate(const char *path)
{
    unsigned int page, npages;
    btfs_dir_entry *first = NULL, *en = NULL;
    unsigned int entries;
    char name[BOOTFS_NAMELEN];
    unsigned int npi;
    unsigned int i;

    if(path[0] == '/') { /* if abs path first directory is root dir */
        page   = 0;
        npages = 1;
    } else {             /* if relative path use current directory */
        page   = _cur_fs->cwd;
        npages = _cur_fs->scwd;
    }

    /* walk throught path */
    npi = _path_itms_count(path);
    for(i=0; i<npi; i++) {
        if(!page) {
            first   = _cur_fs->addr; first++;
            entries = (npages * _cur_fs->hdr->psize) / sizeof(btfs_dir_entry) - 1;
        } else {
            first   = _cur_fs->addr + (page * _cur_fs->hdr->psize);
            entries = (npages * _cur_fs->hdr->psize) / sizeof(btfs_dir_entry);
        }

        /* extract item from path */
        if(!_path_get_item(path, i, name, BOOTFS_NAMELEN))
             return NULL;

        /* locate item in current directory */
        if( (en = _ent_locate(first, entries, name)) == NULL )
             return NULL;

        /* all items by exception the last one must be directories */
        if(i<npi-1 && en->type != OBJ_TYPE_DIRECTORY)
             return NULL;

        /* set new directory */
        page   = en->offset;
        npages = en->size;
    }

    return en;
}

int btfs_chdir(const char *path)
{
    btfs_dir_entry *en = NULL;

    /* if root specified */
    if(_str_len(path) == 1 && path[0] =='/') {
        _cur_fs->cwd  = 0;
        _cur_fs->scwd = 1;
        return 0;
    }

    /* locate dir */
    if( (en = btfs_locate(path)) == NULL)
          return 1;

    /* must be directory */
    if(en->type != OBJ_TYPE_DIRECTORY)
          return 1;

    /* set current path */
    _cur_fs->cwd  = en->offset;
    _cur_fs->scwd = en->size;

    return 0;
}

int btfs_open_e(bootfs_fh_t *fh, btfs_dir_entry *e)
{
    fh->fs  = _cur_fs;
    fh->ent = e;
    fh->pos = 0;

    return 0;
}

int btfs_open_p(bootfs_fh_t *fh, const char *path)
{
    btfs_dir_entry *e = NULL;

    if( (e=btfs_locate(path)) == NULL )
        return 1;

    fh->fs  = _cur_fs;
    fh->ent = e;
    fh->pos = 0;

    return 0;
}

int btfs_close(bootfs_fh_t *fh)
{
    fh->fs  = NULL;
    fh->ent = NULL;
    fh->pos = -1;

    return 0;
}

int btfs_seek(bootfs_fh_t *fh, int offs, unsigned int whence)
{
    int base;

    switch(whence) {
       case SEEK_END:
         base = fh->ent->vsize;
           break;
       case SEEK_CUR:
         base = fh->pos;
           break;
       case SEEK_SET:
       default:
         base = 0;
           break;
    }

    base += offs;

    if(base < 0)
      base = 0;
    else if((unsigned)base > fh->ent->vsize)
      base = fh->ent->vsize;

    fh->pos = base;

    return base;
}

unsigned int btfs_read(void *buf, unsigned int rsize, unsigned int cnt, bootfs_fh_t *fh)
{
    unsigned int n = rsize * cnt;
    unsigned int pos = fh->pos;
    unsigned int fsize = fh->ent->vsize;
    void *src = _cur_fs->addr + (fh->ent->offset * _cur_fs->hdr->psize) + pos;

    if( n > fsize - pos) n = fsize-pos;

    /* copy block of memory */
    _memncpy(buf, src, n);

    return n;
}

unsigned int btfs_write(void *buf, unsigned int rsize, unsigned int cnt, bootfs_fh_t *fh)
{
    unsigned int n = rsize * cnt;
    unsigned int pos = fh->pos;
    unsigned int fsize = fh->ent->vsize;
    void *dst = _cur_fs->addr + (fh->ent->offset * _cur_fs->hdr->psize) + pos;

    if( n > fsize - pos) n = fsize-pos;

    /* copy block of memory */
    _memncpy(dst, buf, n);

    return n;
}

void *btfs_posaddr(bootfs_fh_t *fh, unsigned int pos)
{

    /* pos must be inside of the file */
    if(pos > fh->ent->vsize-1) return NULL;

    return (void *)(_cur_fs->addr + (fh->ent->offset * _cur_fs->hdr->psize) + pos);
}

void *btfs_objaddr(btfs_dir_entry *en)
{
    return (void *)(_cur_fs->addr + (en->offset * _cur_fs->hdr->psize));
}
