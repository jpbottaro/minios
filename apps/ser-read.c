#include <unistd.h>
#include <fcntl.h>
#include "misc.h"

#define MAX_BUF 1024

#define ERR_ARGC "Error in arguments\n"
#define ERR_OPEN "Error in open()\n"
#define ERR_READ "Error in read()\n"
#define ERR_WRITE "Error in write()\n"

void main(int argc, char *argv[])
{
    char buf[MAX_BUF];
    int len, serial;

    if ( (serial = open("/dev/serial", O_RDWR, 0)) < 0) {
        write(STDOUT_FILENO, ERR_OPEN, sizeof(ERR_OPEN) - 1);
        _exit(-1);
    }

    if ( (len = read(serial, buf, MAX_BUF)) < 0 ) {
        write(STDOUT_FILENO, ERR_READ, sizeof(ERR_READ) - 1);
        _exit(-1);
    }

    if (write(STDOUT_FILENO, buf, len) != len) {
        write(STDOUT_FILENO, ERR_WRITE, sizeof(ERR_WRITE) - 1);
        _exit(-1);
    }

    close(serial);

    _exit(0);
}
