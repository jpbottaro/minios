#include <minios/i386.h>
#include <minios/mm.h>
#include <minios/pm.h>
#include "gdt.h"
#include "tss.h"

#define TSS_SIZE    0x68

#define tss_gdt_entry(entry, tss_addr) \
    entry->limit_0_15  = TSS_SIZE - 1; \
    entry->limit_16_19 = 0x0; \
    entry->base_0_15   = (unsigned short) ((unsigned int) tss_addr & 0xFFFF); \
    entry->base_23_16  = (unsigned char) (((unsigned int) tss_addr >> 16) & 0xFF); \
    entry->base_31_24  = (unsigned char) (((unsigned int) tss_addr >> 24) & 0xFF); \
    entry->type        = 0x9; \
    entry->s           = 0x0; \
    entry->dpl         = 0x0; \
    entry->p           = 0x1; \
    entry->avl         = 0x0; \
    entry->l           = 0x0; \
    entry->db          = 0x0; \
    entry->g           = 0x1;

struct tss_s {
	unsigned short ptl;
	unsigned short unused0;
	unsigned int   esp0;
	unsigned short ss0;
	unsigned short unused1;
	unsigned int   esp1;
	unsigned short ss1;
	unsigned short unused2;
	unsigned int   esp2;
	unsigned short ss2;
	unsigned short unused3;
	unsigned int   cr3;
	unsigned int   eip;
	unsigned int   eflags;
	unsigned int   eax;
	unsigned int   ecx;
	unsigned int   edx;
	unsigned int   ebx;
	unsigned int   esp;
	unsigned int   ebp;
	unsigned int   esi;
	unsigned int   edi;
	unsigned short es;
	unsigned short unused4;
	unsigned short cs;
	unsigned short unused5;
	unsigned short ss;
	unsigned short unused6;
	unsigned short ds;
	unsigned short unused7;
	unsigned short fs;
	unsigned short unused8;
	unsigned short gs;
	unsigned short unused9;
	unsigned short ldt;
	unsigned short unused10;
	unsigned short dtrap;
	unsigned short iomap;
} __attribute__((__packed__, aligned (8)));

struct tss_s tss[1];

void tss_init()
{
    gdt_entry *entry;

    entry = (gdt_entry *) gdt_free_entry();
    tss_gdt_entry(entry, tss);

    tss[0].cr3   = rcr3();
    tss[0].ss0   = SEG_DESC_KDATA;
    tss[0].esp0  = KSTACK_PAGE + PAGE_SIZE - 0x10;
    tss[0].dtrap = 0x0;
    tss[0].iomap = 0xFFFF;
    tss[0].cs = 0x0b;
    tss[0].ss = tss[0].ds = tss[0].es = tss[0].fs = tss[0].gs = 0x13;

    ltr(gdt_desc((unsigned int) entry) | 0x3);
}
