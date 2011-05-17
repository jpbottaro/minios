#ifndef __FILE_H__
#define __FILE_H__

/* Setters and getters for acessing fs data in the process table and other
 * external sources
 */

#include <minios/fs.h> /* file_s struct */

int get_fd(ino_t ino_num, unsigned int pos);
int release_fd(int fd);

ino_t current_dir();
void set_current_dir(ino_t ino);

struct file_s *get_file(int fd);

#endif /* __FILE_H__ */
