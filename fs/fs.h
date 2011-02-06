#ifndef __FS_H__
#define __FS_H__

/* Global fs header. Every file contains it. This implementations assumes that
 * ALL the fs is loaded into memory from 'fs_start'. We support only a very
 * limited subset of the standards, things like permissions/ownership are left
 * out, along with obscure flags and the like */

#include <minikernel/fs.h>
#include "super.h"
#include "inode.h"
#include "file.h"

#define ERROR -1

#endif /* __FS_H__ */
