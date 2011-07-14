#include <minios/mm.h>
#include <minios/fs.h>
#include <minios/idt.h>
#include <minios/vga.h>
#include <minios/i386.h>
#include <minios/pipe.h>
#include <minios/sched.h>
#include <minios/debug.h>
#include "scall.h"
#include "clock.h"

#define FS_INITIAL_POS 0x20000

/* temporary, will see what is the better way to load drivers */
#include "../drivers/tty/con.h"
#include "../drivers/ramdisk/ramdisk.h"
#include "../drivers/hdd/hdd.h"
#include "../drivers/serial/serial.h"

/* here is where the magic starts */
void kernel_init()
{
    /* this has to come first, as it clears all system calls */
    scall_init();

    /* clear the screen */
    vga_clear();

    /* initialize interrupt table */
    idt_init();

    /* initialize debug info (and indirectly most exception handlers) */
    debug_init();

    /* initialize memory managment unit */
    mm_init();

    /* enable paging (kernel is identity mapped from 0x0 to 0x400000) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);

    /* set hw clock to 50hz */
    clock_init(50);

    /* add ramdisk driver */
    ramdisk_init(FS_INITIAL_POS);

    /* add ramdisk driver */
    hdd_init();

    /* initialize our file system */
    fs_init(DEV_HDD);

    /* init the pipe module */
    pipe_init();

    /* init console driver */
    con_init();

    /* init serial driver */
    serial_init();

    /* initialize scheduler */
    sched_init();

    /* add the shell as first program */
    sys_newprocess("/bin/cash", NULL);

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
