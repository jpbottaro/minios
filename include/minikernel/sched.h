#ifndef _SCHED_H
#define _SCHED_H

#include <sys/types.h>
#include <minikernel/fs.h>

#define MAX_PROCESSES 20

/* this is where processes start, not at 0x0 */
#define CODE_OFFSET 0x400000

struct process_state_s {
    /* number */
    int i;

    /* process id */
    pid_t pid;

    /* parent */
    struct process_state_s *parent;

    /* process owner's uid and gid */
    pid_t uid;
    pid_t gid;

    /* data to keep track of waitpid sys call */
    struct process_state_s *waiting;
    int *status;
    pid_t child_pid;

    /* fs data */
    struct unused_fd_t unused_fd;
    struct file_s files[MAX_FILES];
    unsigned int curr_dir;

    /* dev io data */
    unsigned int dev;

    /* scheduler ready list pointers */
    CIRCLEQ_ENTRY(process_state_s) ready;

    /* scheduler waiting list pointers */
    LIST_ENTRY(process_state_s) wait;

    /* unused list pointers */
    LIST_ENTRY(process_state_s) unused;

    /* list of used pages */
    LIST_HEAD(pages_list_t, page_s) pages_list;
} __attribute__((__packed__)) ;

extern struct process_state_s ps[MAX_PROCESSES];
extern struct process_state_s *current_process;

unsigned int current_uid();
unsigned int current_gid();

void unblock_process(unsigned int dev);
void block_process(struct process_state_s *process, unsigned int dev);

void sys_exit(int status);
pid_t sys_newprocess(const char *filename, char *const argv[]);
pid_t sys_waitpid(pid_t pid, int *status, int options);
pid_t sys_getpid(void);

#endif /* _SCHED_H */
