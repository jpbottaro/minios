#ifndef __FS_H__
#define __FS_H__

/* Global fs header. Every file contains it. This implementations assumes that
 * ALL the fs is loaded into memory from 'fs_start'. We support only a very
 * limited subset of the standards, things like permissions/ownership are left
 * out, along with obscure flags and the like */

#include <minios/fs.h>
#include "super.h"
#include "inode.h"
#include "file.h"

#define OK 0
#define ERROR -1

struct real_inode_s {
    u16_t i_mode;           /* file type, protection, etc. */
    u16_t i_nlinks;         /* how many links to this file */
    u16_t i_uid;            /* user id of the file's owner */
    u16_t i_gid;            /* group number */
    u32_t i_size;           /* current file size in bytes */
    u32_t i_atime;
    u32_t i_mtime;          /* when was file data last changed */
    u32_t i_ctime;
    u32_t i_zone[NR_ZONES]; /* zone numbers for direct, ind, and dbl ind */
};

extern struct file_s *fs_dev;

struct inode_s *get_root();

#endif /* __FS_H__ */
