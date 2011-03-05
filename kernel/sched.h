#ifndef __SCHED_H__
#define __SCHED_H__

#include <minikernel/sched.h>

void schedule();
int add_process(char *path);
int kill_proces(unsigned int id);

#endif
