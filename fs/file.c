#include "fs.h"
#include <minikernel/sched.h> /* process table (needed for process fd's) */
#include <sys/queue.h>

struct unused_fd_t unused_fd;

void init_fds(unsigned int id)
{
    int i;
    struct file_s *file = ps[id].files;

    file->ino = find_inode(root, "/dev/stdin", FS_SEARCH_GET);
    file->pos = 0; file->fd = 0; file++;
    file->ino = find_inode(root, "/dev/stdout", FS_SEARCH_GET);
    file->pos = 0; file->fd = 1; file++;
    file->ino = find_inode(root, "/dev/stderr", FS_SEARCH_GET);
    file->pos = 0; file->fd = 2; file++;

    LIST_INIT(&unused_fd);
    i = 3;
    for (; file < &ps[id].files[MAX_FILES]; ++file) {
        file->fd = i++;
        file->ino = NO_INODE;
        file->pos = 0;
        LIST_INSERT_HEAD(&unused_fd, file, unused);
    }
}

int get_fd(ino_t ino_num, unsigned int pos)
{
    struct file_s *file = LIST_FIRST(&unused_fd);

    if (file != NULL) {
        LIST_REMOVE(file, unused);
        file->ino = ino_num;
        file->pos = pos;
        return file->fd;
    }

    return ERROR;
}

int release_fd(int fd)
{
    struct file_s *file = &ps[current_process].files[fd];

    if (file->ino != NO_INODE) {
        file->ino = NO_INODE;
        file->pos = 0;
        LIST_INSERT_HEAD(&unused_fd, file, unused);
        return OK;
    } else {
        return ERROR;
    }
}

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
