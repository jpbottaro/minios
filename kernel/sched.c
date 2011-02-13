#include "sched.h"

struct process_state_s ps[MAX_PROCESSES];
unsigned int current_process;

void schedule()
{
}

void add_process(char *path)
{
}

unsigned int current_uid()
{
    return ps[current_process].uid;
}

unsigned int current_gid()
{
    return ps[current_process].gid;
}
