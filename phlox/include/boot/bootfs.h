/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _BOOTFS_H_
#define _BOOTFS_H_

#include <boot/bootfs_struct.h>

#ifndef NULL
# ifdef __cplusplus
#    define NULL 0
# else
#    define NULL ((void *)0)
# endif
#endif

/* The possibilities for the third argument to `seek'.
   These values should not be changed.  */
#define SEEK_SET    0   /* Seek from beginning of file.  */
#define SEEK_CUR    1   /* Seek from current position.   */
#define SEEK_END    2   /* Seek from end of file.        */

typedef struct {
    void         *addr;  /* address of loaded bootfs image */
    btfs_id      *hdr;   /* bootfs header                  */
    unsigned int cwd;    /* page number of current dir     */
    unsigned int scwd;   /* size in pages of current dir   */
} bootfs_t;

typedef struct {
    bootfs_t       *fs;  /* parent FS                       */
    btfs_dir_entry *ent; /* directory entry                 */
    int            pos;  /* current position for read/write */
} bootfs_fh_t;

#ifdef __cplusplus
extern "C" {
#endif

/* mount/umount bootfs.
 * addr   - address of loaded bootfs image;
 * fsinfo - handle of mounted FS;
 * returns 0 on success.
 * NOTE: mount sets mounted fs as current on success.
*/
int btfs_mount(void *addr, bootfs_t *fsinfo);
int btfs_umount(bootfs_t *fsinfo);

/* set current fs
 * fs - mounted fs;
 * returns 0 on success.
*/
int btfs_set_cur_fs(bootfs_t *fs);

/* locate directory entry
 * returns NULL on error.
*/
btfs_dir_entry *btfs_locate(const char *path);

/* change current directory. returns 0 on success */
int btfs_chdir(const char *path);

/* open/close file.
 * fh - file handle;
 * e  - directory entry;
 * path - path;
 * returns 0 on success.
*/
int btfs_open_p(bootfs_fh_t *fh, const char *path);
int btfs_open_e(bootfs_fh_t *fh, btfs_dir_entry *e);
int btfs_close(bootfs_fh_t *fh);

/* seek. returns seeked position from begining of file */
int btfs_seek(bootfs_fh_t *fh, int offs, unsigned int whence);

/* read/write file.
 * buf   - buffer;
 * rsize - record size;
 * cnt   - record count;
 * fh    - file handle;
 * returns count of bytes successfully processed.
*/
unsigned int btfs_read(void *buf, unsigned int rsize, unsigned int cnt, bootfs_fh_t *fh);
unsigned int btfs_write(void *buf, unsigned int rsize, unsigned int cnt, bootfs_fh_t *fh);

/* returns address of byte at position
 * NULL on error
*/
void *btfs_posaddr(bootfs_fh_t *fh, unsigned int pos);

/* returns address of object data */
void *btfs_objaddr(btfs_dir_entry *en);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
