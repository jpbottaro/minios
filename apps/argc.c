#include <unistd.h>
#include "misc.h"

void main(int argc, const char *argv[])
{
    char buf[3] = " \n";

    *buf = '0' + argc;
    write(1, buf, 2);

    _exit(0);
}
