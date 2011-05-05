#ifndef _SCHED_H
#define _SCHED_H

#include <minios/pm.h>

typedef LIST_HEAD(waiting_list_s, process_state_s) waiting_list_t;

void sched_init();
void sched_schedule(int force_sched);
void sched_enqueue(struct process_state_s *process);
void sched_unqueue(struct process_state_s *process);
void sched_block(struct process_state_s *process, waiting_list_t *list);
void sched_unblock(waiting_list_t *list);

extern struct process_state_s *last_process;
extern struct process_state_s *current_process;

#endif /* _SCHED_H */
