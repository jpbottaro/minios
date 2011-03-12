#include <unistd.h>
#include "misc.h"

#define MAX_BUF 100
#define MOTD "echo application, write something and it echoes it\n"

void main()
{
    char buf[MAX_BUF];
    int len;

    write(1, MOTD, sizeof(MOTD));
    len = read(0, buf, MAX_BUF);
    while (len != 0 && mystrncmp(buf, "exit", 4) != 0) {
        write(1, buf, len);
        len = read(0, buf, MAX_BUF);
    }

    exit(0);
}
