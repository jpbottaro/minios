#ifndef __VMM_H__
#define __VMM_H__

#include <minios/mm.h>

/* TODO set right swap offset */
#define SWAP_OFFSET 0x0
#define VPAGES_LEN 0x10

extern struct pages_queue_s victim_pages;

void vmm_init();

void vmm_mem_add_reference(int i);
void vmm_mem_free_reference(int i);

void *vmm_retrieve(mm_page *entry);
struct page_s *vmm_free_page();

#endif /* __VMM_H__ */
