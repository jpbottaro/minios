#ifndef _FS_H
#define _FS_H

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/dir.h>

#define MAX_FILES 10
#define NR_ZONES  10

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

struct file_s {
    unsigned int fd;
    unsigned int ino;
    unsigned int pos;

    LIST_ENTRY(file_s) unused;
};

LIST_HEAD(unused_fd_t, file_s);

int init_fs(char *fs_start);
int sys_open(const char *filename, int flags, int mode);
int sys_close(unsigned int fd);
int sys_lseek(unsigned int fd, off_t offset, int whence);
int sys_read(unsigned int fd, char *buf, unsigned int n);
int sys_write(unsigned int fd, char *buf, unsigned int n);
int sys_unlink(const char *pathname);
int sys_rename(const char *oldpath, const char *newpath);
int sys_chdir(const char *path);
int sys_mkdir(const char *pathname, mode_t mode);
int sys_rmdir(const char *pathname);
int sys_getdents(unsigned int fd, struct dirent *dirp, unsigned int n);
int end_fs();

void init_fds(unsigned int id);

/* flags for find_inode */
#define FS_SEARCH_GET     0x0001
#define FS_SEARCH_ADD     0x0002
#define FS_SEARCH_CREAT   0x0003
#define FS_SEARCH_REMOVE  0x0004
#define FS_SEARCH_LASTDIR 0x0005

ino_t find_inode(struct inode_s *dir, const char *user_path, int flag);
unsigned int copy_file(char *buf, unsigned int n, unsigned int pos,
                       struct inode_s *ino, int flag);

struct inode_s *get_inode(ino_t num);
void           *get_block(zone_t num);

#endif /* _FS_H */
