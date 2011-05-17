#include <minios/mm.h>
#include <minios/fs.h>
#include <minios/vga.h>
#include <minios/i386.h>
#include <minios/sched.h>
#include <minios/idt.h>
#include "scall.h"
#include "clock.h"

#define FS_INITIAL_POS 0x20000

#include <minios/debug.h>

/* here is where the magic starts */
void kernel_init()
{
    /* this has to come first, as it clears all system calls */
    scall_init();

    /* clear the screen */
    vga_init();

    /* initialize memory managment unit */
    mm_init();

    /* enable paging (kernel is identity mapped from 0x0 to 0x400000) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);

    /* initialize interrupt table */
    idt_init();

    /* initialize debug info (and indirectly most exception handlers) */
    debug_init();

    /* set hw clock to 50hz */
    clock_init(50);

    /* initialize our memory mapped file system */
    fs_init(FS_INITIAL_POS);

    /* initialize scheduler */
    sched_init();

    /* add the shell as first program */
    sys_newprocess("/bin/cash", NULL);

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
