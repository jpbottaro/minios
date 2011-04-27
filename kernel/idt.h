#ifndef __IDT_H__
#define __IDT_H__

void idt_init();

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

#endif /* __IDT_H__ */
