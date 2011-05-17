#include <minios/sched.h> /* process table (needed for process fd's) */
#include <minios/debug.h>
#include <sys/queue.h>
#include "fs.h"

/* init fds for a process */
void init_fds(unsigned int id)
{
    int i;
    ino_t ino_num;
    struct inode_s *dev;
    struct file_s *file = ps[id].files;
    struct unused_fd_t *unused_fd;

    if ( (ino_num = find_inode(NULL, "/dev", FS_SEARCH_GET)) == NO_INODE)
        debug_panic("init_fds: no /dev folder (init_fds)");
    dev = get_inode(ino_num);

    file->f_ino = find_inode(dev, "stdin", FS_SEARCH_GET);
    file->f_pos = 0; file->f_fd = 0; file++;
    file->f_ino = find_inode(dev, "stdout", FS_SEARCH_GET);
    file->f_pos = 0; file->f_fd = 1; file++;
    file->f_ino = find_inode(dev, "stderr", FS_SEARCH_GET);
    file->f_pos = 0; file->f_fd = 2; file++;

    unused_fd = &ps[id].unused_fd;
    LIST_INIT(unused_fd);
    i = 3;
    for (; file < &ps[id].files[MAX_FILES]; ++file) {
        file->f_fd = i++;
        file->f_ino = NO_INODE;
        file->f_pos = 0;
        LIST_INSERT_HEAD(unused_fd, file, unused);
    }
}

/* get a new fd */
int get_fd(ino_t ino_num, unsigned int pos)
{
    struct unused_fd_t *unused_fd = &current_process->unused_fd;
    struct file_s *file = LIST_FIRST(unused_fd);

    if (file != NULL) {
        LIST_REMOVE(file, unused);
        file->f_ino = ino_num;
        file->f_pos = pos;
        return file->f_fd;
    }

    return ERROR;
}

/* release a fd */
int release_fd(int fd)
{
    struct unused_fd_t *unused_fd = &current_process->unused_fd;
    struct file_s *file = &current_process->files[fd];

    if (file->f_ino != NO_INODE) {
        file->f_ino = NO_INODE;
        file->f_pos = 0;
        LIST_INSERT_HEAD(unused_fd, file, unused);
        return OK;
    } else {
        return ERROR;
    }
}

void set_file_inode(int fd, ino_t ino)
{
    current_process->files[fd].f_ino = ino;
}

ino_t file_inode(int fd)
{
    return current_process->files[fd].f_ino;
}

struct file_operations_s *fd_op(int fd)
{
    return current_process->files[fd].f_op;
}

void set_file_pos(int fd, unsigned int pos)
{
    current_process->files[fd].f_pos = pos;
}

unsigned int file_pos(int fd)
{
    return current_process->files[fd].f_pos;
}

ino_t current_dir()
{
    return current_process->curr_dir;
}

void set_current_dir(ino_t ino)
{
    current_process->curr_dir = ino;
}

struct file_s *get_file(int fd)
{
    return &current_process->files[fd];
}
