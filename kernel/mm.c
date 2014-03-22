#include <minios/scall.h>
#include <minios/i386.h>
#include <minios/misc.h>
#include <minios/idt.h>
#include <minios/mm.h>
#include <sys/types.h>
#include "debug.h"
#include "vmm.h"
#include "pm.h"

extern void _pf_handler();

struct page_s pages[PAGES_LEN];
mm_page *tables_mem[PAGES_PER_PAGE + 1];

struct pages_queue_s free_pages;
struct pages_queue_s victim_pages;

void mm_map_page(mm_page *dir, void *vir, void *phy);

void mm_mem_add_reference(void *page)
{
    if (page < (void *) KERNEL_PAGES)
        debug_panic("mm_mem_add_reference: page under mem limit");
    if (page > (void *) MEM_LIMIT)
        debug_panic("mm_mem_add_reference: page over mem limit");

    ++pages[hash_page(page)].refcount;
}

void mm_mem_free_reference(void *page)
{
    if (page < (void *) KERNEL_PAGES)
        debug_panic("mm_mem_free_reference: page under mem limit");
    if (page > (void *) MEM_LIMIT)
        debug_panic("mm_mem_free_reference: page over mem limit");

    struct page_s *p = &pages[hash_page(page)];

    if (p->refcount <= 0)
      debug_panic("mm_mem_free_reference: page refcount <= 0");
    if (--p->refcount == 0) {
        if (!p->pinned)
            TAILQ_REMOVE(&victim_pages, p, status);
        TAILQ_INSERT_HEAD(&free_pages, p, status);
    }
}

/* alloc pinned page */
struct page_s *mm_mem_alloc_pinned()
{
    u32_t *start, *end;
    struct page_s *page;

    page = TAILQ_FIRST(&free_pages);
    if (page == NULL) {
        page = vmm_free_page();
        if (page == NULL)
            debug_panic("mm_mem_alloc: no more free pages");
    }
    TAILQ_REMOVE(&free_pages, page, status);

    start = (u32_t *) page->base;
    end = start + PAGE_SIZE / sizeof(u32_t);
    for (; start < end; start++)
        *start = 0;

    page->pinned = 1;
    page->refcount = 1;

    return page;
}

/* alloc unpinned page */
struct page_s *mm_mem_alloc()
{
    struct page_s *page = mm_mem_alloc_pinned();

    TAILQ_INSERT_TAIL(&victim_pages, page, status);
    page->pinned = 0;

    return page;
}

mm_page *mm_search_page(mm_page *dir, void *vir, int flag)
{
    mm_page *dir_entry;

    dir_entry = dir + ((u32_t) vir >> 22);

    /* table not present? */
    if (!(dir_entry->attr & MM_ATTR_P)) {
        if (flag == MM_NOCREAT)
            return NULL;
        void *table_page = mm_mem_alloc_pinned()->base;
        *dir_entry = make_mm_entry_addr(table_page,
                MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
        mm_map_page(dir, table_page, table_page);
    }

    return (mm_page *) (dir_entry->base << 12) +
                       (((u32_t) vir >> 12) & 0x3FF);
}

/* build page and add it to the dirtable, making it pinned */
void *mm_build_page_pinned(mm_page *dir, void *vir)
{
    struct page_s *page = mm_mem_alloc_pinned();
    mm_map_page(dir, vir, page->base);
    return page->base;
}

/* build page and add it to the dirtable */
void *mm_build_page(mm_page *dir, void *vir)
{
    struct page_s *page = mm_mem_alloc();
    mm_map_page(dir, vir, page->base);
    return page->base;
}

/* map a page in the PDT */
void mm_map_page_attr(mm_page *dir, void *vir, void *phy, int attr)
{
    mm_page *table_entry;

    table_entry = mm_search_page(dir, vir, MM_CREAT);
    *table_entry = make_mm_entry_addr(phy,  attr);
    tlbflush();
}

/* map a page in the PDT marked user, r/w, present */
void mm_map_page(mm_page *dir, void *vir, void *phy)
{
    mm_map_page_attr(dir, vir, phy, MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
}

/* umap a page in the PDT */
void mm_umap_page(mm_page *dir, void *vir)
{
    mm_page *table_entry;

    if ( (table_entry = mm_search_page(dir, vir, MM_NOCREAT)) == NULL)
        debug_panic("mm_umap_page: page not mapped");
    table_entry->attr = 0;
    tlbflush();
}

/* translate virtual to physical address */
void *mm_translate(mm_page *dir, void *vir)
{
    void *ret = NULL;
    mm_page *page = mm_search_page(dir, vir, MM_NOCREAT);

    if (page != NULL)
        ret = (void *) (page->base << 12);

    return ret;
}

struct page_s *mm_newpage(mm_page *entry)
{
    struct page_s *new_page = mm_mem_alloc();
    *entry = make_mm_entry_addr(new_page->base, MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);
    tlbflush();
    return new_page;
}

struct page_s *mm_copy_on_write(mm_page *entry)
{
    struct page_s *old_page = &pages[hash_page(entry->base << 12)];

    if (old_page->refcount > 1) {
        /* create new page to replace old page */
        struct page_s *new_page = mm_newpage(entry);

        /* copy the contents of old page to the new page */
        mymemcpy(new_page->base, old_page->base, PAGE_SIZE);

        /* decrease refcount of old page */
        mm_mem_free_reference(old_page->base);

        return new_page;
    }

    entry->attr |= MM_ATTR_RW;

    return old_page;
}

/* new handler for page fault to force exit of ring3 tasks who force it */
void pf_handler()
{
    mm_page *page;
    struct page_s *res;

    if (current_process == NULL)
        isr14();

    res = NULL;
    page = mm_search_page((void *) rcr3(), (void *) rcr2(), MM_NOCREAT);
    if (page != NULL && page->attr & MM_ATTR_US) {
        if (!(page->attr & MM_ATTR_P)) {
            if (page->attr & MM_VALID)
                res = mm_newpage(page);
            else if (page->attr & MM_VM)
                res = vmm_retrieve(page);

        } else if (!(page->attr & MM_ATTR_RW)) {
            if (!(page->attr & MM_SHARED))
                res = mm_copy_on_write(page);
        }
    }

    if (res == NULL)
        debug_panic("pf_handler: segmentation fault");
}

/* make page directory table with first 0x0 to MEM_LIMIT ident mapping (kernel) */
mm_page* mm_dir_new()
{
    u32_t base;
    mm_page *dirbase   = (mm_page *) (mm_mem_alloc_pinned()->base);
    mm_page *tablebase = (mm_page *) (mm_mem_alloc_pinned()->base);

    /* 0111 means present, r/w and user */
    *dirbase = make_mm_entry_addr(tablebase, MM_ATTR_P | MM_ATTR_RW | MM_ATTR_US);

    /* mark first page as not present, to catch NULL pointers */
    *tablebase = make_mm_entry_addr(0, 0);
    tablebase++;

    for (base = PAGE_SIZE; base < MEM_LIMIT; tablebase++, base += PAGE_SIZE)
        *tablebase = make_mm_entry_addr(base, MM_ATTR_P | MM_ATTR_RW);

    /* ident map the directory tables (this way they're 'user' and not 'system') */
    mm_map_page(dirbase, dirbase, dirbase);
    mm_map_page(dirbase, tablebase, tablebase);

    /* build user/kernel stack and args pages */
    mm_build_page(dirbase, (char *) ARG_PAGE);
    mm_build_page_pinned(dirbase, (char *) KSTACK_PAGE);
    mm_build_page_pinned(dirbase, (char *) STACK_PAGE);

    return dirbase;
}

/* XXX we are also copying and referencing the dir tables mappings from the
 * parent process, which we don't want; we should skip those
 */
void copy_table_fork(mm_page *to, mm_page *from)
{
    mm_page *end = from + (PAGE_SIZE / sizeof(mm_page *));

    while (from < end) {
        if (!(from->attr & MM_SHARED))
            from->attr &= ~MM_ATTR_RW;
        *to = *from;
        if ((from->attr & (MM_ATTR_P | MM_ATTR_US)) == (MM_ATTR_P | MM_ATTR_US))
            mm_mem_add_reference((void *) (from->base << 12));
        else if (from->attr & MM_VM)
            vmm_mem_add_reference(from->base);
        to++;
        from++;
    }
}

/* copy a page directory table */
mm_page *mm_dir_cpy(mm_page *dir)
{
    int i = 0;
    void *page;
    mm_page *d, *ret, *end;
    mm_page *dirbase, *tablebase;

    ret = dirbase = tables_mem[i++] = (mm_page *) (mm_mem_alloc_pinned()->base);

    /* XXX HACK!: the minus 1 prevents this from copying the stacks/args,
     * so that we can do it ourselves manually (since they reside in the
     * last page directory entry)
     */
    end = dir + (PAGE_SIZE / sizeof(mm_page *)) - 1;
    for (d = dir; d < end; d++, dirbase++) {
        if (d->attr & MM_ATTR_P) {
            tables_mem[i++] = tablebase = (mm_page *) (mm_mem_alloc_pinned()->base);
            copy_table_fork(tablebase, (mm_page *) (d->base << 12));
            *dirbase = make_mm_entry_addr(tablebase, d->attr);
        }
    }

    /* ident map the directory tables (this way they're 'user' and not 'system') */
    while (--i >= 0)
        mm_map_page(ret, tables_mem[i], tables_mem[i]);

    /* build user/kernel stack and args pages */
    page = mm_build_page(ret, (char *) ARG_PAGE);
    mymemcpy(page, (char *) ARG_PAGE, PAGE_SIZE);

    page = mm_build_page_pinned(ret, (char *) KSTACK_PAGE);
    mymemcpy(page, (char *) KSTACK_PAGE, PAGE_SIZE);

    page = mm_build_page_pinned(ret, (char *) STACK_PAGE);
    mymemcpy(page, (char *) STACK_PAGE, PAGE_SIZE);

    return ret;
}

/* free directory page and all its present tables (recursive helper function) */
void mm_dir_table_free(mm_page *d, int recursive)
{
    mm_page *entry, *end;

    end = d + PAGE_SIZE / sizeof(mm_page);
    for (; d < end; ++d) {
        if (d->attr & MM_ATTR_P) {
            entry = (void *) (d->base << 12);
            if (recursive)
                mm_dir_table_free(entry, 0);
            else if (d->attr & MM_ATTR_US)
                mm_mem_free_reference(entry);
        } else if (d->attr & MM_VM) {
            vmm_mem_free_reference(entry->base);
        }
    }
}

/* free directory page and all its present tables */
void mm_dir_free(mm_page *d)
{
    mm_dir_table_free(d, 1);
}

/* return a 4kb page to a process (fix this, the last_mem thing sucks) */
void *sys_palloc()
{
    void *virt;

    virt = current_process->last_mem;
    current_process->last_mem += PAGE_SIZE;
    /* set bit to remember that this page is valid */
    mm_map_page_attr(current_process->pages_dir, virt, 0,
            MM_ATTR_RW | MM_ATTR_US | MM_VALID);

    return virt;
}

int sys_share_page(void *page)
{
    mm_page *entry;

    entry = mm_search_page(current_process->pages_dir, page, MM_NOCREAT);
    if (entry == NULL)
        return -1;
    /* if we are going to share it, create it now so everybody knows it later */
    if (entry->attr & MM_VALID && !(entry->attr & MM_ATTR_P))
        mm_newpage(entry);
    entry->attr |= MM_SHARED;

    return 0;
}

/* init memory manager */
void mm_init()
{
    int i;

    TAILQ_INIT(&free_pages);
    TAILQ_INIT(&victim_pages);
    for (i = PAGES_LEN - 1; i >= 0; --i) {
        pages[i].base = (void *) KERNEL_PAGES + i * PAGE_SIZE;
        TAILQ_INSERT_HEAD(&free_pages, &pages[i], status);
    }

    /* register sys calls */
    SCALL_REGISTER(45, sys_palloc);
    SCALL_REGISTER(46, sys_share_page);

    /* register page fault handler */
    idt_register(14, _pf_handler, DEFAULT_PL);

    /* enable paging (kernel is identity mapped from 0x0 to MEM_LIMIT) */
    lcr3((unsigned int) mm_dir_new());
    lcr0(rcr0() | 0x80000000);
}
