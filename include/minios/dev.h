#ifndef _DEV_H
#define _DEV_H

#define MAX_DEVICES 20

#define DEV_TTY 0
#define DEV_FS  1

#include <minios/fs.h>
#include <sys/types.h>

/* Devices */
void dev_init(void);
int dev_register(unsigned int major, struct file_operations_s *fops);
int dev_file_calls(struct file_s *flip, dev_t dev);

struct dev_s {
    struct file_operations_s d_op;
};

extern struct dev_s devices[MAX_DEVICES];

#endif /* _DEV_H */
