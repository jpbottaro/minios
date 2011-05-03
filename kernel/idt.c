#include "isr.h"
#include "pic.h"
#include "idt.h"

void idt_register(int intr, void (*isr)(void), int pl)
{
    idt[intr].offset_0_15 = (unsigned short)
            ((unsigned int)(isr) & (unsigned int) 0xFFFF);
    idt[intr].segsel = (unsigned short) 0x0008;
    idt[intr].attr = (unsigned short) pl;
    idt[intr].offset_16_31 = (unsigned short)
            ((unsigned int)(isr) >> 16 & (unsigned int) 0xFFFF);
}

void idt_init() {
    /* enable pic */
    disable_pic();
    reset_pic();
    enable_pic();

    /* register clock handler */
    idt_register(32, _isr32, DEFAULT_PL);

    /* register kbd handler */
    idt_register(33, _isr33, DEFAULT_PL);

    /* register syscall handler */
    idt_register(128, _isr128, DEFAULT_PL);
}

idt_entry idt[255] = {};

idt_descriptor IDT_DESC = {sizeof(idt)-1, (unsigned int)&idt};
