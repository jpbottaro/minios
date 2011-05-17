#include <minios/debug.h>
#include <minios/i386.h>
#include <minios/sem.h>
#include <sys/types.h>
#include "fdd.h"

#define FDD_PORT 0x3F0

/* Intel 82077A ports: */
#define PORT_SRA  0x00  /* R   - Status Register A */
#define PORT_SRB  0x01  /* R   - Status Register B */
#define PORT_DOR  0x02  /* R/W - Digital Output Register */
#define PORT_TDR  0x03  /* R/W - Tape Drive Register */
#define PORT_MSR  0x04  /* R   - Main Status Register */
#define PORT_DSR  0x04  /*   W - Data-rate Select Register (ojo al piojo que es el mismo que el MSR) */
#define PORT_DATA 0x05  /* R/W - "Data", FIFO... esto es como tirás un "file miarchivo" y te dice: "miarchivo: data". */
#define PORT_RSVD 0x06  /* -/- - Este está porque 7 no es potencia de 2. */
#define PORT_DIR  0x07  /* R   - Digital Input Register */
#define PORT_CCR  0x07  /*   W - Configuration Control Register (es el mismo que el DIR) */

/* MSR bits: Main Status Register */
#define FDD_MSR_ACTA 0x01
#define FDD_MSR_ACTB 0x02
#define FDD_MSR_ACTC 0x04
#define FDD_MSR_ACTD 0x08
#define FDD_MSR_BUSY 0x10
#define FDD_MSR_NDMA 0x20
#define FDD_MSR_DIO  0x40
#define FDD_MSR_MRQ  0x80

/* DOR bits: Digital Output Register */
#define FDD_DOR_DR0  0x01
#define FDD_DOR_DR1  0x02
#define FDD_DOR_nREST 0x04
#define FDD_DOR_DMA  0x08
#define FDD_DOR_MOTA 0x10
#define FDD_DOR_MOTB 0x20
#define FDD_DOR_MOTC 0x40
#define FDD_DOR_MOTD 0x80

/* Available Commands (5 bits) */
#define FDD_CMD_READ_TRACK     0x02
#define FDD_CMD_SPECIFY        0x03
#define FDD_CMD_SENSE_DRV      0x04
#define FDD_CMD_WRITE          0x05
#define FDD_CMD_READ           0x06
#define FDD_CMD_RECALIBRATE    0x07
#define FDD_CMD_SENSE_INT      0x08
#define FDD_CMD_WRITE_DEL_DATA 0x09
#define FDD_CMD_READ_DEL_DATA  0x0C
#define FDD_CMD_READ_ID        0x0A
#define FDD_CMD_FORMAT_TRACK   0x0D
#define FDD_CMD_DUMPREG        0x0E
#define FDD_CMD_SEEK           0x0F
#define FDD_CMD_VERSION        0x10
#define FDD_CMD_SCAN_EQUAL     0x11
#define FDD_CMD_PERPENDICULAR_MODE 0x12
#define FDD_CMD_CONFIGURE      0x13
#define FDD_CMD_VERIFY         0x16
#define FDD_CMD_SCAN_LOW_OR_EQUAL 0x19
#define FDD_CMD_SCAN_HIGH_OR_EQUAL 0x1D

#define FDD_CMD_MT   0x80
#define FDD_CMD_MFM  0x40
#define FDD_CMD_SKIP 0x20

/* DMA options */
#define DMA_READ        0x46
#define DMA_WRITE       0x4A
#define DMA_RESET_VAL   0x06

#define DMAIO_ADDR       0x004
#define DMAIO_TOP        0x081
#define DMAIO_COUNT      0x005
#define DMAIO_FLIPFLOP   0x00C
#define DMAIO_MODE       0x00B
#define DMAIO_INIT       0x00A

/** Init **/

void fdd_init(void)
{
}
