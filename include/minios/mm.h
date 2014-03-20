#ifndef _MM_H
#define _MM_H

#include <sys/types.h>

#define MM_ATTR_P     0x001 // Present
#define MM_ATTR_RW    0x002 // Read/Write
#define MM_ATTR_RW_R  0x000 //
#define MM_ATTR_RW_W  0x002 //
#define MM_ATTR_US    0x004 // User/Supervisor
#define MM_ATTR_US_U  0x004 //
#define MM_ATTR_US_S  0x000 //
#define MM_ATTR_WT    0x008 // Write-Through
#define MM_ATTR_CD    0x010 // Cache Disabled
#define MM_ATTR_A     0x020 // Accessed
#define MM_ATTR_D     0x040 // Dirty (for Pages)
#define MM_ATTR_AVL   0x040 // Available (for Directory)
#define MM_ATTR_PAT   0x080 // Page Table Attribute Index (for Pages)
#define MM_ATTR_SZ_4K 0x000 // Page Size (for Directory)
#define MM_ATTR_SZ_4M 0x080 // Page Size (for Directory)
#define MM_ATTR_G     0x100 // Global (ignored for Directory)

#define MM_ATTR_USR   0xE00 // bits for kernel

#define MM_NOCREAT 0x0
#define MM_CREAT   0x1

#define MM_VALID   0x100
#define MM_SHARED  0x200
#define MM_VM      0x400

/* Control Register flags */
#define CR0_PE		0x00000001	// Protection Enable
#define CR0_MP		0x00000002	// Monitor coProcessor
#define CR0_EM		0x00000004	// Emulation
#define CR0_TS		0x00000008	// Task Switched
#define CR0_ET		0x00000010	// Extension Type
#define CR0_NE		0x00000020	// Numeric Errror
#define CR0_WP		0x00010000	// Write Protect
#define CR0_AM		0x00040000	// Alignment Mask
#define CR0_NW		0x20000000	// Not Writethrough
#define CR0_CD		0x40000000	// Cache Disable
#define CR0_PG		0x80000000	// Paging

#define CR4_PCE		0x00000100	// Performance counter enable
#define CR4_MCE		0x00000040	// Machine Check Enable
#define CR4_PSE		0x00000010	// Page Size Extensions
#define CR4_DE		0x00000008	// Debugging Extensions
#define CR4_TSD		0x00000004	// Time Stamp Disable
#define CR4_PVI		0x00000002	// Protected-Mode Virtual Interrupts
#define CR4_VME		0x00000001	// V86 Mode Extensions

#define PAGE_SIZE    0x1000
#define KERNEL_PAGES 0x300000
#define MEM_LIMIT    0x400000

typedef struct str_mm_page {
	u32_t attr:12;
	u32_t base:20;
}  __attribute__((__packed__, aligned (4))) mm_page;

#define make_mm_entry(base, attr) (mm_page){(u32_t) (attr), (u32_t) (base)}
#define make_mm_entry_addr(addr, attr) (mm_page){(u32_t) (attr), \
                                                 (u32_t) (addr) >> 12}

#include <sys/queue.h>
#include <minios/const.h>

#define PAGES_LEN ((MEM_LIMIT - KERNEL_PAGES) / PAGE_SIZE)
#define hash_page(base) ((((u32_t) (base)) - KERNEL_PAGES) / PAGE_SIZE)

struct page_s {
    /* starting address */
    void *base;

    /* ref count */
    int refcount;

    /* free pages list */
    LIST_ENTRY(page_s) status;
} __attribute__((__packed__));

extern struct page_s pages[PAGES_LEN];

void mm_init();
void *mm_mem_alloc();
void *mm_mem_kalloc();
void *mm_build_page(mm_page *dir, void *vir, void *boot_page);
void mm_mem_free_reference(void *page);

mm_page *mm_dir_new();
mm_page *mm_dir_cpy(mm_page *dir);
void mm_dir_free(mm_page *d);

void mm_map_page(mm_page *dir, void *vir, void *phy);
void mm_map_page_attr(mm_page *dir, void *vir, void *phy, int attr);
void mm_umap_page(mm_page *dir, void *vir);

void *sys_palloc();
int sys_share_page(void *page);

#endif /* _MM_H */
