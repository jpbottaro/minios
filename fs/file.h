#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

#include <minios/fs.h> /* file_s struct */

int get_fd(ino_t ino_num, unsigned int pos);
int release_fd(int fd);

struct file_operations_s *fd_op(unsigned int fd);

ino_t current_dir();
void set_current_dir(ino_t ino);

ino_t     file_inode(unsigned int fd);
void  set_file_inode(unsigned int fd, ino_t ino);
    
unsigned int     file_pos(unsigned int fd);
void         set_file_pos(unsigned int fd, unsigned int pos);

#endif /* __FILE_H__ */
