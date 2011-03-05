#ifndef __INODE_H__
#define __INODE_H__

/* Inodes and co. Basic struct taken from minix v3. Of course since our file
 * system resides in memory, we dont use any caching, we can just point to
 * the beginning of the mapped fs in memory and calculate pointers from there
 */

#include <sys/types.h>

#define MAX_PATH 120
#define MAX_NAME 30
#define NO_LINK  ((nlink_t) 0)

struct dir_entry_s {
    u16_t num;
    char name[MAX_NAME];
};

block_t read_map(struct inode_s *ino, unsigned int pos);
struct dir_entry_s *search_inode(struct inode_s *dir, const char *path);

/* constants for attribute i_mode in inode struct */
#define I_TYPE           0170000
#define I_FILE           0100000
#define I_DIRECTORY      0040000
#define I_SPECIAL        0020000
#define IS_FILE(mode)    (((mode) & I_TYPE) == I_FILE)
#define IS_DIR(mode)     (((mode) & I_TYPE) == I_DIRECTORY)
#define IS_CHAR(mode)    (((mode) & I_TYPE) == I_SPECIAL)

#endif /* __INODE_H__ */
