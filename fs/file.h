#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

ino_t     file_inode(unsigned int fd);
void  set_file_inode(unsigned int fd, ino_t ino);
    
unsigned int     file_pos(unsigned int fd);
void         set_file_pos(unsigned int fd, unsigned int pos);

ino_t current_dir();

#endif /* __FILE_H__ */
