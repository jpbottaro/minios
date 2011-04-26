#include <minios/i386.h>
#include "tss.h"
#include "proc.h"
#include "sched.h"

struct process_state_s *current_process;

CIRCLEQ_HEAD(ready_list_t, process_state_s) ready_list;
LIST_HEAD(waiting_list_t, process_state_s) waiting_list;

/* init_timer taken from James Molly in
 * http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs%20and%20the%20PIT.html
 */
void init_timer(u32_t frequency)
{
   /* The value we send to the PIT is the value to divide it's input clock
    * (1193180 Hz) by, to get our required frequency. Important to note is
    * that the divisor must be small enough to fit into 16-bits.
    */
   u32_t divisor = 1193180 / frequency;

   /* Send the command byte */
   outb(0x43, 0x36);

   /* Divisor has to be sent byte-wise, so split here into upper/lower bytes */
   unsigned char l = (unsigned char) (divisor & 0xFF);
   unsigned char h = (unsigned char) ( (divisor>>8) & 0xFF );

   /* Send the frequency divisor */
   outb(0x40, l);
   outb(0x40, h);
}

/* intialize scheduler, must be called before any function */
void init_scheduler()
{
    /* init process table, and add idle as the process nr 1 */
    init_proc();

    /* init scheduler's ready list */
    CIRCLEQ_INIT(&ready_list);

    /* start with no current process */
    current_process = NULL;
}

/* unblock first process waiting for 'dev' */
void unblock_process(unsigned int dev)
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
void block_process(struct process_state_s *process, unsigned int dev)
{
    /* remove from list */
    CIRCLEQ_REMOVE(&ready_list, process, ready);

    /* add to waiting list */
    process->dev = dev;
    LIST_INSERT_HEAD(&waiting_list, process, wait);

    /* if caller is the current process, schedule to next one */
    if (process == current_process) {
        current_process = NULL;
        schedule();
    }
}

/* put a process in the ready list */
void sched_ready(struct process_state_s *process)
{
    CIRCLEQ_INSERT_HEAD(&ready_list, process, ready);
}

/* remove a process from the ready list */
void sched_uready(struct process_state_s *process)
{
    CIRCLEQ_REMOVE(&ready_list, process, ready);
}

/* schedule next ready process (or go idle if no procesess ready) */
void schedule()
{
    struct process_state_s *process;

    /* no process running */
    if (current_process == NULL) {
        /* if any process ready then execute, otherwise go idle */
        if (!CIRCLEQ_EMPTY(&ready_list)) {
            process = CIRCLEQ_FIRST(&ready_list);
            current_process = process;
            load_process(process->i);
        } else {
            current_process = IDLE;
            load_process(1);
        }
    /* if we are idle, check for new processes */
    } else if (current_process == IDLE) {
        if (!CIRCLEQ_EMPTY(&ready_list)) {
            process = CIRCLEQ_FIRST(&ready_list);
            current_process = process;
            load_process(process->i);
        }
    /* if there are more than 1 process ready */
    } else if (CIRCLEQ_NEXT(current_process, ready) !=
               CIRCLEQ_PREV(current_process, ready)) {
        current_process = CIRCLEQ_NEXT(current_process, ready);
        load_process(current_process->i);
    }
}
