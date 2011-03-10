#include <minikernel/panic.h>
#include "i386.h"
#include "mmu.h"

unsigned int kFreePage = KERNEL_PAGES;

void init_mmu()
{
    kFreePage = KERNEL_PAGES;
}

unsigned int new_page()
{
    unsigned int base;

    for (base = kFreePage; base < kFreePage + PAGE_SIZE; base += 4)
        *((unsigned int *) base) = 0;
    kFreePage += PAGE_SIZE;

    return (kFreePage - PAGE_SIZE);
}

void map_page(unsigned int virtual, unsigned int cr3, unsigned int real)
{
    unsigned int dir_index   = (virtual >> 22);
    unsigned int table_index = (virtual >> 12) & 0x3FF;
    unsigned int *dir_entry  = (unsigned int *) (cr3 + dir_index * 4);
    unsigned int *table_entry;
    
    if (!(*dir_entry & 0x1)) // table not present
        *dir_entry = new_page() | 0111;

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

    /* 011 means present, r/w and supervisor */
    *dirbase = ((unsigned int) tablebase | 0111);
    for (base = 0; base < 0x3fffff; base += PAGE_SIZE) {
        *tablebase = ((unsigned int) base | 0111);
        tablebase++;
    }

    return (unsigned int) dirbase;
}
