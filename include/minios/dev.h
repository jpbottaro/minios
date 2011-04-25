#ifndef _DEV_H
#define _DEV_H

#define DEV_STDIN  0
#define DEV_STDOUT 1
#define DEV_STDERR 2

extern int dev_io(unsigned int dev, char *buf, int n, int flag);

#endif /* _DEV_H */
