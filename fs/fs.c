#include "fs.h"
#include <unistd.h> /* get constants for sys calls */
#include <fcntl.h>  /* get constants for sys calls */
#include <minikernel/misc.h>

struct inode_s *root;
char *fs_offset;

ino_t creat_file(const char *filename, struct inode_s *par);
const char *parse_filename(const char *filename);
int get_fd(ino_t ino_num, unsigned int pos);
int release_fd(int fd);

/* initialize fs, needs to be ALL mapped in memory, fs_start being first byte */
int fs_init(char *fs_start)
{
    fs_offset = fs_start;
    read_super();
    root = (struct inode_s *) (fs_offset + INODE_OFFSET);
    return 0;
}

const char *parse_filename(const char *filename)
{
    const char *begin, *end;

    begin = end = filename;
    while (*end != '\0') {
        if (*end == '/')
            begin = end + 1;
        end++;
    }

    return begin;
}

/* create file named after the last component of 'filename' in directory 'par' */
ino_t creat_file(const char *filename, struct inode_s *par)
{
    int i;
    /* get an empty inode */
    int ino_num = empty_inode();
    struct inode_s *ino = get_inode(ino_num);
    /* get an empty directory entry to save our file */
    struct dir_entry_s *dentry = empty_dir_entry(par);
    /* get last component of the filename */
    const char *name = parse_filename(filename);

    /* fill dir entry */
    dentry->num = ino_num;
    mystrncpy(name, dentry->name, MAX_NAME);

    /* fill new inode */
    ino->i_mode   = 0; /* XXX MODIFICARRRRRR */
    ino->i_nlinks = 1;
    ino->i_uid    = 0;
    ino->i_gid    = 0;
    ino->i_size   = 0;
    ino->i_atime  = 0;
    ino->i_mtime  = 0; /* Ver si vale la pena agregar tiempo */
    ino->i_ctime  = 0;
    for(i = 0; i < NR_ZONES; i++) ino->i_zone[i] = 0;

    return ino_num;
}

/* open file and return fd; to make our life easier, always open RW */
int sys_open(const char *filename, int flags, int mode)
{
    struct inode_s *ino, *par;
    ino_t ino_num;
    
    switch (find_inode(get_inode(current_dir()), filename, &ino, &par)) {
        case ERR_BADPATH:
            return ERROR;
        case ERR_NOTEXIST:
            if (!(flags & O_CREAT))
                return ERROR;
            ino_num = creat_file(filename, par);
        default:
            break;
    }

    if (flags & O_TRUNC)
        ino->i_size = 0;

    return get_fd(ino_num, (flags & O_APPEND) ? ino->i_size : 0);
}

/* close file; since our fs resides in memory and we dont have to write changes
 * anywhere, we just release fd */
int sys_close(unsigned int fd)
{
    return release_fd(fd);
}

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

/* read 'n' bytes from a file a put them on buf */
int sys_read(unsigned int fd, char *buf, unsigned int n)
{
    struct inode_s *ino = get_inode(file_inode(fd));
    unsigned int pos = file_pos(fd);
    block_t blocknr;
    char *block;

    if (pos % BLOCK_SIZE != 0) {
        unsigned int off, size;

        if ( (blocknr = read_map(ino, pos)) == NO_BLOCK)
            return ERROR;
        block = get_block(blocknr);
        off = pos % BLOCK_SIZE;
        size = MIN(BLOCK_SIZE - off, n);
        mymemcpy(block + off, buf, size);
        n -= size;
        pos += size;
        buf += size;
    }

    for (; n > BLOCK_SIZE; n -= BLOCK_SIZE, pos += BLOCK_SIZE, buf += BLOCK_SIZE) {
        if ( (blocknr = read_map(ino, pos)) == NO_BLOCK)
            return ERROR;
        block = get_block(blocknr);
        mymemcpy(block, buf, BLOCK_SIZE);
    }

    if (n != 0) {
        if ( (blocknr = read_map(ino, pos)) == NO_BLOCK)
            return ERROR;
        block = get_block(blocknr);
        mymemcpy(block, buf, n);
    }

    return 0;
}

int sys_write(unsigned int fd, char *buf, unsigned int n)
{
    /* XXX HACERRR */
    return 0;
}

/* this one should close all open fds and write changes etc. but, as you
 * probably know already ;), no real need for that */
int fs_end()
{
    return 0;
}

int get_fd(ino_t ino_num, unsigned int pos)
{
    /* XXX HACER */
    return -1;
}

int release_fd(int fd)
{
    /* XXX HACER */
    return -1;
}
