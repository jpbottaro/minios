#include <unistd.h>
#include <fcntl.h>
#include "misc.h"

#define MAX_BUF 1024

#define ERR_OPEN "Error in open()\n"
#define ERR_READ "Error in read()\n"
#define ERR_WRITE "Error in write()\n"

void main(int argc, char *argv[])
{
    char buf[MAX_BUF];
    int i, len, fd;

    for (i = 1; i < argc; i++) {
        if ( (fd = open(argv[i], O_RDONLY, 0)) < 0) {
            write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN) - 1);
            _exit(-1);
        }
        if ( (len = read(fd, buf, MAX_BUF)) < 0) {
            write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ) - 1);
            _exit(-1);
        }
        while (len > 0) {
            if (write(STDOUT_FILENO, buf, len) != len) {
                write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE) - 1);
                _exit(-1);
            }
            if ( (len = read(fd, buf, MAX_BUF)) < 0) {
                write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ) - 1);
                _exit(-1);
            }
        }
        close(fd);
    }

    _exit(0);
}
