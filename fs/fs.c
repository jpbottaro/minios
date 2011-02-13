#include "fs.h"
#include <unistd.h> /* get constants for sys calls */
#include <fcntl.h>  /* get constants for sys calls */
#include <minikernel/misc.h>
#include <minikernel/sched.h> /* get uid and gid of process */

#define FS_READ  0
#define FS_WRITE 1

char *fs_offset;
struct inode_s *root;

static int fs_readwrite(unsigned int fd, char *buf, unsigned int n, int flag);
static void fill_inode(struct inode_s *ino, int mode);

/* initialize fs, needs to be ALL mapped in memory, fs_start being first byte */
int fs_init(char *fs_start)
{
    fs_offset = fs_start;
    root = (struct inode_s *) (fs_offset + INODE_OFFSET);
    read_super();
    return 0;
}

/* fill inode information */
/* FIXME MODIFICARRRRRR */
static void fill_inode(struct inode_s *ino, int mode)
{
    int i;
    ino->i_mode   = mode;
    ino->i_nlinks = 1;
    ino->i_uid    = current_uid();
    ino->i_gid    = current_gid();
    ino->i_size   = 0;
    ino->i_atime  = 0;
    ino->i_mtime  = 0;
    ino->i_ctime  = 0;
    for(i = 0; i < NR_ZONES; i++) ino->i_zone[i] = 0;
}

/* open file and return fd; to make our life easier, always open RW */
int sys_open(const char *filename, int flags, int mode)
{
    int flag;
    ino_t ino_num;
    struct inode_s *ino, *dir;

    flag = (flags & O_CREAT) ? FS_SEARCH_ADD : FS_SEARCH_GET;
    dir = get_inode(current_dir());

    if ( (ino_num = find_inode(dir, filename, flag)) == NO_INODE)
        return ERROR;

    ino = get_inode(ino_num);

    if (flag & O_CREAT)
        fill_inode(ino, mode);

    if (flags & O_TRUNC)
        ino->i_size = 0;

    return get_fd(ino_num, (flags & O_APPEND) ? ino->i_size : 0);
}

/* close file; since our fs resides in memory and we dont have to write changes
 * anywhere, we just release fd
 */
int sys_close(unsigned int fd)
{
    return release_fd(fd);
}

/* move the pointer of a file to a different position in it */
int sys_lseek(unsigned int fd, off_t offset, int whence)
{
    struct inode_s *ino;
    switch (whence) {
        case SEEK_SET:
            set_file_pos(fd, offset);
            break;
        case SEEK_CUR:
            set_file_pos(fd, file_pos(fd) + offset);
            break;
        case SEEK_END:
            ino = get_inode(file_inode(fd));
            set_file_pos(fd, ino->i_size + offset);
            break;
        default:
            return ERROR;
    }
    return 0;
}

static int fs_readwrite(unsigned int fd, char *buf, unsigned int n, int flag)
{
    struct inode_s *ino = get_inode(file_inode(fd));
    unsigned int pos = file_pos(fd);
    unsigned int size, off;
    block_t blocknr;
    char *block;
        
    /* check limit of file in read operation */
    if (flag == FS_READ)
        n = MIN(n, ino->i_size - pos);

    /* if performance is the objective, the first block could be separated so
     * that we dont have to do '%' every cicle and therefore remove 'off'
     * altogether; I prefer small n clean code in these project
     */
    while (n > 0) {
        if ( (blocknr = read_map(ino, pos)) == NO_BLOCK)
            return ERROR;
        block = (char *) get_block(blocknr);

        off = pos % BLOCK_SIZE;
        size = MIN(n, BLOCK_SIZE - off);

        if (flag == FS_READ)
            mymemcpy(block + off, buf, size);
        else
            mymemcpy(buf, block + off, size);

        n -= size;
        pos += size;
        buf += size;
    }

    /* check how much did we read/write */
    n = pos - file_pos(fd);

    /* set new filepos */
    set_file_pos(fd, pos);

    return n;
}

/* read 'n' bytes from a file a put them on buf */
int sys_read(unsigned int fd, char *buf, unsigned int n)
{
    return fs_readwrite(fd, buf, n, FS_READ);
}

/* write 'n' bytes from 'buf' in the file referenced by 'fd' */
int sys_write(unsigned int fd, char *buf, unsigned int n)
{
    return fs_readwrite(fd, buf, n, FS_WRITE);
}

/* this one should close all open fds and write buffered changes etc. but,
 * as you probably know already ;), no real need for that
 */
int fs_end()
{
    return 0;
}
