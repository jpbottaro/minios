#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define ERR_ARGC "Too few arguments\n"
#define ERR_OPEN "Error in open()\n"
#define ERR_PIPE "Error in pipe()\n"
#define ERR_FORK1 "Error in first fork()\n"
#define ERR_FORK2 "Error in second fork()\n"
#define HELLO "How you doin'?\n"

#define READ_SIZE 0x1000

/* Output to stdout the contents of a pipe */
int reader(int fd)
{
    int r;
    char buf[READ_SIZE];

    while ( (r = read(fd, buf, READ_SIZE) > 0))
        write(STDOUT_FILENO, buf, r);

    close(fd);
    return 0;
}

/* Encrypt the contents of a pipe and output to another */
int encrypt(int fd_from, int fd_to)
{
    close(fd_from);
    close(fd_to);
    return 0;
}


/* Write a file to the pipe */
int writer(int file, int fd)
{
    close(file);
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int fd;
    int writer2encrypt[2];
    int encrypt2reader[2];

    if (argc < 2) {
        write(STDOUT_FILENO, ERR_ARGC, sizeof(ERR_ARGC) - 1);
        _exit(-1);
    }

    /* Create 2 pipes. */
    if (pipe(encrypt2reader) && pipe(writer2encrypt)) {
        write(STDOUT_FILENO, ERR_PIPE, sizeof(ERR_PIPE) - 1);
        return -1;
    }

    pid = fork();
    if (pid < (pid_t) 0) {
        write(STDOUT_FILENO, ERR_FORK1, sizeof(ERR_FORK1) - 1);
        return -1;
    } else if (pid == (pid_t) 0) {
        close(writer2encrypt[0]);
        close(writer2encrypt[1]);
        close(encrypt2reader[1]);

        /* READER process. */
        return reader(encrypt2reader[0]);
    } else {
        pid = fork();
        if (pid < (pid_t) 0) {
            write(STDOUT_FILENO, ERR_FORK2, sizeof(ERR_FORK2) - 1);
            return -1;
        } else if (pid == (pid_t) 0) {
            close(encrypt2reader[1]);
            close(writer2encrypt[1]);

            /* ENCRYPT process. */
            return encrypt(writer2encrypt[0], encrypt2reader[1]);
        } else {
            close(encrypt2reader[0]);
            close(encrypt2reader[1]);
            close(writer2encrypt[0]);

            /* Open the file to encrypt */
            if ( (fd = open(argv[1], O_RDONLY, 0)) < 0) {
                write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN) - 1);
                return -1;
            }

            /* WRITER process. */
            return writer(fd, writer2encrypt[1]);
        }
    }

    /* unreachable */
    return -1;
}
