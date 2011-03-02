#ifndef __INODE_H__
#define __INODE_H__

/* Inodes and co. Basic struct taken from minix v3. Of course since our file
 * system resides in memory, we dont use any caching, we can just point to
 * the beginning of the mapped fs in memory and calculate pointers from there
 */

#include <sys/types.h>

#define NR_ZONES 10
#define MAX_PATH 120
#define MAX_NAME 30
#define NO_LINK  ((nlink_t) 0)
#define NO_BLOCK ((block_t) 0)
#define NO_INODE ((ino_t)   0)

struct inode_s {
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

struct dir_entry_s {
    u16_t num;
    char name[MAX_NAME];
};

struct inode_s *get_inode(ino_t num);
void           *get_block(zone_t num);

block_t read_map(struct inode_s *ino, unsigned int pos);
ino_t find_inode(struct inode_s *dir, const char *user_path, int flag);
struct dir_entry_s *search_inode(struct inode_s *dir, const char *path);

/* flags for find_inode */
#define FS_SEARCH_GET     0x0001
#define FS_SEARCH_ADD     0x0002
#define FS_SEARCH_CREAT   0x0003
#define FS_SEARCH_REMOVE  0x0004
#define FS_SEARCH_LASTDIR 0x0005

/* constants for attribute i_mode in inode struct */
#define I_TYPE           0170000
#define I_FILE           0100000
#define I_DIRECTORY      0040000
#define I_SPECIAL        0020000
#define IS_FILE(mode)    (((mode) & I_TYPE) == I_FILE)
#define IS_DIR(mode)     (((mode) & I_TYPE) == I_DIRECTORY)
#define IS_CHAR(mode)    (((mode) & I_TYPE) == I_SPECIAL)

#endif /* __INODE_H__ */
