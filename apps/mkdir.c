#include <unistd.h>
#include "misc.h"

#define ERR_ARGC "To few arguments\n"
#define ERR_MKDIR "Error in mkdir\n"

void main(int argc, const char *argv[])
{
    if (argc < 2) {
        write(STDOUT_FILENO, ERR_ARGC, sizeof(ERR_ARGC) - 1);
        _exit(1);
    }

    if (mkdir(argv[1], 0) < 0) {
        write(STDOUT_FILENO, ERR_MKDIR, sizeof(ERR_MKDIR) - 1);
        _exit(1);
    }

    _exit(0);
}
