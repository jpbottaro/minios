#ifndef _IDT_H
#define _IDT_H

#define DEFAULT_PL 0x8E00

void idt_init();
void idt_register(int intr, void (*isr)(void), int pl);

typedef struct str_idt_descriptor {
	unsigned short idt_length;
	unsigned int idt_addr;
} __attribute__((__packed__)) idt_descriptor;

typedef struct str_idt_entry_fld {
		unsigned short offset_0_15;
		unsigned short segsel;
		unsigned short attr;
		unsigned short offset_16_31;
} __attribute__((__packed__, aligned (8))) idt_entry;

extern idt_entry idt[];
extern idt_descriptor IDT_DESC;

#endif /* _IDT_H */
