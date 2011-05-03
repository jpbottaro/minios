#include <unistd.h> /* get constants for sys calls */
#include <fcntl.h>  /* get constants for sys calls */
#include <minios/dirent.h>
#include <minios/sched.h>
#include <minios/scall.h>
#include <minios/debug.h>
#include <minios/misc.h>
#include <minios/dev.h>
#include "fs.h"

char *fs_offset;
struct inode_s *root;

static int fs_readwrite(int fd, char *buf, unsigned int n, int flag);
static void fill_inode(struct inode_s *ino, int mode);

/* initialize fs, needs to be ALL mapped in memory, fs_start being first byte */
int fs_init(u32_t fs_start)
{
    struct inode_s *ino, *dev;
    ino_t ino_num;
    fs_offset = (char *) fs_start;
    read_super();
    root = (struct inode_s *) (fs_offset + INODE_OFFSET);

    /* make stdin, stdout, stderr */
    if ( (ino_num = find_inode(NULL, "/dev", FS_SEARCH_GET)) == NO_INODE)
        debug_panic("fs_init: no /dev folder!!");
    dev = get_inode(ino_num);

    ino_num = find_inode(dev, "stdin", FS_SEARCH_ADD);
    ino = get_inode(ino_num);
    fill_inode(ino, I_SPECIAL);
    ino->i_zone[0] = DEV_STDIN;
    ino_num = find_inode(dev, "stdout", FS_SEARCH_ADD);
    ino = get_inode(ino_num);
    fill_inode(ino, I_SPECIAL);
    ino->i_zone[0] = DEV_STDOUT;
    ino_num = find_inode(dev, "stderr", FS_SEARCH_ADD);
    ino = get_inode(ino_num);
    fill_inode(ino, I_SPECIAL);
    ino->i_zone[0] = DEV_STDERR;

    /* register sys calls */
    SCALL_REGISTER(3, sys_read);
    SCALL_REGISTER(4, sys_write);
    SCALL_REGISTER(5, sys_open);
    SCALL_REGISTER(6, sys_close);
    SCALL_REGISTER(10, sys_unlink);
    SCALL_REGISTER(12, sys_chdir);
    SCALL_REGISTER(19, sys_lseek);
    SCALL_REGISTER(38, sys_rename);
    SCALL_REGISTER(39, sys_mkdir);
    SCALL_REGISTER(40, sys_rmdir);
    SCALL_REGISTER(141, sys_getdents);

    return OK;
}

/* fill inode information */
static void fill_inode(struct inode_s *ino, int mode)
{
    int i;
    ino->i_mode   = (mode & I_TYPE) ? mode : mode | I_FILE;
    ino->i_nlinks = 1;
    ino->i_uid    = current_uid();
    ino->i_gid    = current_gid();
    ino->i_size   = 0;
    ino->i_atime  = 0;
    ino->i_mtime  = 0;
    ino->i_ctime  = 0;
    for(i = 0; i < NR_ZONES; i++) ino->i_zone[i] = 0;
    ino->i_zone[0] = empty_block();
}

/* open file and return fd; to make our life easier, always open RW */
int sys_open(const char *filename, int flags, int mode)
{
    int flag;
    ino_t ino_num;
    struct inode_s *ino, *dir;

    flag = (flags & O_CREAT) ? FS_SEARCH_CREAT : FS_SEARCH_GET;
    dir = get_inode(current_dir());

    if ( (ino_num = find_inode(dir, filename, flag)) == NO_INODE)
        return ERROR;

    ino = get_inode(ino_num);

    if (flags & O_CREAT)
        fill_inode(ino, mode);

    if (flags & O_TRUNC)
        ino->i_size = 0;

    return get_fd(ino_num, (flags & O_APPEND) ? ino->i_size : 0);
}

/* close file; since our fs resides in memory and we dont have to write changes
 * anywhere, we just release fd
 */
int sys_close(int fd)
{
    return release_fd(fd);
}

/* move the pointer of a file to a different position in it */
int sys_lseek(int fd, off_t offset, int whence)
{
    struct inode_s *ino;
    int pos;

    ino = get_inode(file_inode(fd));
    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = file_pos(fd) + offset;
            break;
        case SEEK_END:
            pos = ino->i_size + offset;
            break;
        default:
            return ERROR;
    }
    pos = pos > ino->i_size ? ino->i_size : pos < 0 ? 0 : pos;
    set_file_pos(fd, pos);
    return OK;
}

static int fs_readwrite(int fd, char *buf, unsigned int n, int flag)
{
    struct inode_s *ino = get_inode(file_inode(fd));
    unsigned int pos = file_pos(fd);

    if (ino == NULL)
        return -1;
 
    /* if its a directory, error */
    if (IS_DIR(ino->i_mode))
        return -1;
       
    /* if its a char special file, call the kernel */
    if (IS_CHAR(ino->i_mode))
        return dev_io(ino->i_zone[0], buf, n, flag);

    /* check limit of file in read operation */
    if (flag == FS_READ)
        n = MIN(n, ino->i_size - pos);

    /* read/write the buffer */
    if ( (pos = copy_file(buf, n, pos, ino, flag)) == ERROR)
        return ERROR;

    /* check how much did we read/write */
    n = pos - file_pos(fd);
    
    /* update inode size */
    if (pos > ino->i_size)
        ino->i_size = pos;

    /* set new filepos */
    set_file_pos(fd, pos);

    return n;
}

/* read/write (flag) to 'buf' 'n' bytes from position 'pos' of the file 'ino' */
int copy_file(char *buf, unsigned int n, unsigned int pos, struct inode_s *ino,
                                                                          int flag)
{
    unsigned int size, off;
    block_t blocknr;
    char *block;

    /* if performance is the objective, the first block could be separated so
     * that we dont have to do '%' every cicle and therefore remove 'off'
     * altogether; I prefer small n clean code in this project
     */
    while (n > 0) {
        if ( (blocknr = read_map(ino, pos, flag)) == NO_BLOCK)
            return ERROR;
        block = (char *) get_block(blocknr);

        off = pos % BLOCK_SIZE;
        size = MIN(n, BLOCK_SIZE - off);

        if (flag == FS_WRITE) mymemcpy(block + off, buf, size);
        else                  mymemcpy(buf, block + off, size);

        n -= size;
        pos += size;
        buf += size;
    }

    return pos;
}

/* read 'n' bytes from a file a put them on buf */
int sys_read(int fd, char *buf, unsigned int n)
{
    return fs_readwrite(fd, buf, n, FS_READ);
}

/* write 'n' bytes from 'buf' in the file referenced by 'fd' */
int sys_write(int fd, char *buf, unsigned int n)
{
    return fs_readwrite(fd, buf, n, FS_WRITE);
}

/* remove file/dir from fs */
int sys_unlink(const char *pathname)
{
    ino_t ino;
    struct inode_s *dir;

    dir = get_inode(current_dir());

    if ( (ino = find_inode(dir, pathname, FS_SEARCH_REMOVE)) == NO_INODE)
        return ERROR;

    rm_inode(ino);

    return OK;
}

int sys_chdir(const char *path)
{
    ino_t ino;

    if ( (ino = find_inode(get_inode(current_dir()), path, FS_SEARCH_GET))
                                                                    == NO_INODE)
        return ERROR;

    if (!IS_DIR(get_inode(ino)->i_mode))
        return ERROR;

    set_current_dir(ino);
    return OK;
}

void last_component(const char *path, char *last)
{
    int i;
    const char *a;

    a = path;
    while (*path != '\0') {
        if (*path == '/' && *(path+1) != '/')
            a = path + 1;
        path++;
    }

    for (i = 0; a[i] != '\0' && a[i] != '/' && i < MAX_NAME; i++)
        last[i] = a[i];
    last[i] = '\0';
}

/* rename oldpath to newpath */
int sys_rename(const char *oldpath, const char *newpath)
{
    ino_t ino_num, parent_num;
    struct inode_s *dir, *last_dir, *ino;
    struct dir_entry_s *dentry;
    char name[MAX_NAME];

    dir = get_inode(current_dir());
    /* get last directory of path */
    if ( (parent_num = find_inode(dir, newpath, FS_SEARCH_LASTDIR)) == NO_INODE)
        return ERROR;
    last_dir = get_inode(parent_num);

    /* get last component of path */
    last_component(newpath, name);

    /* new entry in the last directory of path (or old one if file exists) */
    if ( (dentry = search_inode(last_dir, name)) == NULL)
        if ( (dentry = empty_entry(last_dir)) == NULL)
            return ERROR;

    /* error if file is actually a dir */
    if (dentry->num != 0 && IS_DIR(get_inode(dentry->num)->i_mode))
        return ERROR;

    /* remove entry from the old directory */
    if ( (ino_num = find_inode(dir, oldpath, FS_SEARCH_REMOVE)) == NO_INODE)
        return ERROR;

    /* fill entry in new directory */
    dentry->num = ino_num;
    mystrncpy(dentry->name, name, MAX_NAME);
    last_dir->i_size += DIRENTRY_SIZE;

    /* check if oldpath was a dir, and update '..' in that case */
    ino = get_inode(ino_num);
    if (IS_DIR(ino->i_mode)) {
        dentry = search_inode(ino, "..");
        dentry->num = parent_num;
        last_dir->i_nlinks++;
    }

    return OK;
}

/* create new directory */
int sys_mkdir(const char *pathname, mode_t mode)
{
    ino_t parent_num, ino_num;
    struct inode_s *ino, *dir;
    struct dir_entry_s *dentry;
    char name[MAX_NAME];

    last_component(pathname, name);

    /* get inode numbers from parent and new dir */
    dir = get_inode(current_dir());
    if ( (parent_num = find_inode(dir, pathname, FS_SEARCH_LASTDIR)) == NO_INODE)
        return ERROR;
    dir = get_inode(parent_num);
    if ( (ino_num = find_inode(dir, name, FS_SEARCH_ADD)) == NO_INODE)
        return ERROR;
    ino = get_inode(ino_num);

    /* fill new dir inode */
    fill_inode(ino, mode);
    ino->i_mode = (ino->i_mode & ~I_TYPE) | I_DIRECTORY;

    /* add '.' */
    dentry = empty_entry(ino);
    dentry->num = ino_num;
    mystrncpy(dentry->name, ".", 2);
    ino->i_nlinks++;

    /* and '..' */
    dentry = empty_entry(ino);
    dentry->num = parent_num;
    mystrncpy(dentry->name, "..", 3);
    dir->i_nlinks++;

    /* update dir size */
    ino->i_size = DIRENTRY_SIZE * 2;

    return OK;
}

/* remove directory (only if it is empty) */
int sys_rmdir(const char *pathname)
{
    ino_t parent_num, ino_num;
    struct inode_s *ino, *dir;
    char name[MAX_NAME];

    last_component(pathname, name);

    /* get inode numbers from parent and new dir */
    dir = get_inode(current_dir());
    if ( (parent_num = find_inode(dir, pathname, FS_SEARCH_LASTDIR)) == NO_INODE)
        return ERROR;
    dir = get_inode(parent_num);
    if ( (ino_num = find_inode(dir, name, FS_SEARCH_GET)) == NO_INODE)
        return ERROR;
    ino = get_inode(ino_num);
    
    /* check if its a dir and it is empty */
    if (!IS_DIR(ino->i_mode) || search_inode(ino, NULL) != NULL)
        return ERROR;

    /* this cant give an error */
    find_inode(dir, name, FS_SEARCH_REMOVE);

    /* free blocks and inode */
    rm_inode(ino_num);

    return OK;
}

int sys_getdents(int fd, char *buf, size_t n)
{
    unsigned int i, pos;
    struct inode_s *dir, *ino;
    struct dir_entry_s *dentry;
    struct dirent *dent;

    dir = get_inode(file_inode(fd));

    if (!IS_DIR(dir->i_mode))
        return ERROR;

    i = 0;
    pos = file_pos(fd);
    while (n >= sizeof(struct dirent) + 1) {
        dentry = next_entry(dir, &pos);
        if (dentry == NULL)
            break;
        ino = get_inode(dentry->num);
        dent = (struct dirent *) (buf + i);
        dent->d_ino = dentry->num; 
        dent->d_off = pos - sizeof(struct dir_entry_s);
        dent->d_reclen = mystrncpy(dent->d_name, dentry->name, MAX_NAME) + 
                         1 +                      /* add the \0 */
                         sizeof(ino_t) +          /* add the d_ino */
                         sizeof(off_t) +          /* add the d_off */
                         sizeof(unsigned short) + /* add the d_reclen */
                         1;                       /* add the d_type */
        /* add d_type */
        *(buf + i + dent->d_reclen - 1) = (IS_DIR(ino->i_mode) ? DT_DIR :
                                          IS_FILE(ino->i_mode) ? DT_REG :
                                          IS_CHAR(ino->i_mode) ? DT_CHR :
                                                                 DT_UNK);
        i += dent->d_reclen;
        n -= dent->d_reclen;
    }
    set_file_pos(fd, pos);

    return i;
}

/* this one should close all open fds and write buffered changes etc. but,
 * as you probably know already ;), no real need for that
 */
int end_fs()
{
    return OK;
}
