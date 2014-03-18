#include <minios/dev.h>

/* temporary, will see what is the better way to load drivers */
#include "../drivers/ramdisk/ramdisk.h"
#include "../drivers/serial/serial.h"
#include "../drivers/pipe/pipe.h"
#include "../drivers/tty/con.h"
#include "../drivers/hdd/hdd.h"

#define FS_INITIAL_POS 0x30000

struct dev_s devices[MAX_DEVICES];

void dev_init(void)
{
    ramdisk_init(FS_INITIAL_POS);
    hdd_init();
    con_init();
    serial_init();
    pipe_init();
}

struct file_operations_s *dev_operations(dev_t major)
{
    if (major >= MAX_DEVICES)
        return NULL;
    return &devices[major].d_op;
}

int dev_register(dev_t major, struct file_operations_s *fops)
{
    if (major >= MAX_DEVICES)
        return -1;
    devices[major].d_op = *fops;
    return 0;
}
