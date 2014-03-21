#include <unistd.h>
#include "misc.h"

#define MAX_BUF 100
#define MOTD "Every line entered results in a page requested (empty line to exit)\n"

void main()
{
    int len, *a;
    char buf[MAX_BUF];

    write(1, MOTD, sizeof(MOTD) - 1);
    len = read(0, buf, MAX_BUF);
    while (len != 0 && *buf != '\n') {
        a = (int *) palloc();
        *a = 0;
        len = read(0, buf, MAX_BUF);
    }

    _exit(0);
}
