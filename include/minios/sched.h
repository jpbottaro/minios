#ifndef _SCHED_H
#define _SCHED_H

#include <minios/pm.h>

void sched_init();
void sched_schedule(int force_sched);
void sched_enqueue(struct process_state_s *process);
void sched_unqueue(struct process_state_s *process);
void sched_block(struct process_state_s *process, unsigned int dev);
void sched_unblock(unsigned int dev);

extern struct process_state_s *last_process;
extern struct process_state_s *current_process;

#endif /* _SCHED_H */
