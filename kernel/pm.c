#include <minios/pso_file.h>
#include <minios/sched.h>
#include <minios/scall.h>
#include <minios/misc.h>
#include <minios/i386.h>
#include <minios/idt.h>
#include <minios/mm.h>
#include <minios/fs.h>
#include <fcntl.h>
#include "debug.h"
#include "tss.h"
#include "gdt.h"
#include "pm.h"

/* this page resides in the ident. mapped kernel, used to copy things */
char tmpmem[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

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
    LIST_INIT(&process->pages_list);

    process->eip = (u32_t) &idle_startpoint;
    process->esp = (u32_t) (&KSTACK + KSTACKSIZE - 0x10);
    process->ebp = (u32_t) (&KSTACK + KSTACKSIZE - 0x10);
}

/* new handler for page fault to force exit of ring3 tasks who force it */
void pf_handler()
{
    if (current_process == NULL || current_process->uid == 1) {
        isr14();
    } else {
        sys_exit(-1);
    }
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

    /* register page fault handler */
    //idt_register(14, pf_handler, DEFAULT_PL);
}

/* do a context switch to process number 'process_num' */
void pm_switchto(u32_t process_num)
{
    if (last_process != NULL)
        save_process_state(last_process);
    load_process_state(&ps[process_num]);
}

/* add a page to the processes' list of used pages */
void add_process_page(struct process_state_s *process, void *page)
{
    LIST_INSERT_HEAD(&process->pages_list, &pages[hash_page(page)], status);
}

/* free all pages assigned to a process */
void free_all_pages(u32_t process_num)
{
    struct page_s *p, *n;
    LIST_FOREACH_SAFE(p, &ps[process_num].pages_list, status, n)
        mm_mem_free(p->base);
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
    free_all_pages(current_process->i);
    mm_dir_free(current_process->pages_dir);
    mm_mem_free(current_process->stack);
    mm_mem_free(current_process->kstack);

    /* release inodes */
    fs_closeall(current_process->files);
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

pid_t sys_fork()
{
    mm_page *dirbase;
    void *page, *stack;
    struct process_state_s *process;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        debug_panic("newprocess: No space for new process in ps array!");

    /* fill child entry */
    LIST_REMOVE(process, unused);
    process->run = 0;
    process->pid = pid++;
    process->uid = current_process->uid;
    process->gid = current_process->gid;
    process->waiting = NULL;
    process->parent = current_process;
    process->curr_dir = current_dir();
    process->last_mem = current_process->last_mem;
    process->esp = current_process->esp;
    process->ebp = current_process->ebp;
    process->eip = 0;
    LIST_INIT(&process->pages_list);

    /* copy fds */
    fs_fd_cpy(current_process->i, process->i);

    /* copy the page directory */
    dirbase = mm_dir_cpy(current_process->pages_dir);
    process->pages_dir = dirbase;

    /* build user stack */
    stack = mm_mem_alloc();
    process->stack = stack;
    mm_map_page(dirbase, (void *) STACK_PAGE, stack);
    mymemcpy((char *) STACK_PAGE, stack, PAGE_SIZE);

    /* build kernel stack */
    page = mm_mem_alloc();
    process->kstack = page;
    mm_map_page(dirbase, (void *) KSTACK_PAGE, page);
    mymemcpy((char *) KSTACK_PAGE, page, PAGE_SIZE);

    /* add to scheduler */
    sched_enqueue(process);

    return process->pid;
}

/* create a new process, with binary filename, and arguments argv.
 * this is a merge of fork() and execv()
 */
pid_t sys_newprocess(const char *filename, char *const argv[])
{
    int fd;
    int i, j, size, max, ret, len;
    u32_t mem_offset;
    pso_file pso_header;
    mm_page *dirbase;
    void *page, *stack;
    char *tmpargv[MAX_ARG];
    struct process_state_s *process;

    /* open target file */
    if ( (fd = fs_open(filename, O_RDONLY, 0)) < 0)
        return -1;

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        debug_panic("newprocess: No space for new process in ps array!");

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
    process->esp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    process->ebp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    LIST_INIT(&process->pages_list);
    init_fds(process->i);

    /* build directory table for new process (with kernel already mapped) */
    dirbase = mm_dir_new();
    process->pages_dir = dirbase;
    /* ident map the directory table (this way its 'user' and not 'system') */
    mm_map_page(dirbase, dirbase, dirbase);

    /* how much did read */
    len = 0;

    /* get header */
    if ( (ret = sys_read(fd, (char *) &pso_header, PSO_SIZE)) != PSO_SIZE)
        debug_panic("newprocess: error in sys_read, cant get header");
    len += ret;

    if (mystrncmp((char *) pso_header.signature, "PSO", 3) != 0)
        debug_panic("newprocess: see what to do with tasks with wrong magic");
    mem_offset = pso_header.mem_start;
    process->eip = pso_header._main;

    /* build code */
    max = pso_header.mem_end_disk - pso_header.mem_start;

    /* yak.. first page outside the loop, to add the pso_header too */
    page = mm_mem_alloc();
    add_process_page(process, page);
    mm_map_page((mm_page *) rcr3(), tmpmem, page);
    mymemcpy(tmpmem, (char *) &pso_header, PSO_SIZE);
    if ( (ret = sys_read(fd, tmpmem + PSO_SIZE, PAGE_SIZE - PSO_SIZE)) < 0)
        debug_panic("newprocess: error in sys_read, cant get program");
    len += ret;
    
    mm_map_page(dirbase, (void *) mem_offset, page);
    mm_umap_page((mm_page *) rcr3(), tmpmem);

    for (i = PAGE_SIZE; i < max; i += PAGE_SIZE) {
        /* get new page for code */
        page = mm_mem_alloc();
        add_process_page(process, page);

        /* temporary map page to tmpmem to be able to copy the code */
        mm_map_page((mm_page *) rcr3(), tmpmem, page);

        /* copy file from filesystem to the new page */
        if ( (ret = sys_read(fd, tmpmem, PAGE_SIZE)) < 0)
            debug_panic("newprocess: error in sys_read, cant get program");
        len += ret;

        /* add the new code page to the page directory table */
        mm_map_page(dirbase, (void *) (i + mem_offset), page);

        /* remove temporary ident mapping */
        mm_umap_page((mm_page *) rcr3(), tmpmem);

        if (ret == 0)
            break;
    }

    /* XXX for some reason, the apps dont have their size right in their
     * headers (much bigger), so skip this test for now
     */
    /* 
    if (len != max)
        debug_panic("newprocess: coudn't read enough! the program size does not "
                    "match what the sys_read() call gave us");
    */

    /* close file */
    fs_close(fd);

    /* reserve uninitialized space */
    max = pso_header.mem_end - pso_header.mem_start;
    for (; i < max; i += PAGE_SIZE) {
        /* get new page for code */
        page = mm_mem_alloc();
        add_process_page(process, page);

        /* add the new code page to the page directory table */
        mm_map_page(dirbase, (void *) (i + mem_offset), page);
    }

    /* build user stack */
    stack = mm_mem_alloc();
    process->stack = stack;
    mm_map_page(dirbase, (void *) STACK_PAGE, stack);

    /* build kernel stack */
    page = mm_mem_alloc();
    process->kstack = page;
    mm_map_page(dirbase, (void *) KSTACK_PAGE, page);

    i = j = 0;
    if (argv != NULL) {
        /* add argc and argv to the new process (lets hope it only takes 1 page) */
        page = mm_mem_alloc();
        add_process_page(process, page);
        mm_map_page(dirbase, (void *) ARG_PAGE, page);
        mm_map_page((mm_page *) rcr3(), tmpmem, page);

        /* put the arguments in the new page */
        for (i = j = 0; i < MAX_ARG - 1 && argv[i] != NULL; ++i) {
            tmpargv[i] = (char *) (ARG_PAGE + j);
            size = mystrlen(argv[i]);
            mymemcpy(tmpmem + j, argv[i], size + 1);
            j += size + 1;
        }
        /* copy the tmpargv array of pointers to the arguments */
        tmpargv[i] = NULL;
        mymemcpy(tmpmem + j, (char *) tmpargv, (i + 1) * 4);
        mm_umap_page((mm_page *) rcr3(), tmpmem);
    }
    /* add the values to the stack so that main() can get them */
    mm_map_page((mm_page *) rcr3(), tmpmem, stack);
    *((unsigned int *) (tmpmem + 0xFFC)) = ARG_PAGE + j;
    *((unsigned int *) (tmpmem + 0xFF8)) = i;
    //*((unsigned int *) (tmpmem + 0xFF4)) = 0;
    mm_umap_page((mm_page *) rcr3(), tmpmem);

    /* add to scheduler */
    sched_enqueue(process);

    return process->pid;
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
