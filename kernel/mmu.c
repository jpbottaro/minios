#include "mmu.h"
#include "i386.h"
#include <minikernel/panic.h>

void init_mmu()
{
    kFreePage = KERNEL_PAGES + PAGE_SIZE * 2;
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
        *dir_entry = new_page() | 3;

    table_entry  = (unsigned int *) ((*dir_entry & ~0xFFF) + table_index * 4);
    *table_entry = (real & ~0xFFF) | 3;
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

unsigned int *init_dir_kernel()
{
    unsigned int base;
    unsigned int *dirbase   = (unsigned int *) 0x00100000;
    unsigned int *tablebase = (unsigned int *) 0x00101000;

    *dirbase = ((unsigned int) tablebase | 3); // present and read/write
    for (base = 0; base < 0x1fffff; base += PAGE_SIZE) {
        *tablebase = ((unsigned int) base | 3);
        tablebase++;
    }

    return dirbase;
}
