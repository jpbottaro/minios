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
    int len, from, to;

    if ( (from = open(argv[1], O_RDONLY, 0)) < 0) {
        write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN));
        _exit(-1);
    }

    if ( (to = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 744)) < 0) {
        write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN));
        _exit(-1);
    }

    if ( (len = read(from, buf, MAX_BUF)) < 0 ) {
        write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ));
        _exit(-1);
    }
    while (len > 0) {
        if (write(to, buf, len) != len) {
            write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE));
            _exit(-1);
        }
        if ( (len = read(from, buf, MAX_BUF)) < 0) {
            write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ));
            _exit(-1);
        }
    }
    close(from);
    close(to);

    _exit(0);
}
