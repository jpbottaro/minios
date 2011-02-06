#include "fs.h"
#include <minikernel/sched.h> /* process table (needed for process fd's) */

void set_file_inode(unsigned int fd, ino_t ino)
{
    ps[current_process].files[fd].ino = ino;
}

ino_t file_inode(unsigned int fd)
{
    return ps[current_process].files[fd].ino;
}

void set_file_pos(unsigned int fd, unsigned int pos)
{
    ps[current_process].files[fd].pos = pos;
}

unsigned int file_pos(unsigned int fd)
{
    return ps[current_process].files[fd].pos;
}

ino_t current_dir()
{
    return ps[current_process].curr_dir;
}
