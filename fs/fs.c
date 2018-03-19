#include <unistd.h> /* get constants for sys calls */
#include <fcntl.h>  /* get constants for sys calls */
#include <minios/dirent.h>
#include <minios/sched.h>
#include <minios/scall.h>
#include <minios/debug.h>
#include <minios/misc.h>
#include <minios/dev.h>
#include "fs.h"

struct file_s fs_dev_str;
struct file_s *fs_dev = &fs_dev_str;

static int fs_readwrite(struct file_s *flip, char *buf, unsigned int n, int flag);
static void fill_inode(struct inode_s *ino, int mode);

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
    ino->i_dirty  = 1;
    for(i = 0; i < NR_ZONES; i++) ino->i_zone[i] = 0;
}

/* open file and return fd; to make our life easier, always open RW */
int fs_open(const char *filename, int flags, int mode)
{
    int flag, fd;
    struct inode_s *ino, *dir;

    ino = dir = NULL;

    flag = (flags & O_CREAT) ? FS_SEARCH_CREAT : FS_SEARCH_GET;

    dir = current_dir();

    if ( (ino = find_inode(dir, filename, flag)) == NULL)
        goto err;

    release_inode(dir);

    if (ino->i_zone[0] == 0 && (flags & O_CREAT))
        fill_inode(ino, mode);

    if (flags & O_TRUNC) {
        ino->i_size = 0;
        ino->i_dirty = 1;
    }

    if ( (fd = get_fd(ino, (flags & O_APPEND) ? ino->i_size : 0)) == ERROR)
        goto err;

    return fd;

err:
    release_inode(dir);
    release_inode(ino);

    return ERROR;
}

/* generic close */
int fs_close(int fd)
{
    int r = 0;
    struct file_s *flip = get_file(fd);

    if (flip == NULL)
        return ERROR;

    if (flip->f_op == NULL)
        return ERROR;

    if (flip->f_op->close != NULL)
        r = flip->f_op->close(flip);

    release_fd(fd);

    return r;
}

int fs_closeall()
{
    int i;

    for (i = 0; i < MAX_FILES; i++)
        fs_close(i);

    return OK;
}

/* move the pointer of a file to a different position in it */
size_t fs_lseek(struct file_s *flip, off_t offset, int whence)
{
    struct inode_s *ino;
    int pos;

    ino = flip->f_ino;
    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = flip->f_pos + offset;
            break;
        case SEEK_END:
            pos = ino->i_size + offset;
            break;
        default:
            return ERROR;
    }
    pos = pos > ino->i_size ? ino->i_size : pos < 0 ? 0 : pos;
    flip->f_pos = pos;

    return OK;
}

/* generic lseek */
size_t sys_lseek(int fd, off_t offset, int whence)
{
    struct file_s *flip = get_file(fd);

    if (flip == NULL)
        return ERROR;

    if (flip->f_op == NULL)
        debug_panic("sys_read: f_op is NULL");

    if (flip->f_op->lseek == NULL)
        debug_panic("sys_lseek: lseek func pointer is NULL");

    return flip->f_op->lseek(flip, offset, whence);
}

/* read/write (flag) to 'buf' 'n' bytes from position 'pos' of the file 'ino' */
int copy_file(char *buf, unsigned int n, unsigned int pos, struct inode_s *ino,
                                                                          int flag)
{
    unsigned int size, off;
    block_t blocknr;
    struct buf_s *block;

    while (n > 0) {
        if ( (blocknr = read_map(ino, pos, flag)) == NO_BLOCK)
            return ERROR;
        block = get_block(blocknr);

        off = pos % BLOCK_SIZE;
        size = MIN(n, BLOCK_SIZE - off);

        if (flag == FS_WRITE) {
            mymemcpy(block->b_buffer + off, buf, size);
            block->b_dirty = 1;
        } else {
            mymemcpy(buf, block->b_buffer + off, size);
        }

        n -= size;
        pos += size;
        buf += size;
        release_block(block);
    }

    return pos;
}

static int fs_readwrite(struct file_s *flip, char *buf, unsigned int n, int flag)
{
    struct inode_s *ino = flip->f_ino;
    unsigned int pos = flip->f_pos;

    if (ino == NULL)
        return ERROR;
 
    /* if its a directory, error */
    if (IS_DIR(ino->i_mode))
        return ERROR;
       
    /* check limit of file in read operation */
    if (flag == FS_READ)
        n = MIN(n, ino->i_size - pos);

    /* read/write the buffer */
    if ( (pos = copy_file(buf, n, pos, ino, flag)) == ERROR)
        return ERROR;

    /* check how much did we read/write */
    n = pos - flip->f_pos;
    
    /* update inode size */
    if (pos > ino->i_size && flag == FS_WRITE) {
        ino->i_size = pos;
        ino->i_dirty = 1;
    }

    /* set new filepos */
    flip->f_pos = pos;

    return n;
}

/* read 'n' bytes from a file a put them on buf */
size_t fs_read(struct file_s *flip, char *buf, size_t n)
{
    return fs_readwrite(flip, buf, n, FS_READ);
}

/* write 'n' bytes from 'buf' in the file referenced by 'fd' */
ssize_t fs_write(struct file_s *flip, char *buf, size_t n)
{
    return fs_readwrite(flip, buf, n, FS_WRITE);
}

/* generic read for an fd */
size_t sys_read(int fd, char *buf, size_t n)
{
    struct file_s *flip = get_file(fd);

    if (flip == NULL)
        return ERROR;

    if (flip->f_op == NULL)
        debug_panic("sys_read: f_op is NULL");

    if (flip->f_op->read == NULL)
        debug_panic("sys_read: read func pointer is NULL");

    return flip->f_op->read(flip, buf, n);
}

/* generic write for an fd */
ssize_t sys_write(int fd, char *buf, size_t n)
{
    struct file_s *flip = get_file(fd);

    if (flip == NULL)
        return ERROR;

    if (flip->f_op == NULL)
        debug_panic("sys_read: f_op is NULL");

    if (flip->f_op->write == NULL)
        debug_panic("sys_read: write func pointer is NULL");

    return flip->f_op->write(flip, buf, n);
}

/* remove file/dir from fs */
int fs_unlink(const char *pathname)
{
    struct inode_s *ino, *dir;

    ino = dir = NULL;

    dir = current_dir();
    if ( (ino = find_inode(dir, pathname, FS_SEARCH_REMOVE)) == NULL)
        goto err;
    rm_inode(ino->i_num);
    release_inode(dir);
    release_inode(ino);

    return OK;

err:
    release_inode(dir);
    release_inode(ino);
    return ERROR;
}

int fs_chdir(const char *path)
{
    struct inode_s *ino, *dir;

    ino = dir = NULL;

    dir = current_dir();
    if ( (ino = find_inode(dir, path, FS_SEARCH_GET)) == NULL)
        goto err;
    release_inode(dir);

    if (!IS_DIR(ino->i_mode))
        goto err;

    set_current_dir(ino);
    release_inode(ino);

    return OK;

err:
    release_inode(dir);
    release_inode(ino);
    return ERROR;
}

void process_path(const char *pathname, char *path, char *name)
{
    int i, last;

    i = last = 0;
    while (pathname[i] != '\0') {
        if (pathname[i] == '/' && pathname[i+1] != '/')
            last = i + 1;
        path[i] = pathname[i];
        i++;
    }
    path[last] = '\0';

    for (i = 0; pathname[last] != '\0' && pathname[last] != '/' && i < MAX_NAME - 1; i++, last++)
        name[i] = pathname[last];
    name[i] = '\0';
}

/* rename oldpath to newpath */
int fs_rename(const char *oldpath, const char *newpath)
{
    struct inode_s *dir, *last_dir, *ino;
    char path[MAX_PATH], name[MAX_NAME];

    dir = last_dir = ino = NULL;

    /* get last component of path */
    process_path(newpath, path, name);
    if (name[0] == '\0') {
        char tmp[MAX_PATH];
        process_path(oldpath, tmp, name);
    }

    dir = current_dir();
    /* get last directory of target path */
    if ( (last_dir = find_inode(dir, path, FS_SEARCH_GET)) == NULL)
        goto err;

    /* remove entry from the old directory */
    if ( (ino = find_inode(dir, oldpath, FS_SEARCH_REMOVE)) == NULL)
        goto err;

    /* new entry in the last directory of path (or old one if file exists) */
    if (add_entry(last_dir, ino->i_num, name) == ERROR)
        goto err;

    /* check if oldpath was a dir, and update '..' in that case */
    if (IS_DIR(ino->i_mode)) {
        add_entry(ino, last_dir->i_num, "..");
        last_dir->i_nlinks++;
    }

    release_inode(last_dir);
    release_inode(dir);
    release_inode(ino);

    return OK;

err:
    release_inode(last_dir);
    release_inode(dir);
    release_inode(ino);

    return ERROR;
}

/* create new directory */
int fs_mkdir(const char *pathname, mode_t mode)
{
    struct inode_s *ino, *dir, *tmpdir;
    char path[MAX_PATH], name[MAX_NAME];

    ino = dir = tmpdir = NULL;

    process_path(pathname, path, name);
    tmpdir = current_dir();
    /* get inode numbers from parent and new dir */
    if ( (dir = find_inode(tmpdir, path, FS_SEARCH_GET)) == NULL)
        goto err;
    release_inode(tmpdir);
    if ( (ino = find_inode(dir, name, FS_SEARCH_ADD)) == NULL)
        goto err;

    /* fill new dir inode */
    fill_inode(ino, mode);
    ino->i_mode = (ino->i_mode & ~I_TYPE) | I_DIRECTORY;

    /* add '.' */
    empty_entry(ino, ino->i_num, ".");
    ino->i_nlinks++;

    /* and '..' */
    empty_entry(ino, dir->i_num, "..");
    dir->i_nlinks++;
    dir->i_dirty = 1;

    /* update dir size */
    ino->i_size = DIRENTRY_SIZE * 2;

    release_inode(dir);
    release_inode(ino);

    return OK;

err:
    release_inode(tmpdir);
    release_inode(dir);
    release_inode(ino);

    return ERROR;
}

/* remove directory (only if it is empty) */
int fs_rmdir(const char *pathname)
{
    struct inode_s *ino, *dir, *tmpdir;
    char path[MAX_PATH], name[MAX_NAME];

    ino = dir = tmpdir = NULL;

    process_path(pathname, path, name);
    tmpdir = current_dir();
    /* get inode numbers from parent and new dir */
    if ( (dir = find_inode(tmpdir, path, FS_SEARCH_GET)) == NULL)
        goto err;
    release_inode(tmpdir);
    if ( (ino = find_inode(dir, name, FS_SEARCH_ADD)) == NULL)
        goto err;

    /* check if its a dir and it is empty */
    if (!IS_DIR(ino->i_mode) || ino->i_size != DIRENTRY_SIZE * 2)
        goto err;

    /* this cant give an error */
    release_inode(find_inode(dir, name, FS_SEARCH_REMOVE));

    /* free blocks and inode */
    rm_inode(ino->i_num);

    release_inode(dir);
    release_inode(ino);

    return OK;

err:
    release_inode(tmpdir);
    release_inode(dir);
    release_inode(ino);

    return ERROR;
}

int fs_getdents(int fd, char *buf, size_t n)
{
    unsigned int i, pos;
    struct inode_s *dir, *ino;
    struct dir_entry_s dentry;
    struct dirent *dent;
    struct file_s *flip;

    flip = get_file(fd);
    dir = flip->f_ino;

    if (!IS_DIR(dir->i_mode))
        return ERROR;

    i = 0;
    pos = flip->f_pos;
    while (n >= sizeof(struct dirent) + 1) {
        if (next_entry(dir, &pos, &dentry) == ERROR)
            break;
        ino = get_inode(dentry.num);
        dent = (struct dirent *) (buf + i);
        dent->d_ino = dentry.num; 
        dent->d_off = pos - sizeof(struct dir_entry_s);
        dent->d_reclen = mystrncpy(dent->d_name, dentry.name, MAX_NAME) + 
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
        release_inode(ino);
    }
    flip->f_pos = pos;

    return i;
}

/* generic flush */
int sys_flush(int fd)
{
    struct file_s *flip = get_file(fd);

    if (flip == NULL)
        return ERROR;

    if (flip->f_op == NULL)
        debug_panic("sys_flush: f_op is NULL");

    if (flip->f_op->flush == NULL)
        debug_panic("sys_flush: flush func pointer is NULL");

    return flip->f_op->flush(flip);
}

/* make new device file in /dev folder, with type, major and minor numbers */
int fs_make_dev(const char *name, int type, dev_t major, dev_t minor)
{
    struct inode_s *ino, *dev;

    ino = dev = NULL;

    if ( (dev = find_inode(NULL, "/dev", FS_SEARCH_CREAT)) == NULL)
        debug_panic("fs_make_dev: error searching/creating /dev");

    if ( (ino = find_inode(dev, name, FS_SEARCH_CREAT)) == NULL)
        goto err;

    ino->i_zone[0] = major;
    ino->i_zone[1] = minor;
    ino->i_dirty = 1;

    release_inode(dev);
    release_inode(ino);

    return OK;

err:
    release_inode(dev);
    release_inode(ino);

    return ERROR;
}

struct inode_s *get_root()
{
    return get_inode(1);
}

static struct file_operations_s ops = {
    .read = fs_read,
    .write = fs_write,
    .lseek = fs_lseek
};

int fs_init(dev_t dev)
{
    fs_dev->f_op = dev_operations(dev);
    read_super();
    cache_init();

    /* register sys calls */
    SCALL_REGISTER(3, sys_read);
    SCALL_REGISTER(4, sys_write);
    SCALL_REGISTER(5, fs_open);
    SCALL_REGISTER(6, fs_close);
    SCALL_REGISTER(10, fs_unlink);
    SCALL_REGISTER(12, fs_chdir);
    SCALL_REGISTER(19, sys_lseek);
    SCALL_REGISTER(38, fs_rename);
    SCALL_REGISTER(39, fs_mkdir);
    SCALL_REGISTER(40, fs_rmdir);
    SCALL_REGISTER(141, fs_getdents);
    SCALL_REGISTER(200, sys_flush);

    /* register fs device */
    dev_register(DEV_FS, &ops);

    return OK;
}
