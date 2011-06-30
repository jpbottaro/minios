#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define ERR_PIPE "Error in pipe()\n"
#define ERR_FORK "Error in fork()\n"
#define HELLO "How you doin'?\n"

/* Read characters from the pipe and echo them to stdout. */
/* Taken from http://www.gnu.org/s/hello/manual/libc/Creating-a-Pipe.html */

void read_from_pipe(int file)
{
    char c;

    while (read(file, &c, 1) > 0)
        write(STDOUT_FILENO, &c, 1);
}

/* Write some random text to the pipe. */

void write_to_pipe(int file)
{
    write(file, HELLO, sizeof(HELLO) - 1);
}

int main(void)
{
    pid_t pid;
    int mypipe[2];

    /* Create the pipe. */
    if (pipe(mypipe)) {
        write(STDOUT_FILENO, ERR_PIPE, sizeof(ERR_PIPE) - 1);
        return -1;
    }

    /* Create the child process. */
    pid = fork();
    if (pid == (pid_t) 0) {
        /* This is the child process.
           Close other end first. */
        close(mypipe[1]);
        read_from_pipe(mypipe[0]);
        return -1;
    } else if (pid < (pid_t) 0) {
        /* The fork failed. */
        write(STDOUT_FILENO, ERR_FORK, sizeof(ERR_FORK) - 1);
        return -1;
    } else {
        /* This is the parent process.
           Close other end first. */
        close(mypipe[0]);
        write_to_pipe(mypipe[1]);
        return -1;
    }
}
