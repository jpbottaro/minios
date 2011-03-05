#ifndef __MMU_H__
#define __MMU_H__

#define KERNEL_PAGES    0x00100000
#define PAGE_SIZE       0x1000

unsigned int kFreePage;

void init_mmu();
unsigned int new_page();

void map_page(unsigned int virtual, unsigned int cr3, unsigned int real);
void umap_page(unsigned int virtual, unsigned int cr3);

unsigned int *init_dir_kernel();

#endif /* __MMU_H__ */
