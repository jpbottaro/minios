#ifndef _SEM_H
#define _SEM_H

#include <minios/sched.h>
#include <sys/types.h>

typedef struct str_sem_t {
    unsigned int count;
    waiting_list_t wait;
} sem_t;

void sem_init(sem_t* s, unsigned int val);
void sem_wait(sem_t* s);
void sem_signal(sem_t* s);
void sem_broadcast(sem_t* s);

#endif
