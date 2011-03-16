#include <minikernel/panic.h>
#include <minikernel/sched.h>
#include "i386.h"
#include "mmu.h"

#define PAGES_LEN ((CODE_OFFSET - KERNEL_PAGES) / PAGE_SIZE)
#define hash_page(base) ((base - KERNEL_PAGES) / PAGE_SIZE)

struct page_s {
    /* starting address */
    unsigned int base;

    /* scheduler waiting list */
    LIST_ENTRY(page_s) status;
} __attribute__((__packed__)) ;

struct page_s pages[PAGES_LEN];

LIST_HEAD(free_pages_t, page_s) free_pages;

void init_mmu()
{
    int i;

    LIST_INIT(&free_pages);
    for (i = PAGES_LEN - 1; i >= 0; --i) {
        pages[i].base = KERNEL_PAGES + i * PAGE_SIZE;
        LIST_INSERT_HEAD(&free_pages, &pages[i], status);
    }
}

unsigned int new_page()
{
    struct page_s *page;
    unsigned int base;

    page = LIST_FIRST(&free_pages);
    if (page == NULL)
        panic("new_page: no more free pages!");
    LIST_REMOVE(page, status);

    for (base = page->base; base < page->base + PAGE_SIZE; base++)
        *((unsigned int *) base) = 0;

    return page->base;
}

void free_page(unsigned int base)
{
    if (base < KERNEL_PAGES || base > CODE_OFFSET)
        panic("free_page: page off limits");
    LIST_INSERT_HEAD(&free_pages, &pages[hash_page(base)], status);
}

void map_page(unsigned int virtual, unsigned int cr3, unsigned int real)
{
    unsigned int dir_index   = (virtual >> 22);
    unsigned int table_index = (virtual >> 12) & 0x3FF;
    unsigned int *dir_entry  = (unsigned int *) (cr3 + dir_index * 4);
    unsigned int *table_entry;
    unsigned int table_page;
    
    if (!(*dir_entry & 0x1)) { // table not present
        table_page = new_page() | 0111;
        *dir_entry = table_page;
        map_page(table_page, cr3, table_page);
    }

    table_entry  = (unsigned int *) ((*dir_entry & ~0xFFF) + table_index * 4);
    *table_entry = (real & ~0xFFF) | 0111;
    tlbflush();
}

void umap_page(unsigned int virtual, unsigned int cr3)
{
    unsigned int dir_index   = (virtual >> 22);
    unsigned int table_index = (virtual >> 12) & 0x3FF;
    unsigned int *dir_entry  = (unsigned int *) (cr3 + dir_index * 4);
    unsigned int *table_entry;

    table_entry  = (unsigned int *) ((*dir_entry & ~0xFFF) + table_index * 4);

    if (!(*dir_entry & 0x1) || !(*table_entry & 0x1)) // page not mapped
        panic("umap");

    *table_entry = 0;
    tlbflush();
}

/* make page directory table with first 0x0 to 0x3fffff ident mapping (kernel) */
unsigned int init_directory()
{
    unsigned int base;
    unsigned int *dirbase   = (unsigned int *) new_page();
    unsigned int *tablebase = (unsigned int *) new_page();

    /* 0111 means present, r/w and user */
    *dirbase = ((unsigned int) tablebase | 0111);
    for (base = 0; base < 0x3fffff; base += PAGE_SIZE) {
        /* 011 means present, r/w and supervisor */
        *tablebase = ((unsigned int) base | 011);
        tablebase++;
    }

    return (unsigned int) dirbase;
}
