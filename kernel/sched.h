#ifndef __SCHED_H__
#define __SCHED_H__

#include <minikernel/sched.h>

void schedule();
void add_process(char *path);
void kill_proces(unsigned int id);

#endif
