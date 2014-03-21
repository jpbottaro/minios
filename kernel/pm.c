#include <minios/pso_file.h>
#include <minios/sched.h>
#include <minios/scall.h>
#include <minios/misc.h>
#include <minios/i386.h>
#include <minios/mm.h>
#include <minios/fs.h>
#include <unistd.h>
#include <fcntl.h>
#include "debug.h"
#include "tss.h"
#include "gdt.h"
#include "pm.h"

LIST_HEAD(unused_list_t, process_state_s) unused_list;
struct process_state_s ps[MAX_PROCESSES];
unsigned int pid = 2;

/* label from kernel.asm, just a jmp $ */
extern void idle();

extern char KSTACK;
extern u32_t KSTACKSIZE;
extern u32_t idle_startpoint;

void add_idle()
{
    struct process_state_s *process;

    process = IDLE;
    process->run = 0;
    process->pid = 1;
    process->uid = 1;
    process->gid = 1;
    process->pages_dir = (mm_page *) rcr3();
    process->parent = NULL;

    process->eip = (u32_t) &idle_startpoint;
    process->esp = (u32_t) (&KSTACK + KSTACKSIZE - 0x10);
    process->ebp = (u32_t) (&KSTACK + KSTACKSIZE - 0x10);
}

/* initialize process manager, list of processes, tss, and add idle */
void pm_init()
{
    int i;
    struct process_state_s *process;

    /* init unused process entries list */
    i = 2;
    process = &ps[2];
    LIST_INIT(&unused_list);
    for (; process < &ps[MAX_PROCESSES]; ++process, ++i) {
        process->i = i;
        process->pid = 0;
        LIST_INSERT_HEAD(&unused_list, process, unused);
    }

    /* init needed tss */
    tss_init();

    /* add idle task */
    add_idle();

    /* register sys calls */
    SCALL_REGISTER(1, sys_exit);
    SCALL_REGISTER(2, sys_fork);
    SCALL_REGISTER(7, sys_waitpid);
    SCALL_REGISTER(11, sys_newprocess);
    SCALL_REGISTER(20, sys_getpid);
}

extern void __pm_switchto(struct process_state_s *);

/* do a context switch to process number 'process_num' */
void pm_switchto(u32_t process_num)
{
    __pm_switchto(&ps[process_num]);
}

/* end a process, wake parent if needed, free its pages and reschedule */
void sys_exit(int status)
{
    if (current_process == NULL)
        debug_panic("sys_exit: trying to exit with NULL current_process");

    struct process_state_s *parent = current_process->parent;

    /* schedule a wake up if parent was waiting */
    if (parent != NULL &&
       (parent->waiting == current_process || parent->waiting == parent)) {
        parent->waiting = NULL;
        if (parent->status != NULL)
            *(parent->status) = status;
        parent->child_pid = current_process->pid;
        sched_enqueue(parent);
    }

    /* free process pages */
    mm_dir_free(current_process->pages_dir);

    /* release inodes */
    fs_closeall();
    release_inode(current_process->curr_dir);

    /* delete process */
    current_process->pid = 0;
    LIST_INSERT_HEAD(&unused_list, current_process, unused);
    sched_unqueue(current_process);

    current_process = NULL;
    sched_schedule(0);
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

/* block process until child with 'pid' exits; return its value in status */
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

    current_process->waiting = process;
    current_process->status = status;
    current_process->child_pid = -1;

    if (status != NULL)
        *status = -1;

    /* remove from list */
    sched_unqueue(current_process);
    sched_schedule(1);

    return current_process->child_pid;
}

extern unsigned int reip();

pid_t sys_fork()
{
    mm_page *dirbase;
    struct process_state_s *process;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        debug_panic("newprocess: No space for new process in ps array!");

    /* fill child entry */
    LIST_REMOVE(process, unused);
    process->run = 1;
    process->pid = pid++;
    process->uid = current_process->uid;
    process->gid = current_process->gid;
    process->waiting = NULL;
    process->parent = current_process;
    process->curr_dir = current_dir();
    process->last_mem = current_process->last_mem;

    /* copy fds */
    init_fds(process->i);

    /* copy the page directory */
    dirbase = mm_dir_cpy(current_process->pages_dir);
    process->pages_dir = dirbase;

    /* add to scheduler */
    sched_enqueue(process);

    /* this _has_ to be at the end.. its kinda ugly, but that ship has sailed */
    process->esp = resp();
    process->ebp = rebp();
    process->eip = reip();

    if (current_process == process)
        return process->pid;
    else
        return 0;
}

/* create a new process, with binary filename, and arguments argv.
 * this is a merge of fork() and execv()
 */
pid_t sys_newprocess(const char *filename, char *const argv[])
{
    int fd;
    int i, j, size, max, ret, len;
    pso_file pso_header;
    mm_page *dirbase;
    char *code_start, *tmpargv[MAX_ARG];
    struct process_state_s *process;
    void *page;

    /* open target file */
    fd = 0;
    if ( (fd = fs_open(filename, O_RDONLY, 0)) < 0)
        return -1;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        debug_panic("newprocess: No space for new process in ps array!");

    /* how much did read */
    len = 0;

    /* get header */
    if ( (ret = sys_read(fd, (char *) &pso_header, PSO_SIZE)) != PSO_SIZE)
        goto err;
    len += ret;

    if (mystrncmp((char *) pso_header.signature, "PSO", 3) != 0)
        goto err;

    /* fill child entry */
    LIST_REMOVE(process, unused);
    process->run = 0;
    process->pid = pid++;
    process->uid = 2;
    process->gid = 2;
    process->waiting = NULL;
    process->parent = current_process;
    process->curr_dir = current_dir();
    process->last_mem = (char *) REQUESTED_MEMORY_START;
    process->eip = pso_header._main;
    process->esp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    process->ebp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    init_fds(process->i);

    /* build directory table for new process (with kernel already mapped) */
    dirbase = mm_dir_new();
    process->pages_dir = dirbase;

    /* build code */
    sys_lseek(fd, 0, SEEK_SET);
    code_start = (char *) pso_header.mem_start;
    max = pso_header.mem_end_disk - pso_header.mem_start;
    for (i = 0; i < max; i += PAGE_SIZE) {
        /* get new page for code */
        page = mm_build_page(dirbase, code_start + i);

        /* copy file from filesystem to the new page */
        if ( (ret = sys_read(fd, page, PAGE_SIZE)) < 0)
            debug_panic("newprocess: error in sys_read, cant get program");

        if (ret < PAGE_SIZE)
            break;
    }

    /* close file */
    sys_close(fd);

    i = j = 0;
    page = mm_translate(dirbase, (char *) ARG_PAGE);
    if (argv != NULL) {
        /* put the arguments in the new page */
        for (i = j = 0; i < MAX_ARG - 1 && argv[i] != NULL; ++i) {
            tmpargv[i] = (char *) (ARG_PAGE + j);
            size = mystrlen(argv[i]);
            mymemcpy(page + j, argv[i], size + 1);
            j += size + 1;
        }

        /* copy the tmpargv array of pointers to the arguments */
        tmpargv[i] = NULL;
        mymemcpy(page + j, (char *) tmpargv, (i + 1) * 4);
    }

    /* add the values to the stack so that main() can get them */
    page = mm_translate(dirbase, (char *) STACK_PAGE);
    *((unsigned int *) (page + 0xFFC)) = ARG_PAGE + j;
    *((unsigned int *) (page + 0xFF8)) = i;

    /* add to scheduler */
    sched_enqueue(process);

    return process->pid;

err:
    if (fd)
        sys_close(fd);
    return -1;
}

/* get current process' pid */
pid_t sys_getpid(void)
{
    return current_process->pid;
}

unsigned int current_uid()
{
    if (current_process == NULL)
        return 0;
    return current_process->uid;
}

unsigned int current_gid()
{
    if (current_process == NULL)
        return 0;
    return current_process->gid;
}

unsigned int current_pid()
{
    if (current_process == NULL)
        return 0;
    return current_process->pid;
}
