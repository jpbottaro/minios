#ifndef __PM_H__
#define __PM_H__

#include <minios/pm.h>

#define C_PARAMS_SIZE 0x8

void save_process_state(struct process_state_s *p);
void load_process_state(struct process_state_s *p);

#endif /* __PM_H__ */
