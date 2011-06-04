#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

#include <minios/fs.h> /* file_s struct */

int get_fd(struct inode_s *ino, unsigned int pos);
int release_fd(int fd);

struct file_s *get_file(int fd);

#endif /* __FILE_H__ */
