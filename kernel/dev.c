#include <minios/sched.h>
#include <minios/vga.h>
#include <minios/dev.h>
#include <errno.h>
#include "kbd.h"

void dev_init(void)
{
}

/* assign file operations of device */
int dev_file_calls(struct file_s *flip, dev_t dev)
{
    return 0;
}
/*
    switch (dev) {
        case DEV_STDIN:
            if (flag == FS_READ) {
                sched_block(current_process, &kbd_list);
                sched_schedule(1);
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
*/
