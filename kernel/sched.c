#include <minios/i386.h>
#include <minios/sched.h>
#include <minios/pm.h>

struct process_state_s *last_process;
struct process_state_s *current_process;

CIRCLEQ_HEAD(ready_list_t, process_state_s) ready_list;
LIST_HEAD(waiting_list_t, process_state_s) waiting_list;

/* intialize scheduler, must be called before any function */
void sched_init()
{
    /* init process table, and add idle as the process nr 1 */
    pm_init();

    /* init scheduler's ready list */
    CIRCLEQ_INIT(&ready_list);

    /* start with no current process */
    last_process = current_process = NULL;
}

/* unblock first process waiting for 'dev' */
void sched_unblock(unsigned int dev)
{
    struct process_state_s *process;

    /* unblock process waiting for the device */
    /* XXX this is very _VERY_ limited, since we dont check who gets it, just
     * XXX the first one. It will do for now, only for the keyboard */
    LIST_FOREACH(process, &waiting_list, wait) {
        if (process->dev == dev) {
            LIST_REMOVE(process, wait);
            CIRCLEQ_INSERT_HEAD(&ready_list, process, ready);
            return;
        }
    }
}

/* block a process waiting for device 'dev' */
void sched_block(struct process_state_s *process, unsigned int dev)
{
    /* remove from list */
    CIRCLEQ_REMOVE(&ready_list, process, ready);

    /* add to waiting list */
    process->dev = dev;
    LIST_INSERT_HEAD(&waiting_list, process, wait);

    /* if caller is the current process, schedule to next one */
    if (process == current_process)
        sched_schedule(1);
}

/* put a process in the ready list */
void sched_enqueue(struct process_state_s *process)
{
    CIRCLEQ_INSERT_HEAD(&ready_list, process, ready);
}

/* remove a process from the ready list */
void sched_unqueue(struct process_state_s *process)
{
    CIRCLEQ_REMOVE(&ready_list, process, ready);
}

/* schedule next ready process (or go idle if no procesess ready) */
void sched_schedule(int force_sched)
{
    struct process_state_s *process;

    /* if we are idle, check for new processes */
    if (current_process == IDLE) {
        if (!CIRCLEQ_EMPTY(&ready_list)) {
            process = CIRCLEQ_FIRST(&ready_list);
            last_process = current_process;
            current_process = process;
            pm_switchto(process->i);
        }
    /* no process running (or forced schedule) */
    } else if (current_process == NULL || force_sched) {
        /* if any process ready then execute, otherwise go idle */
        if (!CIRCLEQ_EMPTY(&ready_list)) {
            process = CIRCLEQ_FIRST(&ready_list);
            if (process == current_process)
                return;
            last_process = current_process;
            current_process = process;
            pm_switchto(process->i);
        } else {
            last_process = current_process;
            current_process = IDLE;
            pm_switchto(IDLE_NUM);
        }
    /* if there are more than 1 process ready */
    } else if (CIRCLEQ_NEXT(current_process, ready) !=
               CIRCLEQ_PREV(current_process, ready)) {
        last_process = current_process;
        current_process = CIRCLEQ_NEXT(current_process, ready);
        pm_switchto(current_process->i);
    }
}
