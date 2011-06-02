#include <minios/misc.h>
#include <minios/dev.h>
#include "ramdisk.h"

static char *pos;
static char *ram_offset;

size_t ramdisk_read(struct file_s *flip, char *buf, size_t n)
{
    mymemcpy(buf, pos, n);

    return n;
}

ssize_t ramdisk_write(struct file_s *flip, char *buf, size_t n)
{
    mymemcpy(pos, buf, n);

    return n;
}


size_t ramdisk_lseek(struct file_s *flip, off_t offset, int whence)
{
    pos = ram_offset + offset;

    return 1;
}

static struct file_operations_s ops = {
    .read = ramdisk_read,
    .write = ramdisk_write,
    .lseek = ramdisk_lseek,
};

void ramdisk_init(u32_t offset)
{
    /* register device */
    dev_register(DEV_RAMDISK, &ops);

    pos = ram_offset = (char *) offset;
}
