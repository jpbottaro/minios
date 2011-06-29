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
    struct inode_s *ino;
    struct file_s *file, *parent_file;
    struct unused_fd_t *unused_fd;

    unused_fd = &ps[id].unused_fd;
    LIST_INIT(unused_fd);

    /* separate instance in where there is a parent to copy fds from, and where
     * there is not one */
    if (current_process == NULL) {
        if ( (ino = find_inode(NULL, "/dev/tty", FS_SEARCH_GET)) == NULL)
            debug_panic("init_fds: no /dev/tty");

        /* open stdin/out/err */
        file = ps[id].files;
        for (i = 0; i < 3; ++i) {
            file->f_ino = ino;
            file->f_pos = 0; file->f_fd = i;
            dev_file_calls(file, imayor(file->f_ino));
            ino->i_refcount++;
            file++;
        }
        release_inode(ino);

        for (; i < MAX_FILES; ++i) {
            file->f_fd = i;
            file->f_ino = NULL;
            file->f_pos = 0;
            LIST_INSERT_HEAD(unused_fd, file, unused);
            file++;
        }
    } else {
        for (i = 0; i < MAX_FILES; ++i) {
            file = &ps[id].files[i];
            parent_file = &current_process->files[i];

            file->f_fd = i;
            file->f_ino = parent_file->f_ino;
            file->f_pos = parent_file->f_pos;
            if (file->f_ino == NULL) {
                LIST_INSERT_HEAD(unused_fd, file, unused);
            } else {
                file->f_ino->i_refcount++;
                dev_file_calls(file, imayor(file->f_ino));
            }
        }
    }
}

void fs_fd_cpy(unsigned int pid, unsigned int cid)
{
    struct file_s *pfile, *cfile, *pend;

    pfile = ps[pid].files;
    cfile = ps[cid].files;
    pend = ps[pid].files + MAX_FILES;

    while (pfile < pend) {
        *pfile = *cfile;
        if (cfile->f_ino != NULL)
            cfile->f_ino->i_refcount++;
        pfile++;
        cfile++;
    }
}

/* get a new fd */
int get_fd(struct inode_s *ino, unsigned int pos)
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
        file->f_ino = ino;
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

    if (file->f_ino != NULL) {
        release_inode(file->f_ino);
        file->f_ino = NULL;
        file->f_pos = 0;
        LIST_INSERT_HEAD(unused_fd, file, unused);
        return OK;
    } else {
        return ERROR;
    }
}

int get_fd_pipe(struct file_operations_s *ops, int nr)
{
    struct unused_fd_t *unused_fd;
    struct file_s *file;

    unused_fd = &current_process->unused_fd;
    file = LIST_FIRST(unused_fd);
    if (file != NULL)
        LIST_REMOVE(file, unused);

    if (file != NULL) {
        file->f_op = ops;
        file->pipe_nr = nr;
        return file->f_fd;
    }

    return ERROR;
}

int release_fd_pipe(int fd)
{
    struct unused_fd_t *unused_fd = &current_process->unused_fd;
    struct file_s *file = &current_process->files[fd];

    file->f_ino = NULL;
    file->f_pos = 0;
    LIST_INSERT_HEAD(unused_fd, file, unused);

    return OK;
}

struct inode_s *current_dir()
{
    struct inode_s *dir;

    if (current_process == NULL)
        dir = root;
    else
        dir = current_process->curr_dir;
    dir->i_refcount++;

    return dir;
}

void set_current_dir(struct inode_s *ino)
{
    if (current_process != NULL) {
        release_inode(current_process->curr_dir);
        current_process->curr_dir = ino;
        ino->i_refcount++;
    }
}

struct file_s *get_file(int fd)
{
    if (current_process != NULL)
        return &current_process->files[fd];
    else
        return &tmpfile;
}
