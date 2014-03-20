#ifndef __VMM_H__
#define __VMM_H__

#include <minios/mm.h>

/* TODO set right swap offset */
#define SWAP_OFFSET 0x0
#define VPAGES_LEN 0x10

void vmm_init();

void vmm_mem_add_reference(int i);
void vmm_mem_free_reference(int i);

void *vmm_retrieve(int i);
struct page_s *vmm_free_page();

#endif /* __VMM_H__ */
