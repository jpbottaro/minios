#include <minikernel/misc.h>
#include "sched.h"
#include "tss.h"
#include "mmu.h"
#include "i386.h"

#define IDLE (&ps[1])

struct process_state_s ps[MAX_PROCESSES];
struct process_state_s *current_process;
CIRCLEQ_HEAD(sched_list_t, process_state_s) sched_list;
LIST_HEAD(unused_list_t, process_state_s) unused_list;
unsigned int pid;

/* init_timer taken from James Molly in
 * http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs%20and%20the%20PIT.html
 */
void init_timer(unsigned int frequency)
{
   /* The value we send to the PIT is the value to divide it's input clock
    * (1193180 Hz) by, to get our required frequency. Important to note is
    * that the divisor must be small enough to fit into 16-bits.
    */
   unsigned int divisor = 1193180 / frequency;

   /* Send the command byte */
   outb(0x43, 0x36);

   /* Divisor has to be sent byte-wise, so split here into upper/lower bytes */
   unsigned char l = (unsigned char) (divisor & 0xFF);
   unsigned char h = (unsigned char) ( (divisor>>8) & 0xFF );

   /* Send the frequency divisor */
   outb(0x40, l);
   outb(0x40, h);
}

void init_scheduler()
{
    int i;
    struct process_state_s *process;

    /* init unused process entries list */
    i = 1;
    process = &ps[1];
    LIST_INIT(&unused_list);
    for (; process < &ps[MAX_PROCESSES]; ++process, ++i) {
        process->i = i;
        process->pid = 0;
        LIST_INSERT_HEAD(&unused_list, process, unused);
    }

    /* init scheduler's list */
    CIRCLEQ_INIT(&sched_list);

    /* init tss array */
    init_tss();

    /* add idle task */
    process = IDLE;
    process->i = 1;
    process->pid = 1;
    process->uid = 0;
    process->gid = 0;
    add_idle(1);

    current_process = NULL;
    pid = 2;
}

void schedule()
{
    struct process_state_s *process;

    /* no process running */
    if (current_process == NULL) {
        /* if any process ready then execute, otherwise go idle */
        if (!CIRCLEQ_EMPTY(&sched_list)) {
            process = CIRCLEQ_FIRST(&sched_list);
            current_process = process;
            load_process(process->i);
        } else {
            current_process = IDLE;
            load_process(1);
        }
    /* if we are idle, check for new processes */
    } else if (current_process == IDLE) {
        if (!CIRCLEQ_EMPTY(&sched_list)) {
            process = CIRCLEQ_FIRST(&sched_list);
            current_process = process;
            load_process(process->i);
        }
    /* if there are more than 1 process ready */
    } else if (CIRCLEQ_NEXT(current_process, schedule) !=
               CIRCLEQ_PREV(current_process, schedule)) {
        current_process = CIRCLEQ_NEXT(current_process, schedule);
        load_process(current_process->i);
    }
}

void sys_exit(int status)
{
    struct process_state_s *parent = current_process->parent;

    /* schedule a wake up if parent was waiting */
    if (parent != NULL &&
        (parent->waiting == current_process || parent->waiting == parent)) {
        parent->waiting = NULL;
        *(parent->status) = status;
        parent->child_pid = current_process->pid;
        CIRCLEQ_INSERT_HEAD(&sched_list, parent, schedule);
    }

    current_process->pid = 0;
    LIST_INSERT_HEAD(&unused_list, current_process, unused);
    CIRCLEQ_REMOVE(&sched_list, current_process, schedule);

    current_process = NULL;
    schedule();
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

    /* remove from list */
    CIRCLEQ_REMOVE(&sched_list, current_process, schedule);
    current_process = NULL;
    schedule();

    return current_process->child_pid;
}

pid_t sys_newprocess(const char *filename)
{
    unsigned int i, size, page, dirbase;
    struct process_state_s *process;
    struct inode_s *ino, *dir;
    ino_t curr_dir, ino_num;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        return -1;

    curr_dir = (current_process == NULL) ? 1 : current_process->curr_dir;

    /* get inode of new process' exe */
    dir = get_inode(curr_dir);
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
    process->curr_dir = curr_dir;
    init_fds(process->i);

    /* build directory table for new process (with kernel already mapped) */
    dirbase = init_directory();

    /* build user stack */
    page = new_page();
    map_page(0xFFFFFFFF, dirbase, page);

    /* build kernel stack (we are not using this since everithing is ring 0) */
    page = new_page();
    map_page(0xFFFFEFFF, dirbase, page);

    /* build code */
    for (i = 0; i < ino->i_size; i += PAGE_SIZE) {
        /* get new page for code */
        page = new_page();

        /* temporary map page to 0 to be able to copy the code */
        map_page(0x0, rcr3(), page);

        /* copy file from filesystem to the new page */
        size = MIN(PAGE_SIZE, ino->i_size - i);
        copy_file((char *) 0x0, size, i, ino, FS_READ);

        /* add the new code page to the page directory table */
        map_page(i + CODE_OFFSET, dirbase, page);

        /* remove temporary ident mapping */
        umap_page(0x0, rcr3());
    }
    add_tss(process->i, dirbase);

    /* add to scheduler */
    CIRCLEQ_INSERT_HEAD(&sched_list, process, schedule);

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
