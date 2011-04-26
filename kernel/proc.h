#ifndef __PROC_H__
#define __PROC_H__

#define IDLE (&ps[1])

void init_proc();

extern struct process_state_s ps[MAX_PROCESSES];

#endif /* __PROC_H__ */
