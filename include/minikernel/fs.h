#ifndef _FS_H
#define _FS_H

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/dir.h>

#define MAX_FILES   10

struct file_s {
    unsigned int fd;
    unsigned int ino;
    unsigned int pos;

    LIST_ENTRY(file_s) unused;
};

int fs_init(char *fs_start);
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
int fs_end();

void init_fds(unsigned int id);

#endif /* _FS_H */
