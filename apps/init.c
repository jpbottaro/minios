#include <unistd.h>
#include <fcntl.h>
#include "misc.h"

#define MAX_BUF 1024
#define MAX_CONSOLES 3

void main(int argc, char *argv[])
{
    char tty[] = "/dev/tty0";
    int i;

    for (i = 0; i < MAX_CONSOLES; ++i) {
        tty[8] = i + '0';
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        open(tty, O_RDWR, 0);
        open(tty, O_RDWR, 0);
        open(tty, O_RDWR, 0);
        newprocess("/bin/cash", NULL);
    }

    _exit(0);
}
