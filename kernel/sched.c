#include <minios/i386.h>
#include <minios/sched.h>
#include <minios/pm.h>

struct process_state_s *last_process;
struct process_state_s *current_process;

CIRCLEQ_HEAD(ready_list_s, process_state_s) ready_list;

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

/* unblock first process in waiting list */
void sched_unblock(waiting_list_t *list)
{
    struct process_state_s *process;

    process = LIST_FIRST(list);
    if (process != NULL) {
        LIST_REMOVE(process, wait);
        CIRCLEQ_INSERT_HEAD(&ready_list, process, ready);
    }
}

/* block process and put it in waiting list */
void sched_block(struct process_state_s *process, waiting_list_t *list)
{
    /* remove from list */
    CIRCLEQ_REMOVE(&ready_list, process, ready);

    /* add to waiting list */
    LIST_INSERT_HEAD(list, process, wait);

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
