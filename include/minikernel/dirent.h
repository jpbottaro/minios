#ifndef _DIRENT_H
#define _DIRENT_H

/* taken from minix */

#include <sys/types.h>

struct dirent {
    ino_t   d_ino;           /* I-node number */
    off_t   d_off;           /* Offset in directory */
    unsigned short d_reclen; /* Length of this record */
    char    d_name[1];       /* Null terminated name */
};

int getdents(int fd, struct dirent *buf, size_t n);

#endif /* _DIRENT_H */
