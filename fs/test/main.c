#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <minikernel/fs.h>
#include "../inode.h"

#define BIG 0x100000

char fs[BIG];

struct process_state_s *current_process = NULL;
struct process_state_s *ps = NULL;
current_uid() {return 0;}
current_gid() {return 0;}

int main(int argc, char *argv[])
{
    int fd, ret;
    char *p = fs;
    ino_t ino_num;
    struct inode_s *inode, *par;

    fd = open("fsimage", O_RDONLY);

    while ( (ret = read(fd, p, 1024)) > 0) p += ret;

    printf("Leimos %ld bytes\n", p - fs);

    init_fs(fs);

//    find_inode(NULL, argv[1], FS_SEARCH_REMOVE);

    printf("ret: %d\n", ino_num = find_inode(NULL, argv[1], FS_SEARCH_LASTDIR));
    inode = get_inode(ino_num);

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
