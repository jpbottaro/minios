#include <minios/mm.h>
#include <minios/i386.h>
#include <minios/panic.h>
#include <minios/sched.h>
#include <sys/types.h>

struct page_s pages[PAGES_LEN];

LIST_HEAD(free_pages_t, page_s) free_pages;

/* init memory manager */
void init_mm()
{
    int i;

    LIST_INIT(&free_pages);
    for (i = PAGES_LEN - 1; i >= 0; --i) {
        pages[i].base = (void *) KERNEL_PAGES + i * PAGE_SIZE;
        LIST_INSERT_HEAD(&free_pages, &pages[i], status);
    }
}

/* alloc page */
void *mm_mem_alloc()
{
    u32_t *start, *end;
    struct page_s *page;

    page = LIST_FIRST(&free_pages);
    if (page == NULL)
        panic("mm_mem_alloc: no more free pages");
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
        panic("mm_mem_free: page off limits");
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
        *dir_entry = make_mm_entry_addr(table_page, 0111);
        mm_map_page(dir, table_page, table_page);
    }

    table_entry = (mm_page *) (dir_entry->base << 12) +
                                                (((u32_t) vir >> 12) & 0x3FF);
    *table_entry = make_mm_entry_addr(phy,  0111);
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
        panic("mm_umap_page: page not mapped");

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
    *dirbase = make_mm_entry_addr(tablebase, 0111);
    for (base = 0; base < 0x400000; base += PAGE_SIZE) {
        /* 011 means present, r/w and supervisor */
        *tablebase = make_mm_entry_addr(base, 011);
        tablebase++;
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
