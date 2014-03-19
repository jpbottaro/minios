#include <minios/mm.h>
#include <minios/fs.h>
#include <minios/dev.h>
#include <minios/idt.h>
#include <minios/vga.h>
#include <minios/i386.h>
#include <minios/sched.h>
#include <minios/debug.h>
#include "devfs.h"
#include "scall.h"
#include "clock.h"
#include "vmm.h"

/* here is where the magic starts */
void kernel_init()
{
    /* clear the screen */
    vga_clear();

    /* this has to come first, as it clears all system calls */
    scall_init();

    /* interrupt table */
    idt_init();

    /* debug info (and indirectly most exception handlers) */
    debug_init();

    /* memory managment unit (this enables paging) */
    mm_init();

    /* supported device drivers */
    dev_init();

    /* file system */
    fs_init(DEV_HDD);

    /* virtual memory */
    vmm_init(DEV_HDD, SWAP_OFFSET);

    /* create device nodes (in /dev) */
    devfs_init();

    /* scheduler */
    sched_init();

    /* set hw clock to 50hz */
    clock_init(50);

    /* add the shell as first program */
    sys_newprocess("/bin/init", NULL);

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
