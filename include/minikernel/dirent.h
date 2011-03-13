#ifndef _DIRENT_H
#define _DIRENT_H

/* taken from minix */

#include <sys/types.h>

#define DT_REG  1
#define DT_DIR  2
#define DT_FIFO 3
#define DT_SOCK 4
#define DT_LNK  5
#define DT_BLK  6
#define DT_CHR  7
#define DT_UNK  8

struct dirent {
    ino_t d_ino;             /* I-node number */
    off_t d_off;             /* Offset in directory */
    unsigned short d_reclen; /* Length of this record */
    char d_name[30];         /* Null terminated name */
};

int getdents(int fd, char *buf, size_t n);

#endif /* _DIRENT_H */
