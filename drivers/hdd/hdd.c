/* This driver is _heavily_ based in a version made by Ezequiel Aguerre,
 * and is intended for public use.
 */

#include <minios/debug.h>
#include <minios/i386.h>
#include <minios/misc.h>
#include <minios/sem.h>
#include <minios/dev.h>
#include "hdd.h"

struct ata_controller {
    u16_t port;
    u16_t cport;
    u8_t selected_drive;
    u8_t last_status;
    sem_t sem_transfer;
} controller;

static struct ide_device {
    int present;
    int drive;
    int type;
    int signature;
    int capabilities;
    int cmdset;
    u32_t size_in_sectors;
    int lba48;
    struct ata_controller *controller;
} drive;

static u8_t temp[ATA_SECTOR_SIZE];

void hdd_wait_idle(struct ata_controller *ata)
{
    u8_t status;
    int limit = 1000;

    /* wait 10ms and then timeout */
    do {
        udelay(10);
        status = inb(ata->port + ATA_REG_STATUS);
        limit--;
    } while (status != 0xff && (status & (ATA_SR_BSY | ATA_SR_DRQ)) && limit);
}

int hdd_wait_status(struct ata_controller *ata)
{
    u8_t status;
    int i;

    for (i = 0; i < 1000; i++) {
        status = inb(ata->port + ATA_REG_STATUS);
        if ((status & (ATA_SR_BSY | ATA_SR_DRQ)) == ATA_SR_DRQ)
            return 0;
        if (status & (ATA_SR_ERR | ATA_SR_DF))
            return -1;
        if (!status)
            return -1;

        udelay(10);
    }

    return -1;
}

int hdd_select(struct ide_device *ide)
{
    struct ata_controller *ata = ide->controller;
    u8_t drive = 0xA0 | (ide->drive << 4);

    if (ata->selected_drive == drive)
        return 0;

    hdd_wait_idle(ata);

    outb(ata->port + ATA_REG_HDDEVSEL, drive);
    inb(ata->cport + ATA_REG_ALTSTATUS);
    ATA_DELAY();

    hdd_wait_idle(ata);

    ata->selected_drive = drive;
    return 0;
}

void hdd_reset(struct ata_controller *ata)
{
    outb(ata->cport + ATA_REG_CONTROL, ATA_CTRL_SRST);
    ATA_DELAY();
    outb(ata->cport + ATA_REG_CONTROL, ATA_CTRL_NIEN);
    ATA_DELAY();

    hdd_wait_idle(ata);
}

void hdd_identify(struct ide_device *ide) {
    struct ata_controller *ata = ide->controller;

    /* select device */
    if (hdd_select(ide)) {
        ide->present = 0;
        return;
    }

    /* ask to identify */
    outb(ata->port + ATA_REG_SECCOUNT, ATA_CMD_IDENTIFY);
    outb(ata->port + ATA_REG_LBA_LOW, ATA_CMD_IDENTIFY);
    outb(ata->port + ATA_REG_LBA_MED, ATA_CMD_IDENTIFY);
    outb(ata->port + ATA_REG_LBA_HIGH, ATA_CMD_IDENTIFY);
    outb(ata->port + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ATA_DELAY();
    hdd_wait_idle(ata);

    if (hdd_wait_status(ata)) {
        ide->present = 0;
        return;
    }

    insw(ata->port + ATA_REG_DATA, temp, 256);

    ide->present = 1;
    ide->type = IDE_ATA;
    ide->signature = *((u16_t *) (temp + ATA_IDENT_DEVICETYPE));
    ide->capabilities = *((u16_t *) (temp + ATA_IDENT_CAPABILITIES));
    ide->cmdset = *((u32_t *) (temp + ATA_IDENT_COMMANDSETS));
    ide->lba48 = (ide->cmdset & (1 << 26)) == (1 << 26);

    if (ide->lba48)
        ide->size_in_sectors = *((u32_t *) (temp + ATA_IDENT_MAX_LBA_EXT));
    else
        ide->size_in_sectors = *((u32_t *) (temp + ATA_IDENT_MAX_LBA));
}

/* quik n dirty */
size_t hdd_lseek(struct file_s *flip, off_t offset, int whence)
{
    flip->f_pos = offset;
    return 0;
}

static int hdd_pio_transfer(struct ide_device *ide, u32_t lba,
                            u32_t seccount, void *buffer, int flag) {
    struct ata_controller *ata = ide->controller;
    u8_t cmd = flag == ATA_READ ? ATA_CMD_READ_PIO : ATA_CMD_WRITE_PIO;
    u8_t flush = ATA_CMD_CACHE_FLUSH;

    ATA_USE_TRANSFER(ata);
    if (ide->lba48) {
        outb(ata->port + ATA_REG_HDDEVSEL, 0x40 | (ide->drive << 4));
        outb(ata->port + ATA_REG_SECCOUNT, (u8_t) (seccount >> 8));
        outb(ata->port + ATA_REG_LBA_LOW, (u8_t) (lba >> 24));
        outb(ata->port + ATA_REG_LBA_MED, 0); /* LBA32 */
        outb(ata->port + ATA_REG_LBA_HIGH, 0); /* LBA32 */
        cmd = flag == ATA_READ ? ATA_CMD_READ_PIO_EXT : ATA_CMD_WRITE_PIO_EXT;
        flush = ATA_CMD_CACHE_FLUSH_EXT;
    } else {
        outb(ata->port + ATA_REG_HDDEVSEL, 0xE0 | (ide->drive << 4) | ((lba >> 24) & 0x0F));
    }

    outb(ata->port + ATA_REG_SECCOUNT, (u8_t) seccount);
    outb(ata->port + ATA_REG_LBA_LOW, (u8_t) lba);
    outb(ata->port + ATA_REG_LBA_MED, (u8_t) (lba >> 8));
    outb(ata->port + ATA_REG_LBA_HIGH, (u8_t) (lba >> 16));
    outb(ata->port + ATA_REG_COMMAND, cmd);

    if (flag == ATA_READ) {
        while (seccount--) {
            if (hdd_wait_status(ata))
                goto err;
            insw(ata->port + ATA_REG_DATA, buffer, 256);
            buffer += ATA_SECTOR_SIZE;
        }
    } else {
        if (hdd_wait_status(ata))
            goto err;
        /* cant do outsw for writes, we need to do it by hand */
        while (seccount--) {
            outsw_delay(ata->port + ATA_REG_DATA, buffer, 256);
            buffer += ATA_SECTOR_SIZE;
            if (hdd_wait_status(ata))
                goto err;
        }
        outb(ata->port + ATA_REG_COMMAND, flush);
        if (hdd_wait_status(ata))
            goto err;
    }
    ATA_FREE_TRANSFER(ata);

    return 0;

err:
    ATA_FREE_TRANSFER(ata);
    return -1;
}

static int hdd_pio_read(struct ide_device *ide, u32_t lba, u32_t seccount, void *buffer)
{
    return hdd_pio_transfer(ide, lba, seccount, buffer, ATA_READ);
}
static int hdd_pio_write(struct ide_device *ide, u32_t lba, u32_t seccount, const void *buffer)
{
    return hdd_pio_transfer(ide, lba, seccount, (void *) buffer, ATA_WRITE);
}

size_t hdd_read(struct file_s *flip, char *buf, size_t n)
{
    u32_t first_sector = ATA_TO_SECTOR(flip->f_pos);
    u32_t last_sector = ATA_TO_SECTOR((flip->f_pos + n + ATA_SECTOR_SIZE - 1));
    u32_t seccount;
    u32_t orig_size;
    u32_t temp_off, buf_off, sz;

    /* check limits */
    if (last_sector > drive.size_in_sectors) {
        debug_panic("hdd_read: trying to read beyond disk limit");
        last_sector = drive.size_in_sectors;
        n = drive.size_in_sectors * ATA_SECTOR_SIZE - flip->f_pos;
    }
    seccount = last_sector - first_sector;
    orig_size = n;

    /* check if it is easy */
    if (flip->f_pos == (first_sector << 9) && n % ATA_SECTOR_SIZE == 0) {
        if (hdd_pio_read(&drive, first_sector, seccount, buf))
            return -1;
        return n;
    }

    /* read the unaligned first sector */
    temp_off = flip->f_pos % ATA_SECTOR_SIZE;
    sz = MIN(n, ATA_SECTOR_SIZE - temp_off);
    if (hdd_pio_read(&drive, first_sector, 1, temp))
        return -1;
    mymemcpy(buf, (char *) (temp + temp_off), sz);

    /* read the middle part */
    first_sector++;
    n -= sz;
    buf_off = sz;
    while (n >= ATA_SECTOR_SIZE) {
        if (hdd_pio_read(&drive, first_sector, 1, buf + buf_off))
            return -1;
        n -= ATA_SECTOR_SIZE;
        buf_off += ATA_SECTOR_SIZE;
        first_sector++;
    }

    /* read the (possibly) unaligned last sector */
    if (n > 0) {
        if (hdd_pio_read(&drive, first_sector, 1, temp))
            return -1;   
        mymemcpy(buf + buf_off, (char *) temp, n);
    }

    flip->f_pos += orig_size;
    return orig_size;
}

ssize_t hdd_write(struct file_s *flip, char *buf, size_t n)
{
    u32_t first_sector = ATA_TO_SECTOR(flip->f_pos);
    u32_t last_sector = ATA_TO_SECTOR((flip->f_pos + n + ATA_SECTOR_SIZE - 1));
    u32_t seccount;
    u32_t orig_size;
    u32_t buf_off, temp_off, sz;

    /* check limits */
    if (last_sector > drive.size_in_sectors) {
        debug_panic("hdd_write: trying to write beyond disk limit");
        last_sector = drive.size_in_sectors;
        n = drive.size_in_sectors * ATA_SECTOR_SIZE - flip->f_pos;
    }
    seccount = last_sector - first_sector;
    orig_size = n;

    /* check if it is easy */
    if (flip->f_pos == (first_sector << 9) && n % ATA_SECTOR_SIZE == 0) {
        if (hdd_pio_write(&drive, first_sector, seccount, buf))
            return -1;
        return n;
    }

    /* do it the hard way - first sector */
    buf_off = 0;
    temp_off = flip->f_pos % ATA_SECTOR_SIZE;
    sz = MIN(n, ATA_SECTOR_SIZE - temp_off);
    if (temp_off != 0 || sz < ATA_SECTOR_SIZE) {
        if (hdd_pio_read(&drive, first_sector, 1, temp))
            return -1;
        while (buf_off < sz)
            temp[temp_off++] = buf[buf_off++];
        if (hdd_pio_write(&drive, first_sector, 1, temp))
            return -1;
        first_sector++;
        n -= sz;
    }

    /* write the middle part */
    while (n >= ATA_SECTOR_SIZE) {
        if (hdd_pio_write(&drive, first_sector, 1, buf + buf_off))
            return -1;
        n -= ATA_SECTOR_SIZE;
        buf_off += ATA_SECTOR_SIZE;
        first_sector++;
    }

    /* write the (possibly) unaligned last sector */
    if (n > 0) {
        if (hdd_pio_read(&drive, first_sector, 1, temp))
            return -1;
        temp_off = 0;
        while (temp_off < n)
            temp[temp_off++] = buf[buf_off++];
        if (hdd_pio_write(&drive, first_sector, 1, temp))
            return -1;
    }

    flip->f_pos += orig_size;
    return orig_size;
}

int hdd_flush(struct file_s *flip)
{
    hdd_pio_write(&drive, 0, 0, 0);
    return 0;
}

static struct file_operations_s ops = {
    .read = hdd_read,
    .write = hdd_write,
    .lseek = hdd_lseek,
    .flush = hdd_flush
};

void hdd_init()
{
    /* prepare structs */
    controller.port = 0x1F0;
    controller.cport = 0x3F4;
    sem_init(&controller.sem_transfer, 1);
    drive.controller = &controller;
    drive.drive = ATA_MASTER;

    /* try to identify the device. */
    hdd_reset(&controller);
    hdd_identify(&drive);

    /* make char device in /dev */
    //fs_make_dev("hdd", I_BLOCK, DEV_HDD, 0);

    /* register device */
    dev_register(DEV_HDD, &ops);
}
