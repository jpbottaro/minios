#include <minios/dev.h>

/* temporary, will see what is the better way to load drivers */
#include "../drivers/tty/con.h"
#include "../drivers/ramdisk/ramdisk.h"
#include "../drivers/hdd/hdd.h"
#include "../drivers/serial/serial.h"

#define FS_INITIAL_POS 0x30000

struct dev_s devices[MAX_DEVICES];

void dev_init(void)
{
    ramdisk_init(FS_INITIAL_POS);
    hdd_init();
    con_init();
    serial_init();
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
