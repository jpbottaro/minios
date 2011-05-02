#include <minios/mm.h>
#include <minios/misc.h>
#include <minios/i386.h>
#include <minios/panic.h>
#include <minios/sched.h>
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

/* create a new process, with binary filename, and arguments argv.
 * this is a merge of fork() and execv()
 */
pid_t sys_newprocess(const char *filename, char *const argv[])
{
    u32_t i, j, size;
    mm_page *dirbase;
    void *page, *stack;
    ino_t curr_dir, ino_num;
    struct inode_s *ino, *dir;
    struct process_state_s *process;
    char *tmpargv[MAX_ARG];

    /* get process entry for child */
    process = LIST_FIRST(&unused_list);
    if (process == NULL)
        panic("No space for new process in ps array!");

    curr_dir = (current_process == NULL) ? 1 : current_process->curr_dir;

    /* get inode of new process' exe */
    dir = get_inode(curr_dir);
    if ( (ino_num = find_inode(dir, filename, FS_SEARCH_GET)) == NO_INODE)
        return -1;
    ino = get_inode(ino_num);

    if (!IS_FILE(ino->i_mode))
        return -1;

    /* fill child entry */
    LIST_REMOVE(process, unused);
    process->run = 0;
    process->pid = pid++;
    process->uid = 1;
    process->gid = 1;
    process->waiting = NULL;
    process->parent = current_process;
    process->curr_dir = curr_dir;
    process->eip = CODE_OFFSET;
    process->esp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    process->ebp = STACK_PAGE + (PAGE_SIZE - C_PARAMS_SIZE);
    LIST_INIT(&process->pages_list);
    init_fds(process->i);

    /* build directory table for new process (with kernel already mapped) */
    dirbase = mm_dir_new();
    process->pages_dir = dirbase;
    /* ident map the directory table (this way its 'user' and not 'system') */
    mm_map_page(dirbase, dirbase, dirbase);

    /* build code */
    for (i = 0; i < ino->i_size; i += PAGE_SIZE) {
        /* get new page for code */
        page = mm_mem_alloc();
        add_process_page(process, page);

        /* temporary map page to temmem to be able to copy the code */
        mm_map_page((mm_page *) rcr3(), tmpmem, page);

        /* copy file from filesystem to the new page */
        size = MIN(PAGE_SIZE, ino->i_size - i);
        copy_file((char *) tmpmem, size, i, ino, FS_READ);

        /* add the new code page to the page directory table */
        mm_map_page(dirbase, (void *) (i + CODE_OFFSET), page);

        /* remove temporary ident mapping */
        mm_umap_page((mm_page *) rcr3(), tmpmem);
    }

    /* build user stack */
    stack = mm_mem_alloc();
    add_process_page(process, stack);
    mm_map_page(dirbase, (void *) STACK_PAGE, stack);

    /* build kernel stack (we are not using this since everything is ring 0) */
    page = mm_mem_alloc();
    add_process_page(process, page);
    mm_map_page(dirbase, (void *) KSTACK_PAGE, page);

    i = j = 0;
    if (argv != NULL) {
        /* add argc and argv to the new process (lets hope it only takes 1 page) */
        page = mm_mem_alloc();
        add_process_page(process, page);
        mm_map_page(dirbase, (void *) ARG_PAGE, page);
        mm_map_page((mm_page *) rcr3(), (void *) tmpmem, page);

        /* put the arguments in the new page */
        for (i = j = 0; i < MAX_ARG - 1 && argv[i] != NULL; ++i) {
            tmpargv[i] = (char *) (ARG_PAGE + j);
            size = mystrlen(argv[i]);
            mymemcpy(tmpmem + j, argv[i], size + 1);
            j += size + 1;
        }
        /* copy the tmpargv array of pointers to the arguments */
        tmpargv[i] = NULL;
        mymemcpy((char *) tmpmem + j, (char *) tmpargv, (i + 1) * 4);
        mm_umap_page((mm_page *) rcr3(), tmpmem);
    }
    /* add the values to the stack so that main() can get them */
    mm_map_page((mm_page *) rcr3(), tmpmem, stack);
    *((unsigned int *) (tmpmem + 0xFFC)) = ARG_PAGE + j;
    *((unsigned int *) (tmpmem + 0xFF8)) = i;
    *((unsigned int *) (tmpmem + 0xFF4)) = 0;
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
    return current_process->uid;
}

unsigned int current_gid()
{
    return current_process->gid;
}
