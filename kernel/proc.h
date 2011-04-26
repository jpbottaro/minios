#ifndef __PROC_H__
#define __PROC_H__

#define IDLE (&ps[1])
#define STACK_PAGE  0xFFFFF000
#define KSTACK_PAGE 0xFFFFE000
#define ARG_PAGE    0xFFFFD000

void init_proc();

extern struct process_state_s ps[MAX_PROCESSES];

#endif /* __PROC_H__ */
