#ifndef __TSS_H__
#define __TSS_H__

#include <minios/pm.h>

struct tss_s {
	unsigned short ptl;
	unsigned short  unused0;
	unsigned int esp0;
	unsigned short  ss0;
	unsigned short  unused1;
	unsigned int esp1;
	unsigned short  ss1;
	unsigned short  unused2;
	unsigned int esp2;
	unsigned short  ss2;
	unsigned short  unused3;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned short  es;
	unsigned short  unused4;
	unsigned short  cs;
	unsigned short  unused5;
	unsigned short  ss;
	unsigned short  unused6;
	unsigned short  ds;
	unsigned short  unused7;
	unsigned short  fs;
	unsigned short  unused8;
	unsigned short  gs;
	unsigned short  unused9;
	unsigned short  ldt;
	unsigned short  unused10;
	unsigned short  dtrap;
	unsigned short  iomap;
} __attribute__((__packed__, aligned (8)));

void init_tss();
int add_idle(unsigned int pos);
int add_tss(unsigned int pos, unsigned int user_cr3);
void load_process(unsigned int num);
unsigned int task_desc(unsigned int pos);

extern struct tss_s tss[MAX_PROCESSES];

#endif //__TSS_H__
