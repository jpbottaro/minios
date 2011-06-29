#ifndef __HDD_H__
#define __HDD_H__

#include <minios/dev.h>
#include <sys/types.h>

#define HDD_CMD 0x1F0
#define HDD_CTL 0x3F6

#define REG_DATA	    0	/* data register (offset from the base reg.) */
#define REG_PRECOMP	    1	/* start of write precompensation */
#define REG_COUNT	    2	/* sectors to transfer */
#define REG_SECTOR	    3	/* sector number */
#define REG_CYL_LO	    4	/* low byte of cylinder number */
#define REG_CYL_HI	    5	/* high byte of cylinder number */
#define REG_LDH		    6	/* lba, drive and head */
#define   LDH_DEFAULT		0xA0	/* ECC enable, 512 bytes per sector */

/* Read only registers */
#define REG_STATUS	    7	/* status */
#define   STATUS_BSY		0x80	/* controller busy */
#define	  STATUS_RDY		0x40	/* drive ready */
#define	  STATUS_WF		0x20	/* write fault */
#define	  STATUS_SC		0x10	/* seek complete (obsolete) */
#define	  STATUS_DRQ		0x08	/* data transfer request */
#define	  STATUS_CRD		0x04	/* corrected data */
#define	  STATUS_IDX		0x02	/* index pulse */
#define	  STATUS_ERR		0x01	/* error */
#define	  STATUS_ADMBSY	       0x100	/* administratively busy (software) */
#define REG_ERROR	    1	/* error code */
#define	  ERROR_BB		0x80	/* bad block */
#define	  ERROR_ECC		0x40	/* bad ecc bytes */
#define	  ERROR_ID		0x10	/* id not found */
#define	  ERROR_AC		0x04	/* aborted command */
#define	  ERROR_TK		0x02	/* track zero error */
#define	  ERROR_DM		0x01	/* no data address mark */

/* Write only registers */
#define REG_COMMAND	    7	/* command */
#define   CMD_IDLE		0x00	/* for w_command: drive idle */
#define   CMD_RECALIBRATE	0x10	/* recalibrate drive */
#define   CMD_READ		0x20	/* read data */
#define   CMD_READ_EXT		0x24	/* read data (LBA48 addressed) */
#define   CMD_READ_DMA_EXT	0x25	/* read data using DMA (w/ LBA48) */
#define   CMD_WRITE		0x30	/* write data */
#define	  CMD_WRITE_EXT		0x34	/* write data (LBA48 addressed) */
#define   CMD_WRITE_DMA_EXT	0x35	/* write data using DMA (w/ LBA48) */
#define   CMD_READVERIFY	0x40	/* read verify */
#define   CMD_FORMAT		0x50	/* format track */
#define   CMD_SEEK		0x70	/* seek cylinder */
#define   CMD_DIAG		0x90	/* execute device diagnostics */
#define   CMD_SPECIFY		0x91	/* specify parameters */
#define   CMD_READ_DMA		0xC8	/* read data using DMA */
#define   CMD_WRITE_DMA		0xCA	/* write data using DMA */
#define   CMD_FLUSH_CACHE	0xE7	/* flush the write cache */
#define   ATA_IDENTIFY		0xEC	/* identify drive */
/* #define REG_CTL		0x206	*/ /* control register */
#define REG_CTL		0	/* control register */
#define   CTL_NORETRY		0x80	/* disable access retry */
#define   CTL_NOECC		0x40	/* disable ecc retry */
#define   CTL_EIGHTHEADS	0x08	/* more than eight heads */
#define   CTL_RESET		0x04	/* reset controller */
#define   CTL_INTDISABLE	0x02	/* disable interrupts */
#define REG_CTL_ALTSTAT 0	/* alternate status register */

#define LBA48_CHECK_SIZE	0x0f000000
#define LBA_MAX_SIZE		0x0fffffff	/* Highest sector size for reg LBA */

#define   CTL_EIGHTHEADS	0x08	/* more than eight heads */

#define SECTOR_SIZE 512

void hdd_init();

#endif
