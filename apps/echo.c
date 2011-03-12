#include <unistd.h>
#include "misc.h"

#define MAX_BUF 100

void main(int argc, const char *argv[])
{
    int len;

    len = mystrlen(argv[1]);
    write(1, argv[1], len);
    write(1, "\n", 1);

    _exit(0);
}
