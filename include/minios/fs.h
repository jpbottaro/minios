#ifndef _FS_H
#define _FS_H

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/dir.h>

#define MAX_FILES 15
#define NR_ZONES  10
#define BLOCK_SIZE 1024

/* constants for attribute i_mode in inode struct */
#define I_TYPE           0170000
#define I_FILE           0100000
#define I_DIRECTORY      0040000
#define I_SPECIAL        0020000
#define I_CHAR           0010000
#define I_BLOCK          0000000
#define IS_FILE(mode)    (((mode) & I_TYPE) == I_FILE)
#define IS_DIR(mode)     (((mode) & I_TYPE) == I_DIRECTORY)
#define IS_CHAR(mode)    (((mode) & I_TYPE) == I_CHAR)
#define IS_BLOCK(mode)   (((mode) & I_TYPE) == I_BLOCK)
#define IS_DEV(mode)     (IS_CHAR(mode) || IS_BLOCK(mode))

#define NO_BLOCK ((block_t) 0)
#define NO_INODE ((ino_t)   0)

struct inode_s {
    u32_t i_free;
    u32_t i_num;
    u32_t i_dirty;
    u32_t i_refcount;

    u16_t i_mode;           /* file type, protection, etc. */
    u16_t i_nlinks;         /* how many links to this file */
    u16_t i_uid;            /* user id of the file's owner */
    u16_t i_gid;            /* group number */
    u32_t i_size;           /* current file size in bytes */
    u32_t i_atime;
    u32_t i_mtime;          /* when was file data last changed */
    u32_t i_ctime;
    u32_t i_zone[NR_ZONES]; /* zone numbers for direct, ind, and dbl ind */

    LIST_ENTRY(inode_s) ptr;
};

struct buf_s {
    u32_t b_free;
    u32_t b_num;
    u32_t b_dirty;
    u32_t b_refcount;
    u32_t b_pos;

    char b_buffer[BLOCK_SIZE];

    LIST_ENTRY(buf_s) ptr;
};

struct file_s {
    u32_t f_pos;
    struct inode_s *f_ino;
    struct file_operations_s *f_op;

    int f_used;
    LIST_ENTRY(file_s) unused;
};

struct file_operations_s {
    size_t  (*read)  (struct file_s *, char *, size_t);
    ssize_t (*write) (struct file_s *, char *, size_t);
    size_t  (*lseek) (struct file_s *, off_t, int);
    int     (*flush) (struct file_s *);
    int     (*close) (struct file_s *);
};

LIST_HEAD(unused_fd_s, file_s);

int fs_init(dev_t dev);

int fs_open(const char *filename, int flags, int mode);
int fs_close(int fd);
int fs_closeall();

size_t sys_lseek(int fd, off_t offset, int whence);
size_t sys_read(int fd, char *buf, size_t n);
ssize_t sys_write(int fd, char *buf, size_t n);
int sys_flush(int fd);

void init_fds(unsigned int id);
int get_fd(struct inode_s *ino, unsigned int pos);

struct inode_s *get_free_inode();
struct inode_s *current_dir();
void release_inode(struct inode_s *ino);

int imayor(struct inode_s *ino);
int iminor(struct inode_s *ino);

int fs_make_dev(const char *name, int type, dev_t major, dev_t minor);

#endif /* _FS_H */
