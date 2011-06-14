#include <minios/idt.h>
#include "pic.h"

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
}

idt_entry idt[255] = {};

idt_descriptor IDT_DESC = {sizeof(idt)-1, (unsigned int)&idt};
