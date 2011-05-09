#include <minios/i386.h>
#include <minios/sem.h>

void sem_init(sem_t* s, unsigned int val)
{
    s->count = val;
    LIST_INIT(&s->wait);
}

/* implementation based on
 * http://stackoverflow.com/questions/5839315/
 * can-i-switch-the-test-and-modification-part-in-wait-signal-semaphore */
void sem_wait(sem_t* s)
{
    while (1) {
        long count = s->count;
        if (count > 0) {
            if (cmpxchg(&s->count, count, count - 1))
                return;
            else
                continue;
        } else {
            sched_block(current_process, &s->wait);
            if (s->count == 0) sched_schedule(1);
            sched_unblock(current_process, &s->wait);
        }
    }
}

void sem_signal(sem_t* s)
{
    s->count++;
    sched_unblock(NULL, &s->wait);
}

void sem_broadcast(sem_t* s)
{

}
