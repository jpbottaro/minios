#include <con.h>

void con_init()
{
    /* make stdin, stdout, stderr */
    fs_make_chardev("stdin", I_CHAR, DEV_STDIN, 0);
    fs_make_chardev("stdout", I_CHAR, DEV_STDOUT, 1);
    fs_make_chardev("stderr", I_CHAR, DEV_STDERR, 2);
}
