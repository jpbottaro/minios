#include "isr.h"
#include "idt.h"
#include "i386.h"


#define IDT_ENTRY(number) \
	idt[number].offset_0_15 = (unsigned short) ((unsigned int)(&_isr ## number) & (unsigned int) 0xFFFF); \
	idt[number].segsel = (unsigned short) 0x0008; \
	idt[number].attr = (unsigned short) 0x8E00; \
	idt[number].offset_16_31 = (unsigned short) ((unsigned int)(&_isr ## number) >> 16 & (unsigned int) 0xFFFF);


void init_idt() {
    IDT_ENTRY(0);  // Divide Error
    IDT_ENTRY(1);  // Debug  
    IDT_ENTRY(2);  // NMI Interrupt
    IDT_ENTRY(3);  // Breakpoint	 	
    IDT_ENTRY(4);  // Overflow
    IDT_ENTRY(5);  // Bound Range Exceeded
    IDT_ENTRY(6);  // Invalid Opcode ( Undefined Opcode )
    IDT_ENTRY(7);  // Device Not Available ( No Math Coprocessor )
    IDT_ENTRY(8);  // Double Fault
    IDT_ENTRY(9);  // Coprocessor Segment Overrun (reserved)
    IDT_ENTRY(10); // Invalid TSS
    IDT_ENTRY(11); // Segment Not Present
    IDT_ENTRY(12); // Stack-Segment Fault
    IDT_ENTRY(13); // General Protection
    IDT_ENTRY(14); // Page Fault
    IDT_ENTRY(15); // (Intel Reserved, do not use)
    IDT_ENTRY(16); // Floating-Point Error (Math Fault)
    IDT_ENTRY(17); // Aligment Check 
    IDT_ENTRY(18); // Machine Check
    IDT_ENTRY(19); // Streaming SIMD Extentsions
    IDT_ENTRY(32); // Remapped Clock
    IDT_ENTRY(33); // Remapped Keyboard
    IDT_ENTRY(80); // System Call
}

idt_entry idt[255] = {};

idt_descriptor IDT_DESC = {sizeof(idt)-1, (unsigned int)&idt};
