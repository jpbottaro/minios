#include <minios/scall.h>
#include <minios/pipe.h>
#include <minios/misc.h>
#include <minios/sem.h>
#include <minios/dev.h>

struct pipe_s {
    char buffer[MAX_SIZE];
    int init;
    int end;
    int done;
    sem_t mutex, empty, full;

    struct inode_s *ino_read;
    struct inode_s *ino_write;

    LIST_ENTRY(pipe_s) unused;
} pipes[MAX_PIPES];

LIST_HEAD(unused_pipe_t, pipe_s) unused_pipes;

size_t pipe_read(struct file_s *flip, char *buf, size_t n)
{
    int i, j;

    j = 0;
    i = iminor(flip->f_ino);

    for (; n > 0; n--) {
        sem_wait(&pipes[i].full);
        if (pipes[i].done) {
            while (j < n && pipes[i].init < pipes[i].end) {
                buf[j++] = pipes[i].buffer[pipes[i].init++];
                if (pipes[i].init == MAX_SIZE) pipes[i].init = 0;
            }
            sem_signal(&pipes[i].full);
            return j;
        }
        sem_wait(&pipes[i].mutex);
        buf[j++] = pipes[i].buffer[pipes[i].init++];
        if (pipes[i].init == MAX_SIZE) pipes[i].init = 0;
        sem_signal(&pipes[i].mutex);
        sem_signal(&pipes[i].empty);
    }

    return j;
}

ssize_t pipe_write(struct file_s *flip, char *buf, size_t n)
{
    int i, j;

    j = 0;
    i = iminor(flip->f_ino);

    while (n--) {
        sem_wait(&pipes[i].empty);
        if (pipes[i].done) {
            sem_signal(&pipes[i].empty);
            return j;
        }
        sem_wait(&pipes[i].mutex);
        pipes[i].buffer[pipes[i].end++] = buf[j++];
        if (pipes[i].end == MAX_SIZE) pipes[i].end = 0;
        sem_signal(&pipes[i].mutex);
        sem_signal(&pipes[i].full);
    }

    return j;
}

int pipe_close(struct file_s *flip)
{
    struct pipe_s *pipe = pipes + iminor(flip->f_ino);

    /* if there's no one left, free pipe */
    if (pipe->ino_read->i_refcount + pipe->ino_write->i_refcount == 1) {
        LIST_INSERT_HEAD(&unused_pipes, pipe, unused);
    /* if readers are closed, release writers (and viceversa) */
    } else if (flip->f_ino->i_refcount == 1) {
        pipe->done = 1;
        sem_signal(&pipe->empty);
        sem_signal(&pipe->full);
    }

    return 0;
}

int pipe_flush(struct file_s *flip)
{
    return 0;
}

static struct file_operations_s ops = {
    .read = pipe_read,
    .write = pipe_write,
    .flush = pipe_flush,
    .close = pipe_close
};

int sys_pipe(int filedes[2])
{
    struct pipe_s *pipe;

    pipe = LIST_FIRST(&unused_pipes);
    if (pipe != NULL) {
        LIST_REMOVE(pipe, unused);

        pipe->done = 0;
        pipe->init = 0;
        pipe->end = 0;

        sem_init(&pipe->empty, MAX_SIZE);
        sem_init(&pipe->full, 0);
        sem_init(&pipe->mutex, 1);

        /* Fake inode for read to keep abstraction */
        pipe->ino_read = get_free_inode();
        pipe->ino_read->i_mode = 0;
        pipe->ino_read->i_zone[0] = DEV_PIPE;
        pipe->ino_read->i_zone[1] = pipe - pipes;
        filedes[0] = get_fd(pipe->ino_read, 0);

        /* Fake inode for write to keep abstraction */
        pipe->ino_write = get_free_inode();
        pipe->ino_write->i_mode = 0;
        pipe->ino_write->i_zone[0] = DEV_PIPE;
        pipe->ino_write->i_zone[1] = pipe - pipes;
        filedes[1] = get_fd(pipe->ino_write, 0);

        return 0;
    }

    return -1;
}

void pipe_init()
{
    int i;

    LIST_INIT(&unused_pipes);
    for (i = 0; i < MAX_PIPES; i++)
        LIST_INSERT_HEAD(&unused_pipes, &pipes[i], unused);

    dev_register(DEV_PIPE, &ops);

    SCALL_REGISTER(42, sys_pipe);
}
