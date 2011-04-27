#include <minios/fs.h>
#include <minios/vga.h>
#include <minios/dev.h>
#include <minios/sched.h>
#include "kbd.h"

int dev_io(unsigned int dev, char *buf, int n, int flag)
{
    switch (dev) {
        case DEV_STDIN:
            if (flag == FS_READ) {
                sched_block(current_process, dev);
                return kbd_getline(buf, n);
            }
            break;
        case DEV_STDOUT:
        case DEV_STDERR:
            if (flag == FS_WRITE)
                return vga_write(buf, n);
            break;
        default:
            break;
    }

    return 0;
}
