#include "fs.h"
#include <minikernel/misc.h>

char *parse_path(char *path);
struct inode_s *search_inode(struct inode_s *dir, char *path);
struct inode_s *prev_dir(struct inode_s *dir);

/* find the target inode based on the 'user_path' and the current directory 'dir';
 * return its pointer in res, and the directory it is containd in 'last_dir'
 */
int find_inode(struct inode_s *dir, const char *user_path,
               /* out */
               struct inode_s **res, struct inode_s **last_dir)
{
    struct inode_s *r;
    char *begin, *end, path[MAX_PATH + 1];

    *res = *last_dir = 0;

    /* copy path and check if it is too long */
    if (mystrncpy(user_path, path, MAX_PATH) < 0)
        return ERR_BADPATH;
    
    if (*path == '/') {
        r = root;
    } else {
        /* check if dir was removed */
        if (dir->i_nlinks == NO_LINK)
            return ERR_BADPATH;
        r = dir;
    }

    begin = end = path;
	while (*begin == '/') begin++;
    while (*begin != '\0') {
        /* only follow path if component is in fact a directory */
        if (!IS_DIR(r->i_mode))
            return ERR_BADPATH;

        /* parse next path component */
        end = parse_path(begin);

        /* if end is 0, save last dir */
        if (*end == '\0') *last_dir = r;

        /* advance to the next component of the path */
        if ( (r = search_inode(r, begin)) == 0) {
            if (*end == '\0')
                return ERR_NOTEXIST;
            else
                return ERR_BADPATH;
        }
        begin = end;
    }
    *res = r;

    /* check if last_dir was not set (only reasons can be that path had nothing
     * or that the last component was '.'), in that case we force the search
     */
    if (r == dir || *last_dir == r)
        *last_dir = prev_dir(r);

    return 0;
}
/* parse next path component, omitting heading slashes '/' */
char *parse_path(char *path)
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

/* get the inode of parent directory */
struct inode_s *prev_dir(struct inode_s *dir)
{
    /* only non-root directories */
    if (dir == root || !IS_DIR(dir->i_mode)) return 0;

    /* get first directory block */
    struct dir_entry_s *block = (struct dir_entry_s *) get_block(dir->i_zone[0]);

    /* go to the second entry (skip '.' and land in '..') */
    block++;

    return get_inode(block->num);
}

/* search an inode in a directory based on its name, return a pointer to it
 * if it is found, or 0 otherwise
 * OBS: for the sake of simplicity, we only implemented directories with
 *      1 direct block (that is around 32 files/subdirectories max in it)
 */
struct inode_s *search_inode(struct inode_s *dir, char *name)
{
    if (*name == '\0')
        return 0;

    if (dir == 0)
        return 0;

    /* get the block with the files/subdirectories */
    struct dir_entry_s *dentry = (struct dir_entry_s *) get_block(dir->i_zone[0]);
    struct dir_entry_s *end = dentry + NR_DIR_ENTRIES;

    /* cycle through the dir entries and search for the required name */
    for (; dentry < end; dentry++)
        if (dentry->num != 0 && mystrncmp(name, dentry->name, MAX_NAME) == 0)
            break;

    /* this means we found it */
    if (dentry < end)
        return get_inode(dentry->num);

    return 0;
}

/* find first empty entry in a directory; similar to search_dir
 * OBS: for the sake of simplicity, we only implemented directories with
 *      1 direct block (that is around 32 files/subdirectories max in it)
 */
struct dir_entry_s *empty_dir_entry(struct inode_s *dir)
{
    if (dir == 0)
        return 0;

    struct dir_entry_s *dentry = (struct dir_entry_s *) get_block(dir->i_zone[0]);
    struct dir_entry_s *end = dentry + NR_DIR_ENTRIES;

    for (; dentry < end; dentry++)
        if (dentry->num == 0)
            return dentry;

    return 0;
}

/* get the pointer of an inode based on its number */
struct inode_s *get_inode(ino_t num)
{
    /* get starting point of the memory mapped file system */
    char *p = fs_offset;

    /* check limits */
    if (num > INODE_MAX) return 0;

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
    /* get starting point of the memory mapped file system */
    char *p = fs_offset;

    /* get the desired block */
    p += num * BLOCK_SIZE;

    return (void *) p;
}

/* return an unused inode (and mark it as used) */
ino_t empty_inode()
{
    /* XXX HACERRRRRRRR */
    return NO_INODE;
}

/* return an unused inode (and mark it as used) */
block_t empty_block()
{
    /* XXX HACERRRRRRRR */
    return NO_BLOCK;
}

int rm_inode(ino_t ino_num)
{
    /* XXX HACERRRRRRRR */
    return -1;
}

int rm_block(block_t block_num)
{
    /* XXX HACERRRRRRRR */
    return -1;
}

/* given file inode and position, find in what block that position lies and
 * return it; stripped down version from minixv3, assuming 1-block zones */
block_t read_map(struct inode_s *ino, unsigned int pos)
{
    /* XXX HACERRRRRRRR */
    return NO_BLOCK;
}
