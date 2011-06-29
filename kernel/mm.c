#include <minios/debug.h>
#include <minios/scall.h>
#include <minios/i386.h>
#include <minios/misc.h>
#include <minios/mm.h>
#include <sys/types.h>
#include "pm.h"

struct page_s pages[PAGES_LEN];

LIST_HEAD(free_pages_t, page_s) free_pages;

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
}

/* alloc page */
void *mm_mem_alloc()
{
    u32_t *start, *end;
    struct page_s *page;

    page = LIST_FIRST(&free_pages);
    if (page == NULL)
        debug_panic("mm_mem_alloc: no more free pages");
    LIST_REMOVE(page, status);

    start = (u32_t *) page->base;
    end = start + PAGE_SIZE / 4;
    for (; start < end; start++)
        *start = 0;

    return page->base;
}

/* free page */
void mm_mem_free(void *page)
{
    if (page < (void *) KERNEL_PAGES || page > (void *) CODE_OFFSET)
        debug_panic("mm_mem_free: page off limits");
    LIST_INSERT_HEAD(&free_pages, &pages[hash_page(page)], status);
}

/* map a page in the PDT */
void mm_map_page(mm_page *dir, void *vir, void *phy)
{
    mm_page *dir_entry, *table_entry;

    dir_entry = dir + ((u32_t) vir >> 22);

    /* table not present? */
    if (!(dir_entry->attr & MM_ATTR_P)) {
        void *table_page = mm_mem_alloc();
        *dir_entry = make_mm_entry_addr(table_page, 7);
        mm_map_page(dir, table_page, table_page);
    }

    table_entry = (mm_page *) (dir_entry->base << 12) +
                              (((u32_t) vir >> 12) & 0x3FF);
    *table_entry = make_mm_entry_addr(phy,  7);
    tlbflush();
}

/* umap a page in the PDT */
void mm_umap_page(mm_page *dir, void *vir)
{
    mm_page *dir_entry, *table_entry;

    dir_entry = dir + ((u32_t) vir >> 22);
    table_entry = (mm_page *) (dir_entry->base << 12) +
                                                (((u32_t) vir >> 12) & 0x3FF);

    /* page not mapped? */
    if (!(dir_entry->attr & MM_ATTR_P) || !(table_entry->attr & MM_ATTR_P))
        debug_panic("mm_umap_page: page not mapped");

    table_entry->attr = 0;
    tlbflush();
}

/* make page directory table with first 0x0 to 0x3fffff ident mapping (kernel) */
mm_page* mm_dir_new()
{
    u32_t base;
    mm_page *dirbase   = (mm_page *) mm_mem_alloc();
    mm_page *tablebase = (mm_page *) mm_mem_alloc();

    /* 0111 means present, r/w and user */
    *dirbase = make_mm_entry_addr(tablebase, 7);

    /* mark first page as not present, to catch NULL pointers */
    /* XXX see why it does not work */
    *tablebase = make_mm_entry_addr(0, 2);
    tablebase++;

    for (base = PAGE_SIZE; base < 0x400000; base += PAGE_SIZE) {
        /* 011 means present, r/w and supervisor */
        *tablebase = make_mm_entry_addr(base, 3);
        tablebase++;
    }

    return dirbase;
}

/* copy a page directory table */
mm_page *mm_dir_cpy(mm_page *dir)
{
    mm_page *d, *end;
    mm_page *dirbase, *tablebase;

    dirbase = (mm_page *) mm_mem_alloc();

    end = (mm_page *) ((char *) dir + PAGE_SIZE);
    for (d = dir; d < end; d++, dirbase++) {
        if (d->attr & MM_ATTR_P) {
            tablebase = (mm_page *) mm_mem_alloc();
            *dirbase = make_mm_entry_addr(tablebase, d->attr);
            mymemcpy((char *) (d->base << 12), (char *) tablebase, PAGE_SIZE);
        }
    }

    return dirbase;
}

/* free directory page and all its present tables */
void mm_dir_free(mm_page* d)
{
    mm_page *end = d + PAGE_SIZE / sizeof(mm_page);
    mm_mem_free(d);
    for (; d < end; ++d)
        if (d->attr & MM_ATTR_P)
            mm_mem_free((void *) ((u32_t) d->base << 12));
}

/* return a 4kb page to a process (fix this, the last_mem thing sucks) */
void *sys_palloc()
{
    void *page, *virt;

    page = mm_mem_alloc();
    add_process_page(current_process, page);

    virt = current_process->last_mem;
    current_process->last_mem += PAGE_SIZE;

    mm_map_page(current_process->pages_dir, virt, page);

    return virt;
}
