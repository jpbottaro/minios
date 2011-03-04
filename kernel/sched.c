#include "sched.h"

extern void idle();
extern void save_state(struct process_state_s *p);
extern void load_state(struct process_state_s *p);

struct process_state_s ps[MAX_PROCESSES];
struct process_state_s *current_process;
CIRCLEQ_HEAD(sched_list_t, process_state_s) sched_list;
LIST_HEAD(unused_list_t, process_state_s) unused_list;

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

    current_process = NULL;
}

void schedule()
{
    /* first time we do schedule? go idle */
    if (current_process == NULL) {
        current_process = &ps[0];
        load_state(current_process);

    /* only do this if there are more than 1 process ready */
    } else if (current_process->schedule.cqe_next != current_process) {
        /* save state of current process */
        save_state(current_process);
        /* load and excecute next process */
        current_process = current_process->schedule.cqe_next;
        load_state(current_process);
    }
}

void add_process(char *path)
{
}

unsigned int current_uid()
{
    return current_process->uid;
}

unsigned int current_gid()
{
    return current_process->gid;
}
