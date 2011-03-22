#include <unistd.h>
#include <fcntl.h>

#define ERR_ARGC "Too few arguments\n"
#define ERR_RENAME "Error in rename()\n"

void main(int argc, char *argv[])
{
    if (argc < 3) {
        write(STDOUT_FILENO, ERR_ARGC, sizeof(ERR_ARGC));
        _exit(-1);
    }

    if (rename(argv[1], argv[2]) < 0) {
        write(STDOUT_FILENO, ERR_RENAME, sizeof(ERR_RENAME));
        _exit(-1);
    }

    _exit(0);
}
