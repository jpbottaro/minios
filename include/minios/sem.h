#ifndef _SEM_H
#define _SEM_H

#include <minios/sched.h>
#include <sys/types.h>

typedef struct str_sem_t {
    unsigned int count;
    waiting_list_t wait;
} sem_t;

#define SEM_NEW(val) (sem_t){.vl = (val), .q = (uint_32)-1}

void sem_wait(sem_t* s);
void sem_signal(sem_t* s);
void sem_broadcast(sem_t* s);

#endif
