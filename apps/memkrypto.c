#include <minios/misc.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define ERR_ARGC "Too few arguments\n"
#define ERR_OPEN "Error in open()\n"
#define ERR_READ "Error in read()\n"
#define ERR_WRITE "Error in write()\n"
#define ERR_PIPE "Error in pipe()\n"
#define ERR_FORK1 "Error in first fork()\n"
#define ERR_FORK2 "Error in second fork()\n"

#define READ_SIZE 0x1000

/* output to stdout the contents of a pipe */
int reader(int fd, int file, char *buf, int *len)
{
    int tmp;

    while (read(fd, &tmp, 1) > 0) {
        if (write(file, buf, *len) != *len) {
            write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE) - 1);
            _exit(-1);
        }
    }

    close(file);
    close(fd);
    return 0;
}

/* negate a buffer (which is the same as a XOR with a 111... key) */
void negate(char *buf, int r)
{
    char *b, *end;

    if (r < 0)
        return;

    b = buf;
    end = buf + r;
    while (b < end)
        *(b++) ^= ~0;
}

/* encrypt the contents of a pipe and output to another */
int encrypt(int fd_from, char *buf_from, int *len_from,
            int fd_to,   char *buf_to,   int *len_to)
{
    int tmp;

    while (read(fd_from, &tmp, 1) > 0) {
        mymemcpy(buf_to, buf_from, *len_from);
        negate(buf_to, *len_from);
        *len_to = *len_from;
        if (write(fd_to, &tmp, 1) != 1) {
            write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE) - 1);
            _exit(-1);
        }
    }

    close(fd_from);
    close(fd_to);
    return 0;
}

/* write a file to the pipe */
int writer(int file, int fd, char *buf, int *len)
{
    int tmp;

    while ( (*len = read(file, buf, READ_SIZE)) > 0) {
        if (write(fd, &tmp, 1) != 1) {
            write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE) - 1);
            _exit(-1);
        }
    }

    if (*len < 0) {
        write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ) - 1);
        _exit(-1);
    }

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
    char *buf_writer, *buf_reader;
    int *r;

    if (argc < 2) {
        write(STDOUT_FILENO, ERR_ARGC, sizeof(ERR_ARGC) - 1);
        _exit(-1);
    }

    /* create 2 pipes */
    if (pipe(encrypt2reader) < 0 || pipe(writer2encrypt) < 0) {
        write(STDOUT_FILENO, ERR_PIPE, sizeof(ERR_PIPE) - 1);
        return -1;
    }

    buf_reader = (char *) palloc();
    buf_writer = (char *) palloc();
    r = (int *) palloc();

    pid = fork();
    if (pid < (pid_t) 0) {
        write(STDOUT_FILENO, ERR_FORK1, sizeof(ERR_FORK1) - 1);
        return -1;
    } else if (pid == (pid_t) 0) {
        close(writer2encrypt[0]);
        close(writer2encrypt[1]);
        close(encrypt2reader[1]);

        if (argc > 2) {
            if ( (fd = open(argv[2], O_CREAT | O_RDWR, 0)) < 0) {
                write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN) - 1);
                return -1;
            }
        } else {
            fd = STDOUT_FILENO;
        }

        /* READER process */
        return reader(encrypt2reader[0], fd, buf_reader, r+1);
    } else {
        pid = fork();
        if (pid < (pid_t) 0) {
            write(STDOUT_FILENO, ERR_FORK2, sizeof(ERR_FORK2) - 1);
            return -1;
        } else if (pid == (pid_t) 0) {
            close(encrypt2reader[0]);
            close(writer2encrypt[1]);

            /* ENCRYPT process */
            return encrypt(writer2encrypt[0], buf_writer, r,
                           encrypt2reader[1], buf_reader, r+1);
        } else {
            close(encrypt2reader[0]);
            close(encrypt2reader[1]);
            close(writer2encrypt[0]);

            /* open the file to encrypt */
            if ( (fd = open(argv[1], O_RDONLY, 0)) < 0) {
                write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN) - 1);
                return -1;
            }

            /* WRITER process */
            return writer(fd, writer2encrypt[1], buf_writer, r);
        }
    }

    return 0;
}
