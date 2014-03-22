#include <minios/dev.h>
#include <minios/mm.h>
#include <minios/pm.h>
#include <unistd.h>
#include "debug.h"
#include "vmm.h"
#include "pm.h"

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

/* change all references from a page to another; depending on the direction
 * (from/to main memory), update flags of dir tables accordingly
 */
void vmm_move_references(struct page_s *from, struct page_s *to, int direction)
{
    /* improve this so that we don't have to go to every dir table of every
     * process; keeping references for each page (eg. process nr/virtual address)
     * would make this O(n) with n = references
     */
    int i, idx;
    mm_page *dir_entry, *table_entry;
    mm_page *dir_end, *table_end;

    to->refcount = from->refcount;
    if (direction == VMM_MAIN)
        idx = from - vpages;
    else
        idx = to - vpages;

    for (i = 0; i < MAX_PROCESSES; i++) {
        struct process_state_s *p = &ps[i];
        if (p->pid == 0)
            continue;

        dir_entry = p->pages_dir;
        dir_end = dir_entry + PAGE_SIZE / sizeof(mm_page);
        for (; dir_entry < dir_end; ++dir_entry) {
            if (!(dir_entry->attr & MM_ATTR_P))
                continue;

            table_entry = (mm_page *) (dir_entry->base << 12);
            table_end = table_entry + PAGE_SIZE / sizeof(mm_page);
            for (; table_entry < table_end; ++table_entry) {
                /* don't touch non-user pages (this is the identity map) */
                if (!(table_entry->attr & MM_ATTR_US))
                    continue;
                if ((direction == VMM_MAIN) &&
                    (table_entry->attr & MM_VM) &&
                    (table_entry->base == idx)) {

                    table_entry->base = (u32_t) to->base >> 12;
                    table_entry->attr |= MM_ATTR_P;
                    table_entry->attr &= ~MM_VM;

                } else if ((direction == VMM_SECONDARY) &&
                    (table_entry->attr & MM_ATTR_P) &&
                    ((table_entry->base << 12) == (u32_t) from->base)) {

                    table_entry->base = idx;
                    table_entry->attr &= ~MM_ATTR_P;
                    table_entry->attr |= MM_VM;
                }
            }
        }
    }
}

/* alloc virtual page in secondary storage */
struct page_s *vmm_mem_alloc()
{
    struct page_s *page;

    page = TAILQ_FIRST(&free_vpages);
    if (page != NULL) {
        TAILQ_REMOVE(&free_vpages, page, status);
        page->refcount = 1;
        return page;
    }

    /* TODO remove panic call */
    debug_panic("vmm_mem_alloc: no more free virtual pages");
    return NULL;
}

/* select next victim to be evicted from main memory */
struct page_s *vmm_select_victim()
{
    return TAILQ_FIRST(&victim_pages);
}

/* copy the 'src' page to the secondary device, at the 'offset' position */
void vmm_copy_to_device(void *src, u32_t offset)
{
    vmm_dev->f_op->lseek(vmm_dev, offset, SEEK_SET);
    vmm_dev->f_op->write(vmm_dev, src, PAGE_SIZE);
}

/* copy the page at the 'offset' possition to the src page */
void vmm_copy_from_device(void *src, u32_t offset)
{
    vmm_dev->f_op->lseek(vmm_dev, offset, SEEK_SET);
    vmm_dev->f_op->read(vmm_dev, src, PAGE_SIZE);
}

/* evict page to secondary device */
struct page_s *vmm_evict(struct page_s *page)
{
    struct page_s *vpage = vmm_mem_alloc();

    if (vpage != NULL) {
        vmm_copy_to_device(page->base, (u32_t) vpage->base);
        vmm_move_references(page, vpage, VMM_SECONDARY);
    }

    return vpage;
}

/* free up a page in main memory and return it */
struct page_s *vmm_free_page()
{
    if (!vmm_enabled)
        return NULL;

    struct page_s *page = vmm_select_victim();

    if (page == NULL || vmm_evict(page) == NULL)
        return NULL;

    TAILQ_REMOVE(&victim_pages, page, status);
    TAILQ_INSERT_HEAD(&free_pages, page, status);

    return page;
}

/* retrieve the ith virtual page from secondary storage and place it in memory */
void *vmm_retrieve(mm_page *entry)
{
    if (!vmm_enabled)
        return NULL;

    int i = entry->base;

    if (i < 0 || i > VPAGES_LEN)
        debug_panic("vmm_retrieve: index out of range");

    struct page_s *vpage = &vpages[i];
    struct page_s *page = mm_mem_alloc();

    if (page != NULL) {
        vmm_copy_from_device(page->base, (u32_t) vpage->base);
        vmm_move_references(vpage, page, VMM_MAIN);
    }

    return page->base;
}
