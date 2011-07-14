/* This driver is _heavily_ based in a version made by Ezequiel Aguerre,
 * and is intended for public use.
 */

#ifndef __HDD_H__
#define __HDD_H__

#include <minios/dev.h>
#include <sys/types.h>

/* Offsets de los puertos */
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT   0x02
#define ATA_REG_LBA_LOW    0x03
#define ATA_REG_LBA_MED    0x04
#define ATA_REG_LBA_HIGH   0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_LBA3       0x03
#define ATA_REG_LBA4       0x04
#define ATA_REG_LBA5       0x05

#define ATA_REG_CONTROL    0x02
#define ATA_REG_ALTSTATUS  0x02
#define ATA_REG_DEVADDRESS 0x0D

#define ATA_REG_BMICOM     0x00
#define ATA_REG_BMISTA     0x02
#define ATA_REG_BMIDTP     0x04

/* STATUS */
#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

/* FEATURES/ERROR PORT */
#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

/* COMMAND PORT */
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define ATAPI_CMD_READ            0xA8
#define ATAPI_CMD_EJECT           0x1B

/* IDENTIFICATION SPACE */
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

/* CONTROL PORT */
#define ATA_CTRL_NIEN  0x02
#define ATA_CTRL_SRST  0x04
#define ATA_CTRL_HOB   0x08


/* Interfaces */
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

/* Discos */
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

/* Canales */
#define ATA_PRIMARY    0x00
#define ATA_SECONDARY  0x01

/* Direcciones */
#define ATA_READ       0x00
#define ATA_WRITE      0x01

#define ATA_USE_BUS(x) sem_wait(&((x)->sem_op))
#define ATA_FREE_BUS(x) sem_signal(&((x)->sem_op))
#define ATA_WAIT_IRQ(x) sem_wait(&((x)->sem_irq))

static inline void iowait()
{
    inb(0x1F0 + ATA_REG_STATUS);
}

#define ATA_DELAY() do { iowait();iowait();iowait();iowait();iowait(); } while(0)

void hdd_init();

#endif
