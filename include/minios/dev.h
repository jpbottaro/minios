#ifndef _DEV_H
#define _DEV_H

#define MAX_DEVICES 20

#define DEV_TTY     0
#define DEV_FS      1
#define DEV_SERIAL  2
#define DEV_HDD     3
#define DEV_RAMDISK 4

#include <minios/fs.h>
#include <sys/types.h>

/* Devices */
void dev_init(void);
int dev_register(dev_t major, struct file_operations_s *fops);
struct file_operations_s *dev_operations(dev_t major);

struct dev_s {
    struct file_operations_s d_op;
};

extern struct dev_s devices[MAX_DEVICES];

#endif /* _DEV_H */
