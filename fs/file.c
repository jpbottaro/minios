#include <minios/sched.h> /* process table (needed for process fd's) */
#include <minios/debug.h>
#include <minios/dev.h>
#include <sys/queue.h>
#include "fs.h"

struct file_s tmpfile;

/* init fds for a process */
void init_fds(unsigned int id)
{
    int i;
    struct inode_s *ino;
    struct file_s *file, *parent_file;
    struct unused_fd_s *unused_fd;

    unused_fd = &ps[id].unused_fd;
    LIST_INIT(unused_fd);

    /* separate instance in where there is a parent to copy fds from, and where
     * there isn't */
    if (current_process == NULL) {
        if ( (ino = find_inode(NULL, "/dev/tty", FS_SEARCH_GET)) == NULL)
            debug_panic("init_fds: no /dev/tty");

        /* open stdin/out/err */
        file = ps[id].files;
        for (i = 0; i < 3; ++i) {
            file->f_ino = ino;
            file->f_used = 1;
            file->f_pos = 0;
            file->f_op = dev_operations(imayor(file->f_ino));
            ino->i_refcount++;
            file++;
        }
        release_inode(ino);

        for (; i < MAX_FILES; ++i) {
            file->f_ino = NULL;
            file->f_used = 0;
            file->f_pos = 0;
            LIST_INSERT_HEAD(unused_fd, file, unused);
            file++;
        }
    } else {
        for (i = 0; i < MAX_FILES; ++i) {
            file = &ps[id].files[i];
            parent_file = &current_process->files[i];

            file->f_ino = parent_file->f_ino;
            file->f_pos = parent_file->f_pos;
            file->f_op = parent_file->f_op;
            file->f_used = parent_file->f_used;
            if (!parent_file->f_used) {
                LIST_INSERT_HEAD(unused_fd, file, unused);
            } else if (file->f_ino != NULL) {
                file->f_ino->i_refcount++;
            }
        }
    }
}

/* get a new fd */
int get_fd(struct inode_s *ino, unsigned int pos)
{
    int fd;
    struct file_s *file;

    file = NULL;
    if (current_process != NULL) {
        file = LIST_FIRST(&current_process->unused_fd);
        if (file != NULL) {
            fd = file - current_process->files;
            LIST_REMOVE(file, unused);
        }
    } else {
        fd = 0;
        file = &tmpfile;
    }

    if (file != NULL) {
        file->f_used  = 1;
        file->f_ino = ino;
        file->f_pos = pos;
        file->f_op = dev_operations(IS_DEV(ino->i_mode) ? ino->i_zone[0] : DEV_FS);
        return fd;
    }

    return ERROR;
}

/* release a fd */
int release_fd(int fd)
{
    if (current_process == NULL)
        return OK;

    struct file_s *file = &current_process->files[fd];

    if (file->f_ino != NULL) {
        release_inode(file->f_ino);
        file->f_ino = NULL;
        file->f_pos = 0;
        file->f_used = 0;
        LIST_INSERT_HEAD(&current_process->unused_fd, file, unused);
        return OK;
    } else {
        return ERROR;
    }
}

struct inode_s *current_dir()
{
    struct inode_s *dir;

    if (current_process == NULL)
        dir = get_root();
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
    if (current_process != NULL) {
        struct file_s *file = &current_process->files[fd];
        if (file->f_used)
            return file;
        else
            return NULL;
    } else {
        return &tmpfile;
    }
}
