#include <minikernel/dev.h>
#include <minikernel/fs.h>
#include "keyboardscreen.h"

int dev_io(unsigned int dev, char *buf, unsigned int n, int flag)
{
    switch (dev) {
        case DEV_STDIN:
            if (flag == FS_WRITE)
                print(buf, n);
            break;
        case DEV_STDOUT:
        case DEV_STDERR:
            if (flag == FS_READ)
                get_line(buf, n);
            break;
        default:
            break;
    }

    return 0;
}