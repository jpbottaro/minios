#ifndef __VMM_H__
#define __VMM_H__

#include <minios/mm.h>

#define VPAGES_LEN 0x10

void vmm_init();
void *vmm_retrieve(struct process_state_s *p, void *vir, mm_page *entry);
struct page_s *vmm_free_page();

#endif /* __VMM_H__ */
