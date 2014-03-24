#include <minios/dev.h>
#include <minios/mm.h>
#include <minios/pm.h>
#include <unistd.h>
#include "debug.h"
#include "clock.h"
#include "vmm.h"
#include "pm.h"

struct file_s vmm_dev_str;
struct file_s *vmm_dev = &vmm_dev_str;
struct page_s vpages[VPAGES_LEN];
int vmm_enabled = 0;

struct page_s *mm_mem_alloc();

TAILQ_HEAD(free_vpages_t, page_s) free_vpages;

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

void vmm_walk_page_tables(void (*func)(mm_page *, u32_t, u32_t),
        u32_t from, u32_t to)
{
    int i;
    mm_page *dir_entry, *table_entry;
    mm_page *dir_end, *table_end;

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
                func(table_entry, from, to);
            }
        }
    }
}

void vmm_update_entry_secondary(mm_page *entry, u32_t from, u32_t to)
{
    if ((entry->attr & MM_ATTR_P) && entry->base == from) {
        entry->base = to;
        entry->attr &= ~MM_ATTR_P;
        entry->attr |= MM_VM;
    }
}

void vmm_update_entry_main(mm_page *entry, u32_t from, u32_t to)
{
    if ((entry->attr & MM_VM) && entry->base == from) {
        entry->base = to;
        entry->attr |= MM_ATTR_P;
        entry->attr &= ~MM_VM;
    }
}

/* change all references from a page to another; depending on the direction
 * (from/to main memory), update flags of dir tables accordingly
 */
void vmm_move_references(struct page_s *from_page, struct page_s *to_page,
        int direction)
{
    /* improve this so that we don't have to go to every dir table of every
     * process; keeping references for each page (eg. process nr/virtual address)
     * would make this O(n) with n = references
     */
    u32_t from, to;

    to_page->refcount = from_page->refcount;
    if (direction == VMM_MAIN) {
        from = from_page - vpages;
        to = (u32_t) to_page->base >> 12;
        vmm_walk_page_tables(vmm_update_entry_main, from, to);
    } else {
        from = (u32_t) from_page->base >> 12;
        to = to_page - vpages;
        vmm_walk_page_tables(vmm_update_entry_secondary, from, to);
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
struct page_s *vmm_retrieve(mm_page *entry)
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
        TAILQ_INSERT_HEAD(&free_vpages, &vpages[i], status);
    }

    return page;
}

void vmm_lru_crawler(mm_page *entry, u32_t i, u32_t j)
{
    struct page_s *page;
    void *base;

    if (entry->attr & MM_ATTR_A) {
        entry->attr &= ~MM_ATTR_A;
        base = (void *) (entry->base << 12);
        TAILQ_FOREACH(page, &victim_pages, status) {
            if (page->base == base) {
                TAILQ_REMOVE(&victim_pages, page, status);
                TAILQ_INSERT_TAIL(&victim_pages, page, status);
                return;
            }
        }
    }
}

/* clock watcher to implement LRU-style algorithm */
void vmm_lru_watcher()
{
    vmm_walk_page_tables(vmm_lru_crawler, 0, 0);
}

/* init virtual memory manager */
void vmm_init(dev_t dev, void *swap_offset)
{
    int i;

    TAILQ_INIT(&free_vpages);
    for (i = 0; i < VPAGES_LEN; ++i) {
        vpages[i].base = swap_offset + i * PAGE_SIZE;
        TAILQ_INSERT_TAIL(&free_vpages, &vpages[i], status);
    }

    vmm_dev->f_op = dev_operations(dev);
    clock_add_watcher(vmm_lru_watcher);
    vmm_enabled = 1;
}
