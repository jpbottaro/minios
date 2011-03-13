#include <unistd.h>
#include "misc.h"

void main(int argc, const char *argv[])
{
    if (argc > 1)
        unlink(argv[1]); 

    _exit(0);
}
