#include <minikernel/panic.h>
#include <minikernel/fs.h>
#include <minikernel/sched.h>

void *system_calls[255] = {};

void sys_nocall()
{
    panic("unknown syscall");
}

void init_scall()
{
    int i;

    for (i = 0; i < 255; i++)
        system_calls[i] = sys_nocall;

    system_calls[1] = sys_exit;
    system_calls[3] = sys_read;
    system_calls[4] = sys_write;
    system_calls[5] = sys_open;
    system_calls[6] = sys_close;
    system_calls[7] = sys_waitpid;
    system_calls[10] = sys_unlink;
    system_calls[11] = sys_newprocess;
    system_calls[12] = sys_chdir;
    system_calls[19] = sys_lseek;
    system_calls[20] = sys_getpid;
    system_calls[38] = sys_rename;
    system_calls[39] = sys_mkdir;
    system_calls[40] = sys_rmdir;
    system_calls[141] = sys_getdents;
}
