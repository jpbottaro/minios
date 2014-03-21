#include <minios/dev.h>
#include <minios/mm.h>
#include <minios/pm.h>
#include "debug.h"
#include "vmm.h"

struct file_s vmm_dev_str;
struct file_s *vmm_dev = &vmm_dev_str;
struct page_s vpages[VPAGES_LEN];
int vmm_enabled = 0;

LIST_HEAD(free_vpages_t, page_s) free_vpages;

void vmm_mem_add_reference(int i)
{
    ++vpages[i].refcount;
}

void vmm_mem_free_reference(int i)
{
    if (vpages[i].refcount <= 0)
        debug_panic("vmm_mem_free_reference: vpage refcount <= 0");
    if (--vpages[i].refcount == 0)
        LIST_INSERT_HEAD(&free_vpages, &pages[i], status);
}

/* retrieve the ith virtual page from secondary storage and place it in memory */
void *vmm_retrieve(int i)
{
    if (!vmm_enabled)
        return NULL;

    return NULL;
}

/* free up a page in main memory and return it */
struct page_s *vmm_free_page()
{
    if (!vmm_enabled)
        return NULL;

    return NULL;
}

/* alloc virtual page in secondary storage */
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

    vmm_enabled = 1;
}
