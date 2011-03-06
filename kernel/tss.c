#include "tss.h"
#include "gdt.h"
#include "mmu.h"
#include "i386.h"
#include "sched.h"

#define EFLAGS_MASK 0x00000202
#define TSS_SIZE    0x68
#define KSTACKSIZE  0x1FF0

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

/* label from kernel.asm, just a jmp $ */
extern void idle();
extern unsigned int *kstack;

struct tss_s tss[MAX_PROCESSES];
unsigned short first;

void init_tss()
{
    struct tss_s *t;
    gdt_entry *entry;

    /* fill gdt with tss entries */
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
    tss[pos].ss0    = SEG_DESC_DATA;
    tss[pos].esp0   = ((unsigned int) kstack) + KSTACKSIZE;
    tss[pos].dtrap  = 0x0;
    tss[pos].iomap  = 0xFFFF;

    return 0;   
}

int add_tss(unsigned int pos, unsigned int user_cr3)
{
    tss[pos].cr3    = user_cr3;
    tss[pos].eip    = CODE_OFFSET;
    tss[pos].eflags = EFLAGS_MASK;
    tss[pos].esp    = 0xFFFFFFFF;
    tss[pos].ebp    = 0xFFFFFFFF;
    tss[pos].cs     = SEG_DESC_CODE;
    tss[pos].ds     = SEG_DESC_DATA;
    tss[pos].ss     = SEG_DESC_DATA;
    tss[pos].fs     = SEG_DESC_DATA;
    tss[pos].gs     = SEG_DESC_DATA;
    tss[pos].es     = SEG_DESC_DATA;
    tss[pos].ss0    = SEG_DESC_DATA;
    tss[pos].esp0   = ((unsigned int) kstack) + KSTACKSIZE;
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
