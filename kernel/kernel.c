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
    /* clear the screen */
    vga_clear();

    /* this has to come first, as it clears all system calls */
    scall_init();

    /* initialize interrupt table */
    idt_init();

    /* initialize debug info (and indirectly most exception handlers) */
    debug_init();

    /* initialize memory managment unit (this enables paging) */
    mm_init();

    /* set hw clock to 50hz */
    clock_init(50);

    /* add ramdisk driver */
    ramdisk_init(FS_INITIAL_POS);

    /* initialize our file system */
    fs_init(DEV_RAMDISK);

    /* add hdd driver */
    hdd_init();

    /* init the pipe module */
    pipe_init();

    /* init console driver */
    con_init();

    /* init serial driver */
    serial_init();

    /* initialize scheduler */
    sched_init();

    /* add the shell as first program */
    sys_newprocess("/bin/init", NULL);

    /* enable interruptions (and therefore scheduler) */
    sti();

    for(;;) {}
}
