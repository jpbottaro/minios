#include <minios/dev.h>
#include <minios/fs.h>
#include "../drivers/tty/con.h"

void devfs_init(void)
{
    int i;
    char tty[] = "tty0";

    for (i = 0; i < MAX_CONSOLES; i++) {
        tty[3] = i + '0';
        fs_make_dev(tty, I_CHAR, DEV_TTY, i);
    }
    fs_make_dev("tty", I_CHAR, DEV_TTY, 0);
    fs_make_dev("hdd", I_BLOCK, DEV_HDD, 0);
    fs_make_dev("serial", I_CHAR, DEV_SERIAL, 0);
}
