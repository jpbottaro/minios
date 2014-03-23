#ifndef __INODE_H__
#define __INODE_H__

/* Inodes and co. Basic struct taken from minix v3. Of course since our file
 * system resides in memory, we dont use any caching, we can just point to
 * the beginning of the mapped fs in memory and calculate pointers from there
 */

#include <sys/types.h>

#define MAX_PATH 120
#define MAX_NAME 30
#define MAX_INODES_CACHE 50
#define MAX_BUFFERS_CACHE 10
#define NO_LINK  ((nlink_t) 0)

struct dir_entry_s {
    u16_t num;
    char name[MAX_NAME];
};

void cache_init();
block_t read_map(struct inode_s *ino, unsigned int pos, int flag);
int empty_entry(struct inode_s *dir, ino_t ino_num, char *name);
int add_entry(struct inode_s *dir, ino_t ino_num, char *name);
int next_entry(struct inode_s *dir, unsigned int *p, struct dir_entry_s *dent);
int search_inode(struct inode_s *dir, const char *name, const ino_t ino_num,
                 struct dir_entry_s *dentry, int flag);

/* flags for find_inode */
#define FS_SEARCH_GET     0x0001
#define FS_SEARCH_ADD     0x0002
#define FS_SEARCH_CREAT   0x0003
#define FS_SEARCH_REMOVE  0x0004

struct inode_s *find_inode(struct inode_s *dir, const char *user_path, int flag);

struct inode_s *get_inode(ino_t num);
struct buf_s *get_block(zone_t num);
void release_block(struct buf_s *buf);
void release_inode(struct inode_s *ino);

#endif /* __INODE_H__ */
