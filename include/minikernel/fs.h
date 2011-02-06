#ifndef _FS_H
#define _FS_H

#include <sys/types.h>

#define MAX_FILES   10

struct file_s {
    unsigned int ino;
    unsigned int pos;
};

int fs_init(char *fs_start);
int sys_open(const char *filename, int flags, int mode);
int sys_close(unsigned int fd);
int sys_lseek(unsigned int fd, off_t offset, int whence);
int sys_read(unsigned int fd, char *buf, unsigned int n);
int sys_write(unsigned int fd, char *buf, unsigned int n);
int fs_end();

extern char *fs_offset;
extern struct inode_s *root;

#endif /* _FS_H */
