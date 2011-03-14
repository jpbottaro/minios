#include "fs.h"
#include <minikernel/misc.h>
#include <minikernel/panic.h>

static char *parse_path(char *path);

/* find the target inode based on the 'user_path' and the current directory 'dir';
 * return its pointer in 'res'
 *
 * depending on the parameter 'flag', it removes the file/dir, creates the inode
 * in the directory, or it just gets it if it exists
 */
ino_t find_inode(struct inode_s *dir, const char *user_path, int flag)
{
    struct inode_s *r;
    struct dir_entry_s *dentry;
    char *begin, *end, path[MAX_PATH];
    ino_t tmp;
 
    /* copy path and check if it is too long */
    if (mystrncpy(path, user_path, MAX_PATH) < 0)
        return NO_INODE;
   
    /* start at root if path starts with slash */
    if (dir == NULL || *path == '/') {
        r = root;
    } else {
        /* check if dir was removed or is not a directory */
        if (!IS_DIR(dir->i_mode) || dir->i_nlinks == NO_LINK)
            return NO_INODE;
        r = dir;
    }

    begin = path;
	while (*begin == '/') begin++;

    /* remove the case that the result is root */
    if (*begin == '\0')
        return 1;

    /* search the last directory of the path */
    dentry = NULL;
    end = parse_path(begin);
    while (*end != '\0') {
        /* only follow path if component is in fact a directory */
        if (!IS_DIR(r->i_mode))
            return NO_INODE;

        /* advance to the next component of the path */
        if ( (dentry = search_inode(r, begin)) == NULL)
            return NO_INODE;
        if ( (r = get_inode(dentry->num)) == NULL)
            return NO_INODE;

        /* parse next path component */
        begin = end;
        end = parse_path(begin);
    }

    if (flag == FS_SEARCH_LASTDIR) {
        if (dentry == NULL)
            return current_dir();
        else
            return dentry->num;
    }

    if ( (dentry = search_inode(r, begin)) == NULL)
        return NO_INODE;

    switch (flag) {
        case FS_SEARCH_GET:
            return dentry->num;

        case FS_SEARCH_CREAT:
            if (dentry->num != 0)
                return dentry->num;

            /* CREAT keeps going... */
        case FS_SEARCH_ADD:
            if (dentry->num != 0)
                return NO_INODE;

            if ( (tmp = empty_inode()) == NO_INODE)
                return NO_INODE;
            dentry->num = tmp;
            mystrncpy(dentry->name, begin, MAX_NAME);
            r->i_size += DIRENTRY_SIZE;
            return dentry->num;

        case FS_SEARCH_REMOVE:
            tmp = dentry->num;
            if (tmp != 0) {
                dentry->num = NO_INODE;
                r->i_size -= DIRENTRY_SIZE;
                if (IS_DIR(get_inode(tmp)->i_mode)) {
                    /* we assume the directory is already empty */
                    r->i_nlinks--;
                }
            }
            return tmp;
    }

    return NO_INODE;
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

/* search an inode in a directory based on its name, return a pointer to it
 * if it is found, or an empty entry otherwise
 *
 * if name is null, find the first non-empty entry which is not '.' or '..'
 */
struct dir_entry_s *search_inode(struct inode_s *dir, const char *name)
{
    struct dir_entry_s *dentry, *end, *empty;
    unsigned int pos;

    if (name != NULL && *name == '\0')
        return NULL;

    if (dir == NULL)
        return NULL;

    empty = NULL;
    for (pos = 0; pos < dir->i_size; pos += BLOCK_SIZE) {
        /* get the block with the files/subdirectories */
        dentry = (struct dir_entry_s *) get_block(read_map(dir, pos, FS_READ));
        end = dentry + NR_DIR_ENTRIES;

        /* if there are no more entries.. */
        if (dentry == NULL)
            return empty;

        /* cycle through the dir entries and search for the required name */
        for (; dentry < end; dentry++) {
            if (name != NULL && empty == NULL && dentry->num == 0)
                empty = dentry;

            /* pretty ugly but well... */
            if (dentry->num != 0) {
                if (name == NULL) {
                    if (mystrncmp(".",  dentry->name, 2) != 0 &&
                        mystrncmp("..", dentry->name, 3) != 0)
                        return dentry;
                } else if (mystrncmp(name, dentry->name, MAX_NAME) == 0) {
                    return dentry;
                }
            }

        }
    }

    return empty;
}

/* get the pointer of an inode based on its number */
struct inode_s *get_inode(ino_t num)
{
    /* get starting point of the memory mapped file system */
    char *p = fs_offset;

    /* check limits */
    if (num > INODE_MAX || num == NO_INODE) return NULL;

    /* inode 0 doesn not exist */
    num--;

    /* go to the inode part */
    p += INODE_OFFSET;

    /* get the desired inode */
    p += num * INODE_SIZE;

    return (struct inode_s *) p;
}

/* get the pointer for the start of a block based on its number */
void *get_block(zone_t num)
{
    if (num == NO_BLOCK)
        return NULL;

    /* get starting point of the memory mapped file system */
    char *p = fs_offset;

    /* get the desired block */
    p += num * BLOCK_SIZE;

    return (void *) p;
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
