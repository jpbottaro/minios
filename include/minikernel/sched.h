#ifndef _SCHED_H
#define _SCHED_H

#include <sys/types.h>
#include <minikernel/fs.h>

#define MAX_PROCESSES 20

struct process_state_s {
    /* number */
    int i;

    /* process id */
    unsigned int pid;

    /* process owner's uid and gid */
    unsigned int uid;
    unsigned int gid;

    /* fs data */
    struct file_s files[MAX_FILES];
    unsigned int curr_dir;

    /* scheduler list */
    CIRCLEQ_ENTRY(process_state_s) schedule;

    /* scheduler list */
    LIST_ENTRY(process_state_s) unused;
} __attribute__((__packed__)) ;

extern struct process_state_s ps[MAX_PROCESSES];
extern struct process_state_s *current_process;

unsigned int current_uid();
unsigned int current_gid();

#endif /* _SCHED_H */
