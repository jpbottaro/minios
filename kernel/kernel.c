#include <minios/mm.h>
#include <minios/fs.h>
#include <minios/vga.h>
#include <minios/i386.h>
#include "idt.h"
#include "pic.h"
#include "scall.h"
#include "sched.h"

#define FS_INITIAL_POS 0x20000

void kernel_init()
{
    /* clear the screen */
    vga_init();

    /* initialize memory managment unit */
    init_mm();

    /* enable paging (kernel is identity mapped from 0x0 to 0x400000) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);

    /* initialize interrupt table */
    init_idt();

    /* enable pic */
    disable_pic();
    reset_pic();
    enable_pic();

    /* set hw clock to 50hz */
    init_timer(50);

    /* initialize our memory mapped file system */
    init_fs(FS_INITIAL_POS);

    /* initialize scheduler and add the shell as first program */
    init_scheduler();
    sys_newprocess("/bin/cash", NULL);

    /* initialize system calls */
    init_scall();

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
