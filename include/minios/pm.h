#ifndef _PM_H
#define _PM_H

#include <minios/fs.h>
#include <minios/mm.h>
#include <sys/types.h>

#define IDLE        (&ps[1])
#define IDLE_NUM    1
#define STACK_PAGE  0xFFFFF000
#define KSTACK_PAGE 0xFFFFE000
#define ARG_PAGE    0xFFFFD000
#define MAX_PROCESSES 20
#define MAX_ARG       10

struct process_state_s {
    /* number */
    int i;

    /* process id */
    pid_t pid;

    /* directory of pages */
    mm_page *pages_dir;

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

void pm_init();
void pm_switchto(u32_t process_num);

pid_t sys_newprocess(const char *filename, char *const argv[]);
pid_t sys_waitpid(pid_t pid, int *status, int options);
pid_t sys_getpid(void);
void sys_exit(int status);

#endif /* _PM_H */
