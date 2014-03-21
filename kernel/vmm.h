#ifndef __VMM_H__
#define __VMM_H__

#include <minios/mm.h>

/* TODO set right swap offset/size */
#define SWAP_OFFSET 0x10000
#define VPAGES_LEN 0x100

#define VMM_MAIN      0
#define VMM_SECONDARY 1

extern struct pages_queue_s free_pages;
extern struct pages_queue_s victim_pages;

void vmm_init();

void vmm_mem_add_reference(int i);
void vmm_mem_free_reference(int i);

void *vmm_retrieve(mm_page *entry);
struct page_s *vmm_free_page();

#endif /* __VMM_H__ */
