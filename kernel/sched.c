#include <minikernel/misc.h>
#include "sched.h"
#include "tss.h"
#include "mmu.h"
#include "i386.h"

struct process_state_s ps[MAX_PROCESSES];
struct process_state_s *current_process;
CIRCLEQ_HEAD(sched_list_t, process_state_s) sched_list;
LIST_HEAD(unused_list_t, process_state_s) unused_list;
unsigned int pid;

void init_scheduler()
{
    int i;
    struct process_state_s *process;

    /* init unused process entries list */
    i = 1;
    process = &ps[2];
    LIST_INIT(&unused_list);
    for (; process < &ps[MAX_PROCESSES]; ++process) {
        process->i = i;
        process->pid = 0;
        LIST_INSERT_HEAD(&unused_list, process, unused);
    }

    /* init scheduler's list */
    CIRCLEQ_INIT(&sched_list);

    /* init tss array */
    init_tss();

    /* add idle task */
    process = &ps[1];
    process->i = 1;
    process->pid = 1;
    process->uid = 0;
    process->gid = 0;
    CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);
    add_idle(1);

    current_process = NULL;
    pid = 2;
}

void schedule()
{
    /* first time we do schedule? go idle */
    if (current_process == NULL) {
        current_process = &ps[1];
        load_process(1);
    /* only do this if there are more than 1 process ready */
    } else if (current_process->schedule.cqe_next != current_process) {
        current_process = current_process->schedule.cqe_next;
        load_process(current_process->i);
    }
}

void sys_exit(int status)
{
    struct process_state_s *parent = current_process->parent;

    /* schedule a wake up if parent was waiting */
    if (parent->waiting == current_process || parent->waiting == parent) {
        parent->waiting = NULL;
        *(parent->status) = status;
        parent->child_pid = current_process->pid;
        CIRCLEQ_INSERT_HEAD(&sched_list, parent, schedule);
    }

    current_process->pid = 0;
    LIST_INSERT_HEAD(&unused_list, current_process, unused);
    CIRCLEQ_REMOVE(&sched_list, current_process, schedule);
}

/* find process by pid. bruteforce!! probably would be better if parent would
 * know his childs...
 */
struct process_state_s *find_pid(pid_t pid)
{
    struct process_state_s *process;
    
    for (process = &ps[0]; process < &ps[MAX_PROCESSES]; ++process)
        if (process->pid == pid)
            return process;
    return NULL;
}

pid_t sys_waitpid(pid_t pid, int *status, int options)
{
    struct process_state_s *process;

    if (pid == -1) {
        /* XXX should be error if process has no childs */
        process = current_process;
    } else {
        process = find_pid(pid);

        /* process does not exists (already finished or never existed) */
        if (process == NULL)
            return -1;

        /* only wait a child process */
        if (process->parent != current_process)
            return -1;
    }

    *status = -1;
    current_process->waiting = process;
    current_process->status = status;
    current_process->child_pid = -1;

    /* remove from list, making sure we dont break schedule() */
    /* XXX trouble if only active process is idle, fix schedule() */
    current_process = current_process->schedule.cqe_next;
    CIRCLEQ_REMOVE(&sched_list, current_process, schedule);

    return current_process->child_pid;
}

pid_t sys_newprocess(const char *filename, char *const argvp[])
{
    unsigned int i, size, page, dirbase;
    struct process_state_s *process;
    struct inode_s *ino, *dir;
    ino_t ino_num;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        return -1;

    /* get inode of new process' exe */
    dir = get_inode(current_process->curr_dir);
    if ( (ino_num = find_inode(dir, filename, FS_SEARCH_GET)) == NO_INODE)
            return -1;
    ino = get_inode(ino_num);

    /* fill child entry */
    LIST_REMOVE(process, unused);
    process->pid = pid++;
    process->uid = 0;
    process->gid = 0;
    process->waiting = NULL;
    process->parent = current_process;
    process->curr_dir = current_process->curr_dir;
    init_fds(process->i);

    /* build directory table for new process */
    dirbase = new_page();

    /* build stack */
    page = new_page();
    map_page(0xFFFFFFFF, dirbase, page);

    /* build code */
    for (i = 0; i < ino->i_size; i += PAGE_SIZE) {
        /* get new page for code */
        page = new_page();

        /* temporary ident mapping to be able to copy the code */
        map_page(page, rcr3(), page);

        /* copy file from filesystem to the new page */
        size = MIN(PAGE_SIZE, ino->i_size - i);
        copy_file((char *) page, size, i, ino, FS_READ);

        /* add the new code page to the page directory table */
        map_page(i, dirbase, page);
        
        /* remove temporary ident mapping */
        umap_page(page, rcr3());
    }
    add_tss(process->i, dirbase);

    /* add to scheduler */
    CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);

    /* make child come back to this point */
    return process->pid;
}

pid_t sys_getpid(void)
{
    return current_process->pid;
}

unsigned int current_uid()
{
    return current_process->uid;
}

unsigned int current_gid()
{
    return current_process->gid;
}
