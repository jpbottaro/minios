/* HEAVILY based on at_wini driver from minix v3 */

#include <minios/i386.h>
#include <minios/debug.h>
#include <minios/debug.h>
#include <minios/dev.h>
#include <minios/idt.h>
#include "hdd.h"

#define ULONG_MAX 4294967295UL

/* Common command block */
struct command {
    u8_t	precomp;	/* REG_PRECOMP, etc. */
    u8_t	count;
    u8_t	sector;
    u8_t	cyl_lo;
    u8_t	cyl_hi;
    u8_t	command;

    /* The following at for LBA48 */
    u8_t	count_prev;
    u8_t	sector_prev;
    u8_t	cyl_lo_prev;
    u8_t	cyl_hi_prev;
};

static struct wini {
    unsigned state;         /* drive state: deaf, initialized, dead */
    unsigned short w_status;    /* device status register */
    unsigned lcylinders;        /* logical number of cylinders (BIOS) */
    unsigned lheads;        /* logical number of heads */
    unsigned lsectors;      /* logical number of sectors per track */
    unsigned pcylinders;    /* physical number of cylinders (translated) */
    unsigned pheads;        /* physical number of heads */
    unsigned psectors;      /* physical number of sectors per track */
    unsigned open_ct;       /* in-use count */
    unsigned long size;     /* total size */
} wn;

u8_t tmp_buf[SECTOR_SIZE];

#define id_byte(n)	(&tmp_buf[2 * (n)])
#define id_word(n)	(((u16_t) id_byte(n)[0] <<  0) \
        | ((u16_t) id_byte(n)[1] <<  8))
#define id_longword(n)	(((u32_t) id_byte(n)[0] <<  0) \
        | ((u32_t) id_byte(n)[1] <<  8) \
        | ((u32_t) id_byte(n)[2] << 16) \
        | ((u32_t) id_byte(n)[3] << 24))

int w_waitfor(mask, value)
{
    int s;

    /* wtf polling ! we should have a timeout here or smth */
    while ( ((s = inb(HDD_CMD + REG_STATUS)) & mask) != value)
        ;
    wn.w_status = s;

    return 1;
}

int com_out(struct command *cmd)
{
    w_waitfor(STATUS_BSY, 0);

    wn.w_status = STATUS_ADMBSY;
    outb(HDD_CTL + REG_CTL, wn.pheads >= 8 ? CTL_EIGHTHEADS : 0);
    outb(HDD_CMD + REG_PRECOMP, cmd->precomp);
    outb(HDD_CMD + REG_COUNT, cmd->count);
    outb(HDD_CMD + REG_SECTOR, cmd->sector);
    outb(HDD_CMD + REG_CYL_LO, cmd->cyl_lo);
    outb(HDD_CMD + REG_CYL_HI, cmd->cyl_hi);
    outb(HDD_CMD + REG_COMMAND, cmd->command);

    return 0;
}

int at_intr_wait()
{
    /* Wait for an interrupt */
    int r;

    w_waitfor(STATUS_BSY, 0);
    if ((wn.w_status & (STATUS_BSY | STATUS_WF | STATUS_ERR)) == 0) {
        r = 0;
    } else {
        debug_panic("at_intr_wait: wtf error");
    }
    wn.w_status |= STATUS_ADMBSY;   /* assume still busy with I/O */
    return r;
}

int com_simple(struct command *cmd)
{
    /* A controller command, only one interrupt and no data-out phase. */
    int r;

    if (com_out(cmd) == 0)
        r = at_intr_wait();

    return r;
}

int get_data(int port, u8_t *buf, int size)
{
    int i;
    for (i = 0; i < size; i++)
        buf[i] = inb(port);
    return 0;
}

static int identify()
{
    int r;
    struct command cmd;

    /* Select drive. */
    outb(HDD_CMD + REG_LDH, LDH_DEFAULT);
    inb(HDD_CMD + REG_STATUS);
    inb(HDD_CMD + REG_STATUS);
    inb(HDD_CMD + REG_STATUS);
    inb(HDD_CMD + REG_STATUS);
    w_waitfor(STATUS_BSY, 0);

    cmd.command = ATA_IDENTIFY;
    r = com_simple(&cmd);

    if (r == 0 && w_waitfor(STATUS_DRQ, STATUS_DRQ) &&
            !(wn.w_status & (STATUS_ERR|STATUS_WF))) {

        /* Device information. */
        if (get_data(HDD_CMD + REG_DATA, tmp_buf, SECTOR_SIZE) != 0)
            debug_panic("identify: call to get_data() failed");

        wn.pcylinders = id_word(1);
        wn.pheads = id_word(3);
        wn.psectors = id_word(6);

        /* assume LBA28 */
        wn.size = id_word(60);
        
        /* assume LBA48 */
        //wn.size = id_longword(100);
    }

    return 0;
}

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

    /* make char device in /dev */
    //fs_make_dev("hdd", I_BLOCK, DEV_HDD, 0);

    /* register interruption handler */
    idt_register(46, hdd_intr, DEFAULT_PL);

    /* register device */
    dev_register(DEV_HDD, &ops);
}
