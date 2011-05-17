#ifndef _DEV_H
#define _DEV_H

#define DEV_STDIN  0
#define DEV_STDOUT 0
#define DEV_STDERR 0
#define DEV_FS     1

#include <minios/fs.h>
#include <sys/types.h>

/* Devices */
void dev_init(void);
int dev_file_calls(struct file_s *flip, dev_t dev);

#endif /* _DEV_H */
