#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "misc.h"

#define BUF_SIZE 1024
#define DIRNOEXISTS "The directory does not exists\n"
#define GETDENTSERR "Problem with getdents\n"

/* inspired in the code example from the man pages of getdents(2) */
void main(int argc, const char *argv[])
{
    int fd, bpos, nread, len;
    char d_type;
    struct dirent *d;
    char buf[BUF_SIZE];

    if ( (fd = open((argc > 1) ? argv[1] : ".", O_RDONLY, 0)) < 0) {
        write(STDOUT_FILENO, DIRNOEXISTS, sizeof(DIRNOEXISTS) - 1);
        _exit(-1);
    }

    for (;;) {
        nread = getdents(fd, buf, BUF_SIZE);
        if (nread == -1) {
            write(STDOUT_FILENO, GETDENTSERR, sizeof(GETDENTSERR) - 1);
            _exit(-1);
        }

        if (nread == 0)
            break;

        for (bpos = 0; bpos < nread;) {
            d = (struct dirent *) (buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1);
            write(STDOUT_FILENO, (d_type == DT_REG) ?  "- " :
                                 (d_type == DT_DIR) ?  "d " :
                                 (d_type == DT_CHR) ?  "c " : "? ", 2);
            len = mystrlen((char *) d->d_name);
            write(STDOUT_FILENO, (char *) d->d_name, len);
            write(STDOUT_FILENO, "\n", 1);
            bpos += d->d_reclen;
        }
    }

    close(fd);
    _exit(0);
}
