#include <unistd.h>
#include "misc.h"

#define ERR_ARGC "To few arguments\n"
#define ERR_RMDIR "Error in rmdir\n"

void main(int argc, const char *argv[])
{
    if (argc < 2) {
        write(STDOUT_FILENO, ERR_ARGC, sizeof(ERR_ARGC) - 1);
        _exit(1);
    }

    if (rmdir(argv[1]) < 0) {
        write(STDOUT_FILENO, ERR_RMDIR, sizeof(ERR_RMDIR) - 1);
        _exit(1);
    }

    _exit(0);
}
