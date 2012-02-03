#include <unistd.h>
#include <fcntl.h>
#include <minios/misc.h>
#include <minios/debug.h>
#include <sys/queue.h>
#include "fs.h"

struct inode_s inodes[MAX_INODES_CACHE];
struct buf_s bufs[MAX_BUFFERS_CACHE];

LIST_HEAD(unused_inodes_t, inode_s) unused_inodes;
LIST_HEAD(used_inodes_t, inode_s) used_inodes;
LIST_HEAD(unused_buffers_t, buf_s) unused_buf;
LIST_HEAD(used_buffers_t, buf_s) used_buf;

static char *parse_path(char *path);

/* initialize the inode cache */
void cache_init()
{
    int i;

    LIST_INIT(&used_inodes);
    LIST_INIT(&unused_inodes);
    for (i = 0; i < MAX_INODES_CACHE; ++i) {
        inodes[i].i_free = 1;
        inodes[i].i_num = 0;
        inodes[i].i_dirty = 0;
        inodes[i].i_refcount = 0;
        LIST_INSERT_HEAD(&unused_inodes, &inodes[i], ptr);
    }

    LIST_INIT(&used_buf);
    LIST_INIT(&unused_buf);
    for (i = 0; i < MAX_BUFFERS_CACHE; ++i) {
        bufs[i].b_free = 1;
        bufs[i].b_num = 0;
        bufs[i].b_dirty = 0;
        bufs[i].b_refcount = 0;
        LIST_INSERT_HEAD(&unused_buf, &bufs[i], ptr);
    }
}

/* find the target inode based on the 'user_path' and the current directory 'dir';
 * return its pointer in 'res'
 *
 * depending on the parameter 'flag', it removes the file/dir, creates the inode
 * in the directory, or it just gets it if it exists
 */
struct inode_s *find_inode(struct inode_s *dir, const char *user_path, int flag)
{
    int ret;
    struct inode_s *r;
    struct dir_entry_s dentry;
    char *begin, *end, path[MAX_PATH];
    ino_t tmp;
 
    /* copy path and check if it is too long */
    if (mystrncpy(path, user_path, MAX_PATH) < 0)
        debug_panic("find_inode: could not copy path, too long");
   
    /* start at root if path starts with slash */
    if (dir == NULL || *path == '/') {
        r = root;
    } else {
        /* check if dir was removed or is not a directory */
        if (!IS_DIR(dir->i_mode) || dir->i_nlinks == NO_LINK)
            debug_panic("find_inode: bad starting directory");
        r = dir;
    }

    begin = path;
	while (*begin == '/') begin++;

    /* remove the case that the result is root */
    if (*begin == '\0')
        return root;

    /* up the refcount of the current inode */
    get_inode(r->i_num);

    /* search the last directory of the path */
    dentry.num = 0;
    end = parse_path(begin);
    while (*end != '\0') {
        /* only follow path if component is in fact a directory */
        if (!IS_DIR(r->i_mode))
            goto err_release;

        /* advance to the next component of the path */
        if (search_inode(r, begin, &dentry, FS_SEARCH_GET) < 0)
            goto err_release;
        release_inode(r);
        if ( (r = get_inode(dentry.num)) == NULL)
            debug_panic("find_inode: got null from get_inode in loop...");

        /* parse next path component */
        begin = end;
        end = parse_path(begin);
    }

    if (flag == FS_SEARCH_LASTDIR) {
        /* release the last inode we used for the search */
        release_inode(r);
        if (dentry.num == 0)
            return current_dir();
        else
            return get_inode(dentry.num);
    }

    ret = search_inode(r, begin, &dentry, flag);

    switch (flag) {
        case FS_SEARCH_GET:
            release_inode(r);
            if (ret == ERROR)
                return NULL;
            else
                return get_inode(dentry.num);

        case FS_SEARCH_CREAT:
            if (ret == OK)
                return get_inode(dentry.num);

            /* CREAT keeps going... */
        case FS_SEARCH_ADD:
            if (ret == OK)
                goto err_release;

            if ( (tmp = empty_inode()) == NO_INODE)
                goto err_release;

            empty_entry(r, tmp, begin);
            release_inode(r);
            return get_inode(tmp);

        case FS_SEARCH_REMOVE:
            if (ret == ERROR)
                goto err_release;

            release_inode(r);
            return get_inode(dentry.num);
    }

    return NULL;

err_release:
    release_inode(r);
    return NULL;
}

/* parse next path component, omitting heading slashes '/' */
static char *parse_path(char *path)
{
    while (*path != '\0' && *path != '/')
        path++;
    if (*path == '/') {
        *path = '\0';
        path++;
	    while (*path == '/') path++;
    }

    return path;
}

/* get an empty directory entry and save the new inode */
int empty_entry(struct inode_s *dir, ino_t ino_num, char *name)
{
    unsigned int pos;
    struct dir_entry_s *dentry, *begin, *end;
    struct buf_s *block;
    block_t blocknr;

    pos = 0;
    while ( (blocknr = read_map(dir, pos, FS_WRITE)) != NO_BLOCK) {
        block = get_block(blocknr);
        begin = (struct dir_entry_s *) block->b_buffer;
        end = begin + BLOCK_SIZE / DIRENTRY_SIZE;

        for (dentry = begin; dentry < end; dentry++) {
            if (dentry->num == 0) {
                dentry->num = ino_num;
                mystrncpy(dentry->name, name, MAX_NAME);
                dir->i_size += DIRENTRY_SIZE;
                release_block(block);
                return OK;
            }
        }

        pos += BLOCK_SIZE;
        release_block(block);
    }

    return ERROR;
}

/* get next directory entry of a directory. *p is a pointer to the starting
 * position, and it gets updated with the search, dent is the next entry
 */
int next_entry(struct inode_s *dir, unsigned int *p, struct dir_entry_s *dent)
{
    unsigned int pos;
    struct dir_entry_s *dentry, *begin, *end;
    struct buf_s *block;
    block_t blocknr;

    pos = *p;
    while ( (blocknr = read_map(dir, pos, FS_READ)) != NO_BLOCK) {
        block = get_block(blocknr);
        begin = (struct dir_entry_s *) (block->b_buffer + pos % BLOCK_SIZE);
        end = begin + (BLOCK_SIZE - pos % BLOCK_SIZE) / DIRENTRY_SIZE;

        for (dentry = begin; dentry < end; dentry++) {
            if (dentry->num != 0) {
                *p = pos + ((dentry - begin) + 1) * DIRENTRY_SIZE;
                *dent = *dentry;
                release_block(block);
                return OK;
            }
        }

        pos += (BLOCK_SIZE - pos % BLOCK_SIZE);
        release_block(block);
    }

    *p = pos;
    return ERROR;
}

/* remove entry at a given position */
int remove_entry(struct inode_s *dir, unsigned int *p)
{
    unsigned int pos = *p - DIRENTRY_SIZE; 
    struct dir_entry_s *dentry;
    struct buf_s *block;
    block_t blocknr;

    if ( (blocknr = read_map(dir, pos, FS_READ)) == NO_BLOCK)
        return ERROR;
    block = get_block(blocknr);
    dentry = (struct dir_entry_s *) (block->b_buffer + pos % BLOCK_SIZE);
    dentry->num = NO_INODE;
    dir->i_size -= DIRENTRY_SIZE;
    release_block(block);

    return OK;
}

/* search an inode in a directory based on its name, return a pointer to it
 * if it is found, or an empty entry otherwise
 *
 * if name is null, find the first non-empty entry which is not '.' or '..'
 */
int search_inode(struct inode_s *dir, const char *name, struct dir_entry_s *dentry,                                                                         int flag)
{
    unsigned int pos, i, entries;

    if (name != NULL && *name == '\0')
        debug_panic("search_inode: no name to search");

    if (dir == NULL)
        debug_panic("search_inode: NULL directory");

    pos = 0;
    entries = dir->i_size / DIRENTRY_SIZE;
    for (i = 0; i < entries; ++i) {
        if (next_entry(dir, &pos, dentry) == ERROR)
            break;

        if (dentry->num != 0) {
            if (name == NULL) {
                if (mystrncmp(".",  dentry->name, 2) != 0 &&
                    mystrncmp("..", dentry->name, 3) != 0)
                    return OK;
            } else if (mystrncmp(name, dentry->name, MAX_NAME) == 0) {
                if (flag == FS_SEARCH_REMOVE)
                    remove_entry(dir, &pos);
                return OK;
            }
        }
    }

    return ERROR;
}

struct inode_s *_get_inode(ino_t num)
{
    struct inode_s *ino;

    LIST_FOREACH(ino, &used_inodes, ptr) {
        if (ino->i_num == num) {
            return ino;
        }
    }

    ino = LIST_FIRST(&unused_inodes);
    if (ino == NULL)
        debug_panic("_get_inode: no more free inodes");
    LIST_REMOVE(ino, ptr);
    LIST_INSERT_HEAD(&used_inodes, ino, ptr);

    return ino;
}

void copy_i2r(struct inode_s *ino, struct real_inode_s *real)
{
    int i;

    real->i_mode = ino->i_mode;
    real->i_nlinks = ino->i_nlinks;
    real->i_uid = ino->i_uid;
    real->i_gid = ino->i_gid;
    real->i_size = ino->i_size;
    real->i_atime = ino->i_atime;
    real->i_mtime = ino->i_mtime;
    real->i_ctime = ino->i_ctime;
    for (i = 0; i < NR_ZONES; ++i)
        real->i_zone[i] = ino->i_zone[i];
}

void copy_r2i(struct real_inode_s *real, struct inode_s *ino)
{
    int i;

    ino->i_mode = real->i_mode;
    ino->i_nlinks = real->i_nlinks;
    ino->i_uid = real->i_uid;
    ino->i_gid = real->i_gid;
    ino->i_size = real->i_size;
    ino->i_atime = real->i_atime;
    ino->i_mtime = real->i_mtime;
    ino->i_ctime = real->i_ctime;
    for (i = 0; i < NR_ZONES; ++i)
        ino->i_zone[i] = real->i_zone[i];
}

/* get the pointer of an inode based on its number */
struct inode_s *get_inode(ino_t num)
{
    struct real_inode_s real_ino;
    struct inode_s *ino;
    int p = 0;

    /* check limits */
    if (num > INODE_MAX || num == NO_INODE)
        debug_panic("get_inode: got bad number");

    ino = _get_inode(num);

    if (!ino->i_free) {
        ino->i_refcount++;
        return ino;
    }

    /* go to the inode part */
    p += INODE_OFFSET;

    /* get the desired inode */
    p += (num - 1) * INODE_SIZE;

    /* ask the inode from the device we are in */
    fs_dev->f_op->lseek(fs_dev, p, SEEK_SET);
    fs_dev->f_op->read(fs_dev, (char *) &real_ino, sizeof(struct real_inode_s));

    /* copy it to our cache */
    copy_r2i(&real_ino, ino);
    ino->i_pos = p;
    ino->i_free = 0;
    ino->i_num = num;
    ino->i_refcount = 1;
    ino->i_dirty = 0;

    return ino;
}

void release_inode(struct inode_s *ino)
{
    struct real_inode_s real;

    if (ino == NULL)
        return;
    if (--ino->i_refcount == 0) {
        copy_i2r(ino, &real);
        fs_dev->f_op->lseek(fs_dev, ino->i_pos, SEEK_SET);
        fs_dev->f_op->write(fs_dev, (char *) &real, sizeof(struct real_inode_s));
        ino->i_dirty = 0;
        ino->i_free = 1;
        ino->i_num = 0;
        LIST_REMOVE(ino, ptr);
        LIST_INSERT_HEAD(&unused_inodes, ino, ptr);
    }
}

struct buf_s *_get_block(zone_t num)
{
    struct buf_s *buf;

    LIST_FOREACH(buf, &used_buf, ptr) {
        if (buf->b_num == num) {
            return buf;
        }
    }

    buf = LIST_FIRST(&unused_buf);
    if (buf == NULL)
        debug_panic("_get_buf: no more free buffers");
    LIST_REMOVE(buf, ptr);
    LIST_INSERT_HEAD(&used_buf, buf, ptr);

    return buf;
}

/* get the pointer for the start of a block based on its number */
struct buf_s *get_block(zone_t num)
{
    struct buf_s *buf;

    if (num == NO_BLOCK)
        return NULL;

    buf = _get_block(num);

    if (!buf->b_free) {
        buf->b_refcount++;
        return buf;
    }

    /* read from device */
    fs_dev->f_op->lseek(fs_dev, num * BLOCK_SIZE, SEEK_SET);
    fs_dev->f_op->read(fs_dev, buf->b_buffer, BLOCK_SIZE);
    buf->b_pos = num * BLOCK_SIZE;
    buf->b_free = 0;
    buf->b_refcount = 1;
    buf->b_dirty = 0;

    return buf;
}

void release_block(struct buf_s *buf)
{
    if (buf == NULL)
        return;
    if (--buf->b_refcount <= 0) {
        fs_dev->f_op->lseek(fs_dev, buf->b_pos, SEEK_SET);
        fs_dev->f_op->write(fs_dev, buf->b_buffer, BLOCK_SIZE);
        buf->b_dirty = 0;
        buf->b_free = 1;
        buf->b_num = 0;
        LIST_REMOVE(buf, ptr);
        LIST_INSERT_HEAD(&unused_buf, buf, ptr);
    }
}

/* given file inode and position, find in what block that position lies and
 * return it; we assume 1 block zones
 */
/* XXX I could improve this to support indirect blocks if its necessary */
block_t read_map(struct inode_s *ino, unsigned int pos, int flag)
{
    unsigned int block;

    block = pos / BLOCK_SIZE;

    if (block < DIRECT_ZONES) {
        if (ino->i_zone[block] == NO_BLOCK && flag == FS_WRITE)
            return (ino->i_zone[block] = empty_block());
        else
            return ino->i_zone[block];
    }

    return NO_BLOCK;
}

int imayor(struct inode_s *ino)
{
    return ino->i_zone[0];
}

int iminor(struct inode_s *ino)
{
    return ino->i_zone[1];
}
