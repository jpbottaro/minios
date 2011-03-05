#include "sched.h"
#include "tss.h"

struct process_state_s ps[MAX_PROCESSES];
struct process_state_s *current_process;
CIRCLEQ_HEAD(sched_list_t, process_state_s) sched_list;
LIST_HEAD(unused_list_t, process_state_s) unused_list;
unsigned int pid;

void init_scheduler()
{
    int i;
    struct process_state_s *process;

    /* init unused process entries list */
    i = 1;
    process = &ps[2];
    LIST_INIT(&unused_list);
    for (; process < &ps[MAX_PROCESSES]; ++process) {
        process->i = i;
        process->pid = 0;
        LIST_INSERT_HEAD(&unused_list, process, unused);
    }

    /* init scheduler's list */
    CIRCLEQ_INIT(&sched_list);

    /* init tss array */
    init_tss();

    /* add idle task */
    process = &ps[1];
    process->i = 1;
    process->pid = 1;
    process->uid = 0;
    process->gid = 0;
    CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);
    add_idle(1);

    current_process = NULL;
    pid = 2;
}

void schedule()
{
    /* first time we do schedule? go idle */
    if (current_process == NULL) {
        current_process = &ps[1];
        load_process(1);
    /* only do this if there are more than 1 process ready */
    } else if (current_process->schedule.cqe_next != current_process) {
        current_process = current_process->schedule.cqe_next;
        load_process(current_process->i);
    }
}

int add_process(char *path)
{   
/*
    struct process_state_s *process;

    process = LIST_FIRST(&unused_fd);
    if (process != NULL) {
        LIST_REMOVE(process, unused);
        process->pid = pid++;
        process->uid = 0;
        process->gid = 0;
        CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);
        add_tss(process->i, eip, size);
    }
*/
    return -1;
}

int kill_process(unsigned int pid)
{
    return -1;
}

unsigned int current_uid()
{
    return current_process->uid;
}

unsigned int current_gid()
{
    return current_process->gid;
}
