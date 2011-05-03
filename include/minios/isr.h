#ifndef __ISR_H__
#define __ISR_H__

#include <sys/types.h>

typedef struct str_exp_state {
    u32_t nr;
    char name[20];
	u32_t gs;
	u32_t fs;
	u32_t es;
	u32_t ds;
	u32_t ss;
	u32_t edi;
	u32_t esi;
	u32_t ebp;
	u32_t esp;
	u32_t ebx;
	u32_t edx;
	u32_t ecx;
	u32_t eax;
	u32_t errcode;
	u32_t org_eip;
	u32_t org_cs;
	u32_t eflags;
	u32_t org_esp;
	u32_t org_ss;
} __attribute__((__packed__)) exp_state;

#endif
