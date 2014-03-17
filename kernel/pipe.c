#include <minios/scall.h>
#include <minios/pipe.h>
#include <minios/misc.h>
#include <minios/sem.h>
#include <minios/dev.h>

struct pipe_s {
    int i;
    char buffer[MAX_SIZE];
    int init;
    int end;
    int done;
    sem_t mutex, empty, full;

    struct inode_s ino;

    LIST_ENTRY(pipe_s) unused;
} pipes[MAX_PIPES];

LIST_HEAD(unused_pipe_t, pipe_s) unused_pipes;

size_t pipe_read(struct file_s *flip, char *buf, size_t n)
{
    int i, j;

    j = 0;
    i = iminor(flip->f_ino);

    if (pipes[i].done) {
        while (j < n && pipes[i].init < pipes[i].end) {
            buf[j++] = pipes[i].buffer[pipes[i].init++];
            if (pipes[i].init == MAX_SIZE) pipes[i].init = 0;
        }
        return j;
    }

    for (; n > 0; n--) {
        sem_wait(&pipes[i].full);
        if (pipes[i].done) {
            while (j < n && pipes[i].init < pipes[i].end) {
                buf[j++] = pipes[i].buffer[pipes[i].init++];
                if (pipes[i].init == MAX_SIZE) pipes[i].init = 0;
            }
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

    if (pipes[i].done)
        return n;

    while (n--) {
        sem_wait(&pipes[i].empty);
        if (pipes[i].done)
            return j;
        sem_wait(&pipes[i].mutex);
        pipes[i].buffer[pipes[i].end++] = buf[j++];
        if (pipes[i].end == MAX_SIZE) pipes[i].end = 0;
        sem_signal(&pipes[i].mutex);
        sem_signal(&pipes[i].full);
    }

    return j;
}

int pipe_flush(struct file_s *flip)
{
    return 0;
}

int pipe_close(struct file_s *flip)
{
    int i = iminor(flip->f_ino);

    /* release anybody that might be waiting */
    pipes[i].done = 1;
    sem_signal(&pipes[i].full);
    sem_signal(&pipes[i].empty);

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

        /* Fake inode to keep abstraction */
        pipe->ino.i_mode = 0;
        pipe->ino.i_refcount = -1;
        pipe->ino.i_zone[0] = DEV_PIPE;
        pipe->ino.i_zone[1] = pipe - pipes;

        sem_init(&pipe->empty, MAX_SIZE);
        sem_init(&pipe->full, 0);
        sem_init(&pipe->mutex, 1);

        filedes[0] = get_fd(&pipe->ino, 0);
        filedes[1] = get_fd(&pipe->ino, 0);

        return 0;
    }

    return -1;
}

void pipe_init()
{
    int i;

    LIST_INIT(&unused_pipes);
    for (i = 0; i < MAX_PIPES; i++) {
        pipes[i].i = i;
        LIST_INSERT_HEAD(&unused_pipes, &pipes[i], unused);
    }

    dev_register(DEV_PIPE, &ops);

    SCALL_REGISTER(42, sys_pipe);
}
