#include <minios/sched.h>
#include <minios/vga.h>
#include <minios/dev.h>
#include <errno.h>
#include "kbd.h"

struct dev_s devices[MAX_DEVICES];

void dev_init(void)
{

}

/* assign file operations of device */
int dev_file_calls(struct file_s *flip, dev_t dev)
{
    if (dev >= MAX_DEVICES)
        return -1;
    flip->f_op = &devices[dev].d_op;
    return 0;
}

int dev_register(unsigned int major, struct file_operations_s *fops)
{
    if (major >= MAX_DEVICES)
        return -1;
    devices[major].d_op = *fops;
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
