#include <minios/mm.h>
#include <minios/fs.h>
#include <minios/vga.h>
#include <minios/i386.h>
#include <minios/sched.h>
#include "idt.h"
#include "pic.h"
#include "scall.h"
#include "clock.h"

#define FS_INITIAL_POS 0x20000

/* here is where the magic starts */
void kernel_init()
{
    /* clear the screen */
    vga_init();

    /* initialize memory managment unit */
    mm_init();

    /* enable paging (kernel is identity mapped from 0x0 to 0x400000) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);

    /* initialize interrupt table */
    idt_init();

    /* enable pic */
    disable_pic();
    reset_pic();
    enable_pic();

    /* set hw clock to 50hz */
    clock_init(50);

    /* initialize our memory mapped file system */
    init_fs(FS_INITIAL_POS);

    /* initialize scheduler and add the shell as first program */
    sched_init();
    sys_newprocess("/bin/cash", NULL);

    /* initialize system calls */
    scall_init();

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
