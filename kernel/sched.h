#ifndef __SCHED_H__
#define __SCHED_H__

#include <minios/sched.h>

#define MAX_ARG 10

void init_scheduler();
void init_timer(u32_t frequency);
void schedule();

void sched_ready(struct process_state_s *process);
void sched_uready(struct process_state_s *process);

extern struct process_state_s *current_process;

#endif
