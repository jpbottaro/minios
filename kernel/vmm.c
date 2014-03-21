#include <minios/dev.h>
#include <minios/mm.h>
#include <minios/pm.h>
#include "debug.h"
#include "vmm.h"

struct file_s vmm_dev_str;
struct file_s *vmm_dev = &vmm_dev_str;
struct page_s vpages[VPAGES_LEN];
int vmm_enabled = 0;

TAILQ_HEAD(free_vpages_t, page_s) free_vpages;

/* init virtual memory manager */
void vmm_init(dev_t dev, void *swap_offset)
{
    int i;

    vmm_dev->f_op = dev_operations(dev);

    TAILQ_INIT(&free_vpages);
    for (i = 0; i < VPAGES_LEN; ++i) {
        vpages[i].base = swap_offset + i * PAGE_SIZE;
        TAILQ_INSERT_TAIL(&free_vpages, &vpages[i], status);
    }

    vmm_enabled = 1;
}

void vmm_mem_add_reference(int i)
{
    ++vpages[i].refcount;
}

void vmm_mem_free_reference(int i)
{
    if (vpages[i].refcount <= 0)
        debug_panic("vmm_mem_free_reference: vpage refcount <= 0");
    if (--vpages[i].refcount == 0)
        TAILQ_INSERT_HEAD(&free_vpages, &vpages[i], status);
}

/* alloc virtual page in secondary storage */
void *vmm_mem_alloc()
{
    struct page_s *page;

    page = TAILQ_FIRST(&free_vpages);
    if (page == NULL)
        debug_panic("vmm_mem_alloc: no more free virtual pages");
    TAILQ_REMOVE(&free_vpages, page, status);
    page->refcount = 1;

    return page->base;
}

struct page_s *vmm_select_victim()
{
    struct page_s *page = TAILQ_FIRST(&victim_pages);

    if (page != NULL) {
        TAILQ_REMOVE(&victim_pages, page, status);
        TAILQ_INSERT_TAIL(&victim_pages, page, status);
    }

    return page;
}

/* free up a page in main memory and return it */
struct page_s *vmm_free_page()
{
    struct page_s *page = NULL;

    if (vmm_enabled) {
    }

    return page;
}

/* retrieve the ith virtual page from secondary storage and place it in memory */
void *vmm_retrieve(mm_page *entry)
{
    void *page = NULL;

    if (vmm_enabled) {
    }

    return page;
}
