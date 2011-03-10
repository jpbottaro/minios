#ifndef __GDT_H__
#define __GDT_H__

#define SEG_DESC_KCODE 0x8
#define SEG_DESC_KDATA 0x10
#define SEG_DESC_UCODE 0x1B
#define SEG_DESC_UDATA 0x23
#define SEG_DESC_VIDEO 0x28

typedef struct str_gdt_descriptor {
	unsigned short gdt_length;
	unsigned int gdt_addr;
} __attribute__((__packed__)) gdt_descriptor;


typedef struct str_gdt_entry {
	unsigned short limit_0_15;
	unsigned short base_0_15;
	unsigned char base_23_16;
	unsigned char type:4;       // code/data
	unsigned char s:1;          // 0 = system 1 = code or data
	unsigned char dpl:2;        // privilege level
	unsigned char p:1;          // present
	unsigned char limit_16_19:4;
	unsigned char avl:1;        // 0 always
	unsigned char l:1;          // 0 (64-bit code segment)
	unsigned char db:1;
	unsigned char g:1;          // 1 (4k chunks)
	unsigned char base_31_24;
} __attribute__((__packed__, aligned (8))) gdt_entry;

/** Tabla GDT **/
extern gdt_entry gdt[];
extern gdt_descriptor GDT_DESC;

unsigned int gdt_free_entry();
unsigned int gdt_desc(unsigned int ptr);

#define GDT_COUNT 128
#endif //__GDT_H__
