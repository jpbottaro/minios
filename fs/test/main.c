#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define BIG 0x100000
#define NR_ZONES 9

typedef unsigned char   u8_t;      /* 8 bit type */
typedef unsigned short u16_t;      /* 16 bit type */
typedef signed char     i8_t;      /* 8 bit signed type */
typedef short          i16_t;      /* 16 bit signed type */

#if __SIZEOF_LONG__ > 4
/* compiling with gcc on some (e.g. x86-64) platforms */
typedef unsigned int  u32_t;       /* 32 bit type */
typedef int           i32_t;      /* 32 bit signed type */
#else
/* default for ACK or gcc on 32 bit platforms */
typedef unsigned long  u32_t;      /* 32 bit type */
typedef long           i32_t;      /* 32 bit signed type */
#endif

struct inode_s {
    u16_t i_mode;      /* file type, protection, etc. */
    u16_t i_nlinks;    /* how many links to this file */
    u16_t i_uid;       /* user id of the file's owner */
    u16_t i_gid;       /* group number */
    u32_t i_size;      /* current file size in bytes */
    u32_t i_atime;
    u32_t i_mtime;     /* when was file data last changed */
    u32_t i_ctime;
    u32_t i_zone[10];  /* zone numbers for direct, ind, and dbl ind */
};

struct dir_entry_s {
    u16_t num;
    char name[30];
};

void *get_block(unsigned short);
struct inode_s *get_inode(unsigned short);
int fs_init(char *);
int print_dir(struct inode_s *dir);
int find_inode(struct inode_s *dir, char *user_path,
               /* out */
               struct inode_s **res, struct inode_s **last_dir);

extern struct inode_s *root;


char fs[BIG];

int main(int argc, char *argv[])
{
    int fd, ret;
    char *p = fs;
    struct inode_s *inode, *par;

    fd = open("fsimage", O_RDONLY);

    while ( (ret = read(fd, p, 1024)) > 0) p += ret;

    printf("Leimos %ld bytes\n", p - fs);

    fs_init(fs);

    printf("ret: %d\n", find_inode(root, argv[1], &inode, &par));

    printf("Number of links: %d\n", inode->i_nlinks);
    printf("User ID: %d\n", inode->i_uid);
    printf("Group ID: %d\n", inode->i_gid);
    printf("Size: %d\n", inode->i_size);
    printf("Time: %d\n", inode->i_mtime);

    print_dir(inode);

    return 0;
}

#define DIR_MAXENTRIES 10

int print_dir(struct inode_s *dir)
{
    int i;

    /* get the block with the files/subdirectories */
    printf("Numero del bloque: %d\n", dir->i_zone[0]);

 struct dir_entry_s *dentry = (struct dir_entry_s *) get_block(dir->i_zone[0]);

    /* cycle through the dir entries and search for the required name */
    for (i = 0; i < DIR_MAXENTRIES;i++) { // && dentry->num != 0; i++) {
        printf("Entrada #%d: %d %s\n", i, dentry->num, dentry->name);
        dentry++;
    }

    return 0;
}
