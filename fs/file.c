#include <minios/sched.h> /* process table (needed for process fd's) */
#include <minios/debug.h>
#include <minios/dev.h>
#include <sys/queue.h>
#include "fs.h"

struct file_s tmpfile = {.f_fd = 0};

/* init fds for a process */
void init_fds(unsigned int id)
{
    int i;
    ino_t ino_num;
    struct inode_s *ino;
    struct file_s *file = ps[id].files;
    struct unused_fd_t *unused_fd;

    if ( (ino_num = find_inode(NULL, "/dev/tty", FS_SEARCH_GET)) == NO_INODE)
        debug_panic("init_fds: no /dev/tty");
    ino = get_inode(ino_num);

    file->f_ino = ino;
    file->f_pos = 0; file->f_fd = 0;
    dev_file_calls(file, imayor(file->f_ino));
    file++;
    file->f_ino = ino;
    file->f_pos = 0; file->f_fd = 1;
    dev_file_calls(file, imayor(file->f_ino));
    file++;
    file->f_ino = ino;
    file->f_pos = 0; file->f_fd = 2;
    dev_file_calls(file, imayor(file->f_ino));
    file++;

    unused_fd = &ps[id].unused_fd;
    LIST_INIT(unused_fd);
    i = 3;
    for (; file < &ps[id].files[MAX_FILES]; ++file) {
        file->f_fd = i++;
        file->f_ino = NULL;
        file->f_pos = 0;
        LIST_INSERT_HEAD(unused_fd, file, unused);
    }
}

/* get a new fd */
int get_fd(ino_t ino_num, unsigned int pos)
{
    struct unused_fd_t *unused_fd;
    struct file_s *file;

    if (current_process != NULL) {
        unused_fd = &current_process->unused_fd;
        file = LIST_FIRST(unused_fd);
        if (file != NULL)
            LIST_REMOVE(file, unused);
    } else {
        file = &tmpfile;
    }

    if (file != NULL) {
        file->f_ino = get_inode(ino_num);
        file->f_pos = pos;
        return file->f_fd;
    }

    return ERROR;
}

/* release a fd */
int release_fd(int fd)
{
    if (current_process == NULL)
        return OK;

    struct unused_fd_t *unused_fd = &current_process->unused_fd;
    struct file_s *file = &current_process->files[fd];

    if (file->f_ino != NO_INODE) {
        file->f_ino = NULL;
        file->f_pos = 0;
        LIST_INSERT_HEAD(unused_fd, file, unused);
        return OK;
    } else {
        return ERROR;
    }
}

ino_t current_dir()
{
    if (current_process == NULL) return 1;
    else                         return current_process->curr_dir;
}

void set_current_dir(ino_t ino)
{
    if (current_process != NULL)
        current_process->curr_dir = ino;
}

struct file_s *get_file(int fd)
{
    if (current_process != NULL)
        return &current_process->files[fd];
    else
        return &tmpfile;
}
