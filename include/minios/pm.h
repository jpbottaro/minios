#ifndef _PM_H
#define _PM_H

#include <minios/fs.h>
#include <minios/mm.h>
#include <sys/types.h>

#define IDLE          (&ps[1])
#define IDLE_NUM      1
#define STACK_PAGE    0xFFFFF000
#define KSTACK_PAGE   0xFFFFE000
#define ARG_PAGE      0xFFFFD000
#define EFLAGS_MASK   0x00000202
#define MAX_ARG       10
#define MAX_PROCESSES 20
#define REQUESTED_MEMORY_START 0xE0000000

struct process_state_s {
    /* number */
    int i;

    /* process id */
    pid_t pid;

    /* run = 0 means the process was never run */
    u32_t run;

    /* process data */
    u32_t eip;
    u32_t esp;
    u32_t ebp;

    /* directory of pages */
    mm_page *pages_dir;

    /* parent */
    struct process_state_s *parent;

    /* process owner's uid and gid */
    pid_t uid;
    pid_t gid;

    /* where to map requested memory */
    char *last_mem;
    void *stack;
    void *kstack;

    /* data to keep track of waitpid sys call */
    struct process_state_s *waiting;
    int *status;
    pid_t child_pid;

    /* fs data */
    struct unused_fd_t unused_fd;
    struct file_s files[MAX_FILES];
    struct inode_s *curr_dir;

    /* scheduler ready list pointers */
    CIRCLEQ_ENTRY(process_state_s) ready;

    /* scheduler waiting list pointers */
    LIST_ENTRY(process_state_s) wait;

    /* unused list pointers */
    LIST_ENTRY(process_state_s) unused;
} __attribute__((__packed__)) ;

extern struct process_state_s ps[MAX_PROCESSES];
extern struct process_state_s *current_process;

unsigned int current_pid();
unsigned int current_uid();
unsigned int current_gid();

void pm_init();
void pm_switchto(u32_t process_num);

pid_t sys_fork();
pid_t sys_newprocess(const char *filename, char *const argv[]);
pid_t sys_waitpid(pid_t pid, int *status, int options);
pid_t sys_getpid(void);
void sys_exit(int status);

#endif /* _PM_H */
