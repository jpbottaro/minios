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

int hdd_wait_status(struct ata_controller *ata, int advanced_check)
{
    int i;

    /* waste 400ms */
    for (i = 0; i < 4; i++)
        inb(ata->port + ATA_REG_ALTSTATUS);

    /* wait for bsy to be cleared */
    while (inb(ata->port + ATA_REG_STATUS) & ATA_SR_BSY)
        ;

    if (advanced_check) {
        unsigned char state = inb(ata->port + ATA_REG_STATUS);

        if (state & ATA_SR_ERR)
            return -1;
        if (state & ATA_SR_DF)
            return -1;
        if ((state & ATA_SR_DRQ) == 0)
            return -1;
    }

    return 0;
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

void hdd_identify(struct ide_device *ide)
{
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

    if (hdd_wait_status(ata, 1)) {
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
                            u32_t seccount, void *buffer, int flag)
{
    struct ata_controller *ata = ide->controller;
    u8_t cmd = flag == ATA_READ ? ATA_CMD_READ_PIO : ATA_CMD_WRITE_PIO;
    u8_t flush = ATA_CMD_CACHE_FLUSH;

    if (ide == NULL || seccount == 0 || buffer == NULL)
        return 0;

    ATA_USE_TRANSFER(ata);
    if (ide->lba48) {
        outb(ata->port + ATA_REG_HDDEVSEL, 0x40 | (ide->drive << 4));
        outb(ata->port + ATA_REG_SECCOUNT, (u8_t) (seccount >> 8));
        outb(ata->port + ATA_REG_LBA_LOW, (u8_t) (lba >> 24));
        outb(ata->port + ATA_REG_LBA_MED, 0); /* LBA32 */
        outb(ata->port + ATA_REG_LBA_HIGH, 0); /* LBA32 */
        cmd = (flag == ATA_READ) ? ATA_CMD_READ_PIO_EXT : ATA_CMD_WRITE_PIO_EXT;
        flush = ATA_CMD_CACHE_FLUSH_EXT;
    } else {
        outb(ata->port + ATA_REG_HDDEVSEL, 0xE0 | (ide->drive << 4) |
                                                  ((lba >> 24) & 0x0F));
    }

    outb(ata->port + ATA_REG_SECCOUNT, (u8_t) seccount);
    outb(ata->port + ATA_REG_LBA_LOW, (u8_t) lba);
    outb(ata->port + ATA_REG_LBA_MED, (u8_t) (lba >> 8));
    outb(ata->port + ATA_REG_LBA_HIGH, (u8_t) (lba >> 16));
    outb(ata->port + ATA_REG_COMMAND, cmd);

    if (flag == ATA_READ) {
        while (seccount--) {
            if (hdd_wait_status(ata, 1))
                goto err;
            insw(ata->port + ATA_REG_DATA, buffer, ATA_SECTOR_SIZE / 2);
            buffer += ATA_SECTOR_SIZE;
        }
    } else {
        while (seccount--) {
            hdd_wait_status(ata, 0);
            outsw_delay(ata->port + ATA_REG_DATA, buffer, ATA_SECTOR_SIZE / 2);
            buffer += ATA_SECTOR_SIZE;
        }
        outb(ata->port + ATA_REG_COMMAND, flush);
        hdd_wait_status(ata, 0);
    }
    ATA_FREE_TRANSFER(ata);
    return 0;

err:
    ATA_FREE_TRANSFER(ata);
    return -1;
}

static int hdd_pio_read(struct ide_device *ide, u32_t lba, u32_t seccount,
                        void *buffer)
{
    return hdd_pio_transfer(ide, lba, seccount, buffer, ATA_READ);
}
static int hdd_pio_write(struct ide_device *ide, u32_t lba, u32_t seccount,
                         const void *buffer)
{
    return hdd_pio_transfer(ide, lba, seccount, (void *) buffer, ATA_WRITE);
}

size_t hdd_read(struct file_s *flip, char *buf, size_t n)
{
    u32_t current_sector = ATA_TO_SECTOR(flip->f_pos);
    u32_t last_sector = ATA_TO_SECTOR(flip->f_pos + n + ATA_SECTOR_SIZE - 1);
    u32_t remaining, seccount, orig_size, temp_off, buf_off, sz;

    /* check limits */
    if (last_sector > drive.size_in_sectors) {
        debug_panic("hdd_read: trying to read beyond disk limit");
        last_sector = drive.size_in_sectors;
        n = drive.size_in_sectors * ATA_SECTOR_SIZE - flip->f_pos;
    }
    seccount = last_sector - current_sector;
    orig_size = n;

    /* check if it is easy */
    if (flip->f_pos == (current_sector << 9) && n % ATA_SECTOR_SIZE == 0) {
        if (hdd_pio_read(&drive, current_sector, seccount, buf))
            return -1;
        flip->f_pos += n;

        return n;
    }

    /* read the unaligned first sector */
    temp_off = flip->f_pos % ATA_SECTOR_SIZE;
    remaining = ATA_SECTOR_SIZE - temp_off;
    sz = MIN(n, remaining);
    if (hdd_pio_read(&drive, current_sector, 1, temp))
        return -1;
    mymemcpy((char *) buf, (char *) (temp + temp_off), sz);

    /* read the middle part */
    current_sector++;
    n -= sz;
    buf_off = sz;
    while (n >= ATA_SECTOR_SIZE) {
        if (hdd_pio_read(&drive, current_sector, 1, buf + buf_off))
            return -1;
        n -= ATA_SECTOR_SIZE;
        buf_off += ATA_SECTOR_SIZE;
        current_sector++;
    }

    /* read the (possibly) unaligned last sector */
    if (n > 0) {
        if (hdd_pio_read(&drive, current_sector, 1, temp))
            return -1;
        mymemcpy((char *) (buf + buf_off), (char *) temp, n);
    }

    flip->f_pos += orig_size;

    return orig_size;
}

ssize_t hdd_write(struct file_s *flip, char *buf, size_t n)
{
    u32_t current_sector = ATA_TO_SECTOR(flip->f_pos);
    u32_t last_sector = ATA_TO_SECTOR((flip->f_pos + n + ATA_SECTOR_SIZE - 1));
    u32_t remaining, seccount, orig_size, buf_off, temp_off;

    /* check limits */
    if (last_sector > drive.size_in_sectors) {
        debug_panic("hdd_write: trying to write beyond disk limit");
        last_sector = drive.size_in_sectors;
        n = drive.size_in_sectors * ATA_SECTOR_SIZE - flip->f_pos;
    }
    orig_size = n;

    /* check if it is easy */
    if (flip->f_pos == (current_sector << 9) && n % ATA_SECTOR_SIZE == 0) {
        if (hdd_pio_write(&drive, current_sector, last_sector - current_sector, buf))
            return -1;
        flip->f_pos += n;

        return n;
    }

    /* first sector */
    buf_off = 0;
    temp_off = flip->f_pos % ATA_SECTOR_SIZE;
    if (temp_off != 0) {
        remaining = ATA_SECTOR_SIZE - temp_off;
        u32_t sz = MIN(n, remaining);
        if (hdd_pio_read(&drive, current_sector, 1, temp))
            return -1;
        mymemcpy((char *) (temp + temp_off), buf, sz);
        buf_off = sz;
        if (hdd_pio_write(&drive, current_sector, 1, temp))
            return -1;
        current_sector++;
        n -= sz;
    }

    /* write the middle part */
    if (n >= ATA_SECTOR_SIZE) {
        seccount = n / ATA_SECTOR_SIZE;
        if (hdd_pio_write(&drive, current_sector, seccount, buf + buf_off))
            return -1;
        n %= ATA_SECTOR_SIZE;
        buf_off += seccount * ATA_SECTOR_SIZE;
        current_sector += seccount;
    }

    /* write the (possibly) unaligned last sector */
    if (n > 0) {
        if (hdd_pio_read(&drive, current_sector, 1, temp))
            return -1;
        mymemcpy((char *) temp, buf + buf_off, n);
        if (hdd_pio_write(&drive, current_sector, 1, temp))
            return -1;
    }

    flip->f_pos += orig_size;
    return orig_size;
}

int hdd_flush(struct file_s *flip)
{
    u8_t flush = ATA_CMD_CACHE_FLUSH;
    int ret;

    ATA_USE_TRANSFER(drive.controller);
    if (drive.lba48)
        flush = ATA_CMD_CACHE_FLUSH_EXT;
    outb(drive.controller->port + ATA_REG_COMMAND, flush);
    ret = hdd_wait_status(drive.controller, 1);
    ATA_FREE_TRANSFER(drive.controller);

    return ret;
}

static struct file_operations_s ops = {
    .read = hdd_read,
    .write = hdd_write,
    .lseek = hdd_lseek,
    .flush = hdd_flush
};

int hdd_test(struct ide_device *ide)
{
    if (hdd_pio_read(&drive, 0, 1, temp))
        debug_panic("hdd_test: error in read");
    /* this is kinda risky... but what the heck, carpe diem */
    if (hdd_pio_write(&drive, 0, 1, temp))
        debug_panic("hdd_test: error in write");
    return 0;
}

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

    /* test the ata device, check if it actually works of if its a lier */
    hdd_test(&drive);

    /* register device */
    dev_register(DEV_HDD, &ops);
}
