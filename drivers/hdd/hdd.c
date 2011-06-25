#include <minios/i386.h>
#include <minios/debug.h>
#include <minios/dev.h>
#include <minios/idt.h>
#include "hdd.h"

size_t hdd_read(struct file_s *flip, char *buf, size_t n)
{
    return 0;
}

ssize_t hdd_write(struct file_s *flip, char *buf, size_t n)
{
    return 0;
}

int hdd_flush(struct file_s *flip)
{
    return 0;
}

static struct file_operations_s ops = {
    .read = hdd_read,
    .write = hdd_write,
    .flush = hdd_flush
};

static int identify()
{
    return 0;
}

static int io_test()
{
    return 0;
}

void hdd_handler()
{

}

extern void hdd_intr();

void hdd_init()
{
    /* preparar struct */

    /* Try to identify the device. */
    if (identify() != 0)
        debug_panic("hdd_init(): cannot find hdd");

    /* Do a test transaction */
    if (io_test() != 0)
        debug_panic("hdd_init(): failed test");

    /* make char device in /dev */
    //fs_make_dev("hdd", I_BLOCK, DEV_HDD, 0);

    /* register interruption handler */
    idt_register(46, hdd_intr, DEFAULT_PL);

    /* register device */
    dev_register(DEV_HDD, &ops);
}
