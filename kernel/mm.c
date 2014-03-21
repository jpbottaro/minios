#include <minios/scall.h>
#include <minios/i386.h>
#include <minios/misc.h>
#include <minios/idt.h>
#include <minios/mm.h>
#include <sys/types.h>
#include "debug.h"
#include "vmm.h"
#include "pm.h"

extern void _pf_handler();

struct page_s pages[PAGES_LEN];

LIST_HEAD(free_pages_t, page_s) free_pages;

void mm_mem_add_reference(void *page)
{
    if (page < (void *) KERNEL_PAGES)
        debug_panic("mm_mem_add_reference: page under mem limit");
    if (page > (void *) MEM_LIMIT)
        debug_panic("mm_mem_add_reference: page over mem limit");

    ++pages[hash_page(page)].refcount;
}

void mm_mem_free_reference(void *page)
{
    if (page < (void *) KERNEL_PAGES)
        debug_panic("mm_mem_free_reference: page under mem limit");
    if (page > (void *) MEM_LIMIT)
        debug_panic("mm_mem_free_reference: page over mem limit");

    struct page_s *p = &pages[hash_page(page)];

    if (p->refcount <= 0)
      debug_panic("mm_mem_free_reference: page refcount <= 0");
    if (--p->refcount == 0)
        LIST_INSERT_HEAD(&free_pages, p, status);
}

mm_page *search_page(mm_page *dir, void *vir, int flag)
{
    mm_page *dir_entry;

    dir_entry = dir + ((u32_t) vir >> 22);

    /* table not present? */
    if (!(dir_entry->attr & MM_ATTR_P)) {
        if (flag == MM_NOCREAT)
            return NULL;
        void *table_page = mm_mem_alloc();
        *dir_entry = make_mm_entry_addr(table_page,
                MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
        mm_map_page(dir, table_page, table_page);
    }

    return (mm_page *) (dir_entry->base << 12) +
                       (((u32_t) vir >> 12) & 0x3FF);
}

void *mm_newpage(mm_page *page)
{
    mm_page *new_page = mm_mem_alloc();
    if (new_page != NULL) {
        *page = make_mm_entry_addr(new_page,
                MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
        tlbflush();
    }
    return new_page;
}

void *mm_copy_on_write(void *vir, mm_page *entry)
{
    char *old_page = (char *) (entry->base << 12);

    if (pages[hash_page(old_page)].refcount > 1) {
        /* create new page to replace old_page */
        if (mm_newpage(entry) == NULL)
            return NULL;

        /* copy the contents of old_page to the new page */
        mm_map_page((void *) rcr3(), 0, old_page);
        mymemcpy(vir, 0, PAGE_SIZE);
        mm_umap_page((void *) rcr3(), 0);

        /* decrease refcount of old_page */
        mm_mem_free_reference(old_page);
    } else {
        entry->attr |= MM_ATTR_RW;
    }

    return vir;
}

/* new handler for page fault to force exit of ring3 tasks who force it */
void pf_handler()
{
    mm_page *page, *res;

    if (current_process == NULL)
        isr14();

    res = NULL;
    page = search_page((void *) rcr3(), (void *) rcr2(), MM_NOCREAT);
    if (page != NULL && page->attr & MM_ATTR_US) {
        if (!(page->attr & MM_ATTR_P)) {
            if (page->attr & MM_VALID)
                res = mm_newpage(page);
            else if (page->attr & MM_VM)
                res = vmm_retrieve(page->base);

        } else if (!(page->attr & MM_ATTR_RW)) {
            if (!(page->attr & MM_SHARED))
                res = mm_copy_on_write((void *) rcr2(), page);
        }
    }

    if (res == NULL)
        debug_panic("pf_handler: segmentation fault");
}

/* init memory manager */
void mm_init()
{
    int i;

    LIST_INIT(&free_pages);
    for (i = PAGES_LEN - 1; i >= 0; --i) {
        pages[i].base = (void *) KERNEL_PAGES + i * PAGE_SIZE;
        LIST_INSERT_HEAD(&free_pages, &pages[i], status);
    }

    /* register sys calls */
    SCALL_REGISTER(45, sys_palloc);
    SCALL_REGISTER(46, sys_share_page);

    /* register page fault handler */
    idt_register(14, _pf_handler, DEFAULT_PL);

    /* enable paging (kernel is identity mapped from 0x0 to MEM_LIMIT) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);
}

/* alloc page */
void *mm_mem_alloc()
{
    u32_t *start, *end;
    struct page_s *page;

    page = LIST_FIRST(&free_pages);
    if (page == NULL) {
        page = vmm_free_page();
        if (page == NULL)
            debug_panic("mm_mem_alloc: no more free pages");
    }
    LIST_REMOVE(page, status);

    start = (u32_t *) page->base;
    end = start + PAGE_SIZE / 4;
    for (; start < end; start++)
        *start = 0;

    page->refcount = 1;

    return page->base;
}

/* map a page in the PDT */
void mm_map_page_attr(mm_page *dir, void *vir, void *phy, int attr)
{
    mm_page *table_entry;

    table_entry = search_page(dir, vir, MM_CREAT);
    *table_entry = make_mm_entry_addr(phy,  attr);
    tlbflush();
}

/* map a page in the PDT marked user, r/w, present */
void mm_map_page(mm_page *dir, void *vir, void *phy)
{
    mm_map_page_attr(dir, vir, phy, MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
}

/* umap a page in the PDT */
void mm_umap_page(mm_page *dir, void *vir)
{
    mm_page *table_entry;

    if ( (table_entry = search_page(dir, vir, MM_NOCREAT)) == NULL)
        debug_panic("mm_umap_page: page not mapped");
    table_entry->attr = 0;
    tlbflush();
}

/* make page directory table with first 0x0 to MEM_LIMIT ident mapping (kernel) */
mm_page* mm_dir_new()
{
    u32_t base;
    mm_page *dirbase   = (mm_page *) mm_mem_alloc();
    mm_page *tablebase = (mm_page *) mm_mem_alloc();

    /* 0111 means present, r/w and user */
    *dirbase = make_mm_entry_addr(tablebase, MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);

    /* mark first page as not present, to catch NULL pointers */
    *tablebase = make_mm_entry_addr(0, 0);
    tablebase++;

    for (base = PAGE_SIZE; base < MEM_LIMIT; tablebase++, base += PAGE_SIZE)
        *tablebase = make_mm_entry_addr(base, MM_ATTR_P | MM_ATTR_RW);

    /* ident map the directory tables (this way they're 'user' and not 'system') */
    mm_map_page(dirbase, dirbase, dirbase);
    mm_map_page(dirbase, tablebase, tablebase);

    return dirbase;
}

void copy_table_fork(mm_page *to, mm_page *from)
{
    mm_page *end = from + (PAGE_SIZE / sizeof(mm_page *));

    while (from < end) {
        if (!(from->attr & MM_SHARED))
            from->attr &= ~MM_ATTR_RW;
        *to = *from;
        if ((from->attr & (MM_ATTR_P | MM_ATTR_US)) == (MM_ATTR_P | MM_ATTR_US))
            mm_mem_add_reference((void *) (from->base << 12));
        else if (from->attr & MM_VM)
            vmm_mem_add_reference(from->base);
        to++;
        from++;
    }
}

/* copy a page directory table */
mm_page *mm_dir_cpy(mm_page *dir)
{
    mm_page *d, *ret, *end;
    mm_page *dirbase, *tablebase;

    ret = dirbase = (mm_page *) mm_mem_alloc();

    end = dir + (PAGE_SIZE / sizeof(mm_page *));
    for (d = dir; d < end; d++, dirbase++) {
        if (d->attr & MM_ATTR_P) {
            tablebase = (mm_page *) mm_mem_alloc();
            mm_map_page(dirbase, tablebase, tablebase);
            copy_table_fork(tablebase, (mm_page *) (d->base << 12));
            *dirbase = make_mm_entry_addr(tablebase, d->attr);
        } else {
            *dirbase = make_mm_entry_addr(0, 0);
        }
    }

    /* the stacks are special cases of non-shared pages that we want as RW, and
     * we replicate them manually later */
    d = search_page(dir, (char *) STACK_PAGE, MM_NOCREAT);
    d->attr |= MM_ATTR_RW;
    d = search_page(dir, (char *) KSTACK_PAGE, MM_NOCREAT);
    d->attr |= MM_ATTR_RW;

    return ret;
}

/* free directory page and all its present tables (recursive helper function) */
void mm_dir_table_free(mm_page *d, int recursive)
{
    mm_page *entry, *end;

    end = d + PAGE_SIZE / sizeof(mm_page);
    for (; d < end; ++d) {
        if (d->attr & MM_ATTR_P) {
            entry = (void *) (d->base << 12);
            if (recursive)
                mm_dir_table_free(entry, 0);
            else if (d->attr & MM_ATTR_US)
                mm_mem_free_reference(entry);
        } else if (d->attr & MM_VM) {
            vmm_mem_free_reference(entry->base);
        }
    }
}

/* free directory page and all its present tables */
void mm_dir_free(mm_page *d)
{
    mm_dir_table_free(d, 1);
}

/* build page and add it to the dirtable; optionally bootstrap */
void *mm_build_page(mm_page *dir, void *vir, void *boot_page)
{
    void *page = mm_mem_alloc();

    mm_map_page(dir, vir, page);
    if (boot_page != NULL) {
        mm_map_page((void *) rcr3(), 0, page);
        mymemcpy(0, boot_page, PAGE_SIZE);
        mm_umap_page((void *) rcr3(), 0);
    }

    return page;
}

/* return a 4kb page to a process (fix this, the last_mem thing sucks) */
void *sys_palloc()
{
    void *virt;

    virt = current_process->last_mem;
    current_process->last_mem += PAGE_SIZE;
    /* set bit to remember that this page is valid */
    mm_map_page_attr(current_process->pages_dir, virt, 0,
            MM_ATTR_RW | MM_ATTR_US | MM_VALID);

    return virt;
}

int sys_share_page(void *page)
{
    mm_page *entry;

    entry = search_page(current_process->pages_dir, page, MM_NOCREAT);
    if (entry == NULL)
        return -1;
    /* if we are going to share it, create it now so everybody knows it later */
    if (entry->attr & MM_VALID && !(entry->attr & MM_ATTR_P))
        mm_newpage(entry);
    entry->attr |= MM_SHARED;

    return 0;
}
