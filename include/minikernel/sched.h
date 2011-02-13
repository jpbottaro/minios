#ifndef _SCHED_H
#define _SCHED_H

#include <sys/types.h>
#include <minikernel/fs.h>

#define MAX_PROCESSES 20

struct process_state_s {
    /* data for context switch */
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int ss;

    /* process id */
    unsigned int pid;

    /* process owner's uid and gid */
    unsigned int uid;
    unsigned int gid;

    /* fs data */
    struct file_s files[MAX_FILES];
    unsigned int curr_dir;
};

extern struct process_state_s ps[MAX_PROCESSES];
extern unsigned int current_process;

unsigned int current_uid();
unsigned int current_gid();

#endif /* _SCHED_H */
