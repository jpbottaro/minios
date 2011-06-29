#ifndef _FS_H
#define _FS_H

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/dir.h>

#define MAX_FILES 10
#define NR_ZONES  10

#define BLOCK_SIZE 1024

struct inode_s {
    u32_t i_free;
    u32_t i_num;
    u32_t i_dirty;
    u32_t i_refcount;
    u32_t i_pos;

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
    int f_fd;
    u32_t f_pos;
    int f_pipenr;
    struct inode_s *f_ino;

    struct file_operations_s *f_op;

    LIST_ENTRY(file_s) unused;
};

struct file_operations_s {
    size_t  (*read)  (struct file_s *, char *, size_t);
    ssize_t (*write) (struct file_s *, char *, size_t);
    size_t  (*lseek) (struct file_s *, off_t, int);
    int     (*flush) (struct file_s *);
    int     (*close) (struct file_s *);
};

extern struct inode_s *root;

LIST_HEAD(unused_fd_t, file_s);

int fs_init(dev_t dev);
int fs_open(const char *filename, int flags, int mode);
int sys_close(int fd);
int fs_closeall(struct file_s files[]);
size_t sys_lseek(int fd, off_t offset, int whence);
size_t fs_lseek(struct file_s *flip, off_t offset, int whence);
size_t sys_read(int fd, char *buf, size_t n);
size_t fs_read(struct file_s *flip, char *buf, size_t n);
ssize_t sys_write(int fd, char *buf, size_t n);
ssize_t fs_write(struct file_s *flip, char *buf, size_t n);
int fs_unlink(const char *pathname);
int fs_rename(const char *oldpath, const char *newpath);
int fs_chdir(const char *path);
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
int fs_getdents(int fd, char *buf, size_t n);
int sys_flush(int fd);
int fs_end();

void init_fds(unsigned int id);
int get_fd_pipe(struct file_operations_s *ops, int nr);
int release_fd_pipe(int fd);
struct inode_s *current_dir();
void set_current_dir(struct inode_s *ino);

/* flags for find_inode */
#define FS_SEARCH_GET     0x0001
#define FS_SEARCH_ADD     0x0002
#define FS_SEARCH_CREAT   0x0003
#define FS_SEARCH_REMOVE  0x0004
#define FS_SEARCH_LASTDIR 0x0005

struct inode_s *find_inode(struct inode_s *dir, const char *user_path, int flag);

/* flags for copy_file */
#define FS_READ  0
#define FS_WRITE 1

int copy_file(char *buf, unsigned int n, unsigned int pos, struct inode_s *ino,
                                                                         int flag);

#define NO_BLOCK ((block_t) 0)
#define NO_INODE ((ino_t)   0)

struct inode_s *get_inode(ino_t num);
struct buf_s *get_block(zone_t num);

void release_inode(struct inode_s *ino);
void release_block(struct buf_s *buf);

int imayor(struct inode_s *ino);
int iminor(struct inode_s *ino);

/* constants for attribute i_mode in inode struct */
#define I_TYPE           0170000
#define I_FILE           0100000
#define I_DIRECTORY      0040000
/* XXX FIXXX, put real constants */
#define I_SPECIAL        0020000
#define I_CHAR           0010000
#define I_BLOCK          0000000
#define IS_FILE(mode)    (((mode) & I_TYPE) == I_FILE)
#define IS_DIR(mode)     (((mode) & I_TYPE) == I_DIRECTORY)
#define IS_CHAR(mode)    (((mode) & I_TYPE) == I_CHAR)
#define IS_BLOCK(mode)   (((mode) & I_TYPE) == I_BLOCK)
#define IS_DEV(mode)     (IS_CHAR(mode) || IS_BLOCK(mode))

int fs_make_dev(const char *name, int type, dev_t major, dev_t minor);

#endif /* _FS_H */
