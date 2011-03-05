#include "tss.h"
#include "gdt.h"
#include "mmu.h"
#include "i386.h"
#include "sched.h"

#define EFLAGS_MASK 0x00000202
#define TSS_SIZE    0x68

#define tss_gdt_entry(entry, tss_addr) \
    entry->limit_0_15  = TSS_SIZE - 1; \
    entry->limit_16_19 = 0x0; \
    entry->base_0_15   = (unsigned short) ((unsigned int) tss_addr & 0xFFFF); \
    entry->base_23_16  = (unsigned char) (((unsigned int) tss_addr >> 16) & 0xFF); \
    entry->base_31_24  = (unsigned char) (((unsigned int) tss_addr >> 24) & 0xFF); \
    entry->type        = 0x9; \
    entry->s           = 0x0; \
    entry->dpl         = 0x0; \
    entry->p           = 0x1; \
    entry->avl         = 0x0; \
    entry->l           = 0x0; \
    entry->db          = 0x0; \
    entry->g           = 0x1;

extern void idle();

struct tss_s tss[MAX_PROCESSES];
unsigned short first;

void init_tss()
{
    struct tss_s *t;
    gdt_entry *entry;

    /* load gdt with tss entries */
    for (t = &tss[0]; t < &tss[MAX_PROCESSES]; t++) {
        entry = (gdt_entry *) gdt_free_entry();
        if (t == &tss[0])
            first = gdt_desc((unsigned int) entry);
        tss_gdt_entry(entry, t);
    }

    /* Initial task, we wont use this again */
    tss[0].cr3    = rcr3();
    tss[0].dtrap  = 0x0;
    tss[0].iomap  = 0xFFFF;
    ltr(first);
}

unsigned int task_desc(unsigned int pos)
{
    return first + pos * sizeof(gdt_entry);
}

int add_idle(unsigned int pos)
{
    unsigned int stack = new_page();
    map_page(stack, rcr3(), stack);

    tss[pos].cr3    = (unsigned int) rcr3();
    tss[pos].eip    = (unsigned int) idle;
    tss[pos].eflags = EFLAGS_MASK;
    tss[pos].esp    = stack + PAGE_SIZE - 1;
    tss[pos].ebp    = stack + PAGE_SIZE - 1;
    tss[pos].cs     = SEG_DESC_CODE;
    tss[pos].ds     = SEG_DESC_DATA;
    tss[pos].ss     = SEG_DESC_DATA;
    tss[pos].fs     = SEG_DESC_DATA;
    tss[pos].gs     = SEG_DESC_DATA;
    tss[pos].es     = SEG_DESC_VIDEO;
    tss[pos].dtrap  = 0x0;
    tss[pos].iomap  = 0xFFFF;

    return 0;   
}

int add_tss(unsigned int pos, unsigned int eip, unsigned int size)
{
    unsigned int i, stack, code[MAX_PAGES];
    unsigned int *user_cr3, *code_ptr;

    /* reserve pages for stack and code */
    stack = new_page();
    for (i = 0; i * PAGE_SIZE < size; i++) {
        code[i] = new_page();
        /* temporary ident mapping to be able to copy the code */
        map_page(code[i], rcr3(), code[i]);
    }

    /* create directoy table for the new process */
    user_cr3 = init_dir_user(code, i, stack);

    /* copy the code */
    code_ptr = code;
    for (i = eip; i < eip + size; i += 4, code_ptr++)
        *code_ptr = *((unsigned int *) i);

    /* remove temporary mapping */
    for (i = 0; i * PAGE_SIZE < size; i++)
        umap_page(code[i], rcr3());

    tss[pos].cr3    = (unsigned int) user_cr3;
    tss[pos].eip    = 0;
    tss[pos].eflags = EFLAGS_MASK;
    tss[pos].esp    = 0xFFFFFFFF;
    tss[pos].ebp    = 0xFFFFFFFF;
    tss[pos].cs     = SEG_DESC_CODE;
    tss[pos].ds     = SEG_DESC_DATA;
    tss[pos].ss     = SEG_DESC_DATA;
    tss[pos].fs     = SEG_DESC_DATA;
    tss[pos].gs     = SEG_DESC_DATA;
    tss[pos].es     = SEG_DESC_VIDEO;
    tss[pos].dtrap  = 0x0;
    tss[pos].iomap  = 0xFFFF;

    return 0;
}

/* the mmu is so simple that it does not free up memory, so nothing to see
 * here, move along
 */
void free_tss(unsigned int pos)
{
}
