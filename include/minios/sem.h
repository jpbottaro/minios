#ifndef _SEM_H
#define _SEM_H

#include <tipos.h>

typedef sint_32 sem_id;

typedef struct str_sem_t {
	uint_32 vl; /* Free places      */
	uint_32 q;  /* Wait-event queue */
} sem_t;

#define SEM_NEW(val) (sem_t){.vl = (val), .q = (uint_32)-1}

void sem_wait(sem_t* s);
void sem_signal(sem_t* s);
void sem_broadcast(sem_t* s);

#endif
