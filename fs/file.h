#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

#include <minios/fs.h> /* file_s struct */

int get_fd(ino_t ino_num, unsigned int pos);
int release_fd(int fd);

struct file_operations_s *fd_op(int fd);

ino_t current_dir();
void set_current_dir(ino_t ino);

ino_t     file_inode(int fd);
void  set_file_inode(int fd, ino_t ino);
    
unsigned int     file_pos(int fd);
void         set_file_pos(int fd, unsigned int pos);

struct file_s *get_file(int fd);

#endif /* __FILE_H__ */
