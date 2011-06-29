#include <minios/scall.h>
#include <minios/pipe.h>
#include <minios/sem.h>
#include <minios/fs.h>

struct pipe_s {
    int i;
    unsigned char buffer[MAX_SIZE];
    int pos;
    int done;
    sem_t mutex, empty, full;

    LIST_ENTRY(pipe_s) unused;
} pipes[MAX_PIPES];

LIST_HEAD(unused_pipe_t, pipe_s) unused_pipes;

size_t pipe_read(struct file_s *flip, char *buf, size_t n)
{
    int i, j;

    j = 0;
    i = flip->pipe_nr;

    if (pipes[i].done)
        return -1;

    while (n--) {
        sem_wait(&pipes[i].full);
        if (pipes[i].done)
            return -1;
        sem_wait(&pipes[i].mutex);
        buf[j++] = pipes[i].buffer[--pipes[i].pos];
        sem_signal(&pipes[i].mutex);
        sem_signal(&pipes[i].empty);
    }

    return j;
}

ssize_t pipe_write(struct file_s *flip, char *buf, size_t n)
{
    int i, j;

    j = 0;
    i = flip->pipe_nr;

    if (pipes[i].done)
        return -1;

    while (n--) {
        sem_wait(&pipes[i].empty);
        if (pipes[i].done)
            return -1;
        sem_wait(&pipes[i].mutex);
        pipes[i].buffer[pipes[i].pos++] = buf[j++];
        sem_signal(&pipes[i].mutex);
        sem_signal(&pipes[i].full);
    }

    return j;
}

int pipe_flush(struct file_s *flip)
{
    int i = flip->pipe_nr;

    /* release anybody that might be waiting */
    pipes[i].done = 1;
    sem_signal(&pipes[i].full);
    sem_signal(&pipes[i].empty);

    /* release fd */
    release_fd_pipe(flip->f_fd);
    return 0;
}

size_t pipe_readnot(struct file_s *flip, char *buf, size_t n)
{
    return -1;
}

ssize_t pipe_writenot(struct file_s *flip, char *buf, size_t n)
{
    return -1;
}

static struct file_operations_s ops_read = {
    .read = pipe_read,
    .write = pipe_writenot,
    .flush = pipe_flush
};

static struct file_operations_s ops_write = {
    .read = pipe_readnot,
    .write = pipe_write,
    .flush = pipe_flush
};

int sys_pipe(int filedes[2])
{
    struct pipe_s *pipe;

    pipe = LIST_FIRST(&unused_pipes);
    if (pipe != NULL) {
        LIST_REMOVE(pipe, unused);

        pipe->pos = 0;
        pipe->done = 0;
        sem_init(&pipe->empty, MAX_SIZE);
        sem_init(&pipe->full, 0);
        sem_init(&pipe->mutex, 1);

        filedes[0] = get_fd_pipe(&ops_read, pipe->i);
        filedes[1] = get_fd_pipe(&ops_write, pipe->i);
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

    SCALL_REGISTER(42, sys_pipe);
}