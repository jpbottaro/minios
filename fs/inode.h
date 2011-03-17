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

block_t read_map(struct inode_s *ino, unsigned int pos, int flag);
struct dir_entry_s *empty_entry(struct inode_s *dir);
struct dir_entry_s *next_entry(struct inode_s *dir, unsigned int *p);
struct dir_entry_s *search_inode(struct inode_s *dir, const char *path);

#endif /* __INODE_H__ */
