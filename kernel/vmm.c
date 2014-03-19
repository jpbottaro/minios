#include <minios/mm.h>
#include <minios/pm.h>
#include <minios/dev.h>
#include "debug.h"
#include "vmm.h"

struct file_s vmm_dev_str;
struct file_s *vmm_dev = &vmm_dev_str;
struct page_s vpages[VPAGES_LEN];

LIST_HEAD(free_vpages_t, page_s) free_vpages;

void increase_refcount_vpage(int i)
{
    ++vpages[i].refcount;
}

void decrease_refcount_vpage(int i)
{
    if (vpages[i].refcount <= 0)
        debug_panic("decrease_refcount_vpage: vpage refcount <= 0");
    if (--vpages[i].refcount == 0)
        LIST_INSERT_HEAD(&free_vpages, &pages[i], status);
}

void *vmm_retrieve(struct process_state_s *p, void *vir, mm_page *entry)
{
    return NULL;
}

struct page_s *vmm_free_page()
{
    return NULL;
}

/* init virtual memory manager */
void vmm_init(dev_t dev, void *swap_offset)
{
    int i;

    vmm_dev->f_op = dev_operations(dev);

    LIST_INIT(&free_vpages);
    for (i = VPAGES_LEN - 1; i >= 0; --i) {
        vpages[i].base = swap_offset + i * PAGE_SIZE;
        LIST_INSERT_HEAD(&free_vpages, &vpages[i], status);
    }
}

/* alloc page */
void *vmm_mem_alloc()
{
    struct page_s *page;

    page = LIST_FIRST(&free_vpages);
    if (page == NULL)
        debug_panic("vmm_mem_alloc: no more free virtual pages");
    LIST_REMOVE(page, status);
    page->refcount = 1;

    return page->base;
}
