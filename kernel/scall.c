#include <minios/sched.h>
#include <minios/debug.h>
#include <minios/vga.h>
#include <minios/idt.h>
#include <minios/mm.h>
#include <minios/fs.h>

void *system_calls[255] = {};

extern void scall_handler();

void sys_nocall()
{
    debug_panic("scall: unknown system call");
}

void scall_register(int nr, void (*sys)())
{
    system_calls[nr] = sys;
}

void scall_init()
{
    int i;

    for (i = 0; i < 255; i++)
        system_calls[i] = sys_nocall;

    /* register syscall handler */
    idt_register(128, scall_handler, USER_PL);
}
