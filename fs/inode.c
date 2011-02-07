#include "fs.h"
#include <minikernel/misc.h>


ino_t search_inode(struct inode_s *dir, char *path, int flag);
struct inode_s *prev_dir(struct inode_s *dir);
char *parse_path(char *path);

/* find the target inode based on the 'user_path' and the current directory 'dir';
 * return its pointer in 'res'
 *
 * depending on the parameter 'flag', it removes the file/dir, creates the inode
 * in the directory, or it just gets it if it exists
 */
ino_t find_inode(struct inode_s *dir, const char *user_path, int flag)
{
    struct inode_s *r;
    char *begin, *end, path[MAX_PATH];
    ino_t ino_num;

    ino_num = NO_INODE;
    
    /* start at root if path starts with slash */
    if (*path == '/') {
        ino_num = 1;
        r = root;
    } else {
        /* check if dir was removed or is not a directory */
        if (!IS_DIR(dir->i_mode) || dir->i_nlinks == NO_LINK)
            return NO_INODE;
        r = dir;
    }

    /* copy path and check if it is too long */
    if (mystrncpy(user_path, path, MAX_PATH) < 0)
        return NO_INODE;

    /* search the last directory of the path */
    begin = path;
	while (*begin == '/') begin++;
    end = parse_path(begin);
    while (*end != '\0') {
        /* only follow path if component is in fact a directory */
        if (!IS_DIR(r->i_mode))
            return NO_INODE;

        /* advance to the next component of the path */
        if ( (ino_num = search_inode(r, begin, FS_SEARCH_GET)) == NO_INODE)
            return NO_INODE;
        r = get_inode(ino_num);

        /* parse next path component */
        begin = end;
        end = parse_path(begin);
    }

    return search_inode(r, begin, flag);
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

/* search an inode in a directory based on its name, return a pointer to it
 * if it is found, or 0 otherwise
 */
ino_t search_inode(struct inode_s *dir, char *name, int flag)
{
    struct dir_entry_s *dentry;
    struct dir_entry_s *end;
    struct dir_entry_s *empty;
    unsigned int pos;

    if (*name == '\0')
        return NO_INODE;

    if (dir == 0)
        return NO_INODE;

    empty = 0;
    for (pos = 0; pos < dir->i_size; pos += BLOCK_SIZE) {
        /* get the block with the files/subdirectories */
        dentry = (struct dir_entry_s *) get_block(read_map(dir, pos));
        end = dentry + NR_DIR_ENTRIES;

        /* cycle through the dir entries and search for the required name */
        for (; dentry < end; dentry++) {
            if (!empty && dentry->num == 0)
                empty = dentry;
            if (dentry->num != 0 && mystrncmp(name, dentry->name, MAX_NAME) == 0) {
                switch (flag) {
                    case FS_SEARCH_REMOVE:
                        rm_inode(dentry->num);
                        dentry->num = 0;
                        return OK;
                    case FS_SEARCH_GET:
                    case FS_SEARCH_ADD:
                        return dentry->num;
                }
            }
        }
    }

    /* if we didnt find the file, and our flag asks to create it... */
    if (flag == FS_SEARCH_ADD) {
        ino_t ino_num = empty_inode();        
        empty->num = ino_num;
        mystrncpy(name, empty->name, MAX_NAME);
        return ino_num;
    }

    return NO_INODE;
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
