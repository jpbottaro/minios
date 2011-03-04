#include "sched.h"

extern void idle();
extern void save_state(struct process_state_s *p);
extern void load_state(struct process_state_s *p);

struct process_state_s ps[MAX_PROCESSES];
CIRCLEQ_HEAD(sched_list_t, process_state_s) sched_list;
LIST_HEAD(unused_list_t, process_state_s) unused_list;
unsigned int current_process;
struct process_state_s *p;

void init_scheduler()
{
    int i;
    struct process_state_s *process;

    i = 1;
    process = &ps[1];
    LIST_INIT(&unused_list);
    for (; process < &ps[MAX_PROCESSES]; ++process) {
        process->i = i;
        process->pid = 0;
        LIST_INSERT_HEAD(&unused_list, process, unused);
    }

    /* add idle task */
    process = &ps[0];
    process->i = 0;
    process->pid = 1;
    process->uid = 0;
    process->gid = 0;
    process->eip = (unsigned int) idle;

    CIRCLEQ_INIT(&sched_list);
    CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);
}

void schedule()
{
    /* dont do anything if there is only one process */
    if (p->schedule.cqe_next == p)
        return;

    /* save state of current process */
    save_state(p);

    /* load and excecute next process */
    p = p->schedule.cqe_next;
    current_process = p->i;
    load_state(p);
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
