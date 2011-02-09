#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

#include <minikernel/fs.h> /* file_s struct */

int get_fd(ino_t ino_num, unsigned int pos);
int release_fd(int fd);

ino_t current_dir();

ino_t     file_inode(unsigned int fd);
void  set_file_inode(unsigned int fd, ino_t ino);
    
unsigned int     file_pos(unsigned int fd);
void         set_file_pos(unsigned int fd, unsigned int pos);

extern LIST_HEAD(unused_fd_t, file_s) unused_fd;

#endif /* __FILE_H__ */
