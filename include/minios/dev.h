#ifndef _DEV_H
#define _DEV_H

#define DEV_STDIN  0
#define DEV_STDOUT 1
#define DEV_STDERR 2

extern int dev_io(unsigned int dev, char *buf, int n, int flag);

#include <sys/types.h>

/* Devices */
typedef struct str_dev device;
typedef uint_32(dev_flush_t)(device* this);

struct str_dev {
	uint_32 clase;
	uint_32 refcount;
	dev_flush_t* flush;
} __attribute__((packed));

/* Char devices */

typedef struct str_chardev chardev;

typedef sint_32(chardev_read_t)(chardev* this, void* buf, uint_32 size);
typedef sint_32(chardev_write_t)(chardev* this, const void* buf, uint_32 size);
typedef sint_32(chardev_seek_t)(chardev* this, uint_32 pos);
typedef uint_32(chardev_flush_t)(chardev* this);

struct str_chardev {
	uint_32 clase;
	uint_32 refcount;
	chardev_flush_t* flush;
	chardev_read_t* read;
	chardev_write_t* write;
	chardev_seek_t* seek;
} __attribute__((packed));


/* Block devices */
typedef struct str_blockdev blockdev;

typedef sint_32(blockdev_read_t)(blockdev* this, uint_32 pos, void* buf, uint_32 size);
typedef sint_32(blockdev_write_t)(blockdev* this, uint_32 pos, const void* buf, uint_32 size);
typedef uint_32(blockdev_flush_t)(blockdev* this);

struct str_blockdev {
	uint_32 clase;
	uint_32 refcount;
	blockdev_flush_t* flush;
	blockdev_read_t* read;
	blockdev_write_t* write;
	uint_32 size;
} __attribute__((packed));

void device_init(void);

int device_descriptor(chardev* dev);


#endif /* _DEV_H */
