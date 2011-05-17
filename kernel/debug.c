#include <minios/i386.h>
#include <minios/idt.h>
#include <minios/pm.h>
#include "debug.h"

#define OPCODE_CALL_REL  0xE8
#define OPCODE_CALL_ADDR 0xFF

#define INTR_FUNC(NAME, NR, ERROR) \
    void isr ## NR() \
    { \
        int off  = ((ERROR) ? 0 : -1); \
        u32_t *ebp = (u32_t *) rebp(); \
        exp_state test = {.eax = reax(), .ebx = rebx(), \
                          .ecx = recx(), .edx = redx(), \
                          .nr = (NR), .name = (NAME), \
                          .es = res(), .ds = rds(), .fs = rfs(), \
                          .gs = rgs(), .ss = rss(), \
                          .edi = redi(), .esi = resi(), \
                          .ebp = rebp(), .esp = resp(), \
                          .errcode = ((ERROR) ? *(ebp + 1) : 0), \
                          .org_eip = *(ebp + (u32_t)(2 + off)), \
                          .org_cs  = *(ebp + (u32_t)(3 + off)), \
                          .eflags  = *(ebp + (u32_t)(4 + off)), \
                          .org_esp = *(ebp + (u32_t)(5 + off)), \
                          .org_ss  = *(ebp + (u32_t)(6 + off)) }; \
        debug_kernelpanic((u32_t *) resp(), &test); \
        for (;;) {} \
    }

INTR_FUNC("DIVIDE ERROR", 0, 0);
INTR_FUNC("DEBUG", 1, 0);
INTR_FUNC("NMI", 2, 0);
INTR_FUNC("BREAKPOINT", 3, 0);
INTR_FUNC("OVERFLOW", 4, 0);
INTR_FUNC("BOUND RANGE", 5, 0);
INTR_FUNC("INVALID OPCODE", 6, 0);
INTR_FUNC("DEVICE NOT AVAILABLE", 7, 0);
INTR_FUNC("DOBLE FAULT", 8, 1);
INTR_FUNC("COPROCESSOR", 9, 0);
INTR_FUNC("INVALID TSS", 10, 1);
INTR_FUNC("SEGMENT NOT PRESENT", 11, 1);
INTR_FUNC("STACK FAULT", 12, 1);
INTR_FUNC("GENERAL PROTECTION", 13, 1);
INTR_FUNC("PAGE FAULT", 14, 1);
INTR_FUNC("FPU", 16, 0);
INTR_FUNC("ALIGNMENT", 17, 1);
INTR_FUNC("MACHINE CHECK", 18, 0);
INTR_FUNC("SIMD", 19, 0);

void debug_panic(const char *msg)
{
    vga_pprintf(24, 0, "PANIC! %s", msg);
    for (;;) {}
}

void debug_init()
{
    idt_register(1, isr1, DEFAULT_PL);
    idt_register(2, isr2, DEFAULT_PL);
    idt_register(3, isr3, DEFAULT_PL);
    idt_register(4, isr4, DEFAULT_PL);
    idt_register(5, isr5, DEFAULT_PL);
    idt_register(6, isr6, DEFAULT_PL);
    idt_register(7, isr7, DEFAULT_PL);
    idt_register(8, isr8, DEFAULT_PL);
    idt_register(9, isr9, DEFAULT_PL);
    idt_register(10, isr10, DEFAULT_PL);
    idt_register(11, isr11, DEFAULT_PL);
    idt_register(12, isr12, DEFAULT_PL);
    idt_register(13, isr13, DEFAULT_PL);
    idt_register(14, isr14, DEFAULT_PL);
    idt_register(16, isr16, DEFAULT_PL);
    idt_register(17, isr17, DEFAULT_PL);
    idt_register(18, isr18, DEFAULT_PL);
    idt_register(19, isr19, DEFAULT_PL);
}

u32_t call_address(u8_t *instr)
{
    switch (*instr) {
        case 0xE8:
            return *((u32_t *) (instr + 1));
        case 0xFF:
            /* HACER */
        default:
            break;
    }
    return 0;
}

void debug_kernelpanic(const u32_t* stack, const exp_state* expst)
{
    int i;
    const u32_t *st;
    u32_t *p;
    u32_t address;

    vga_init();

    vga_pprintf(0, 0, "Kernel Panic!");

    /* STACK */
    vga_pprintf(2, 0, "Stack:");
    for (i = 0; i < 14; ++i) {
        st = (stack + i * 4);
        vga_pprintf(3 + i, 0, "%8x:%8x %8x %8x %8x",
                st, *st, *(st + 1), *(st + 2), *(st + 3));
        vga_pwrite(3 + i, 45, (char *) st, 16);
    }

    /* REGISTERS */
    vga_pprintf(2, 62, "EAX %8x", expst->eax);
    vga_pprintf(3, 62, "EBX %8x", expst->ebx);
    vga_pprintf(4, 62, "ECX %8x", expst->ecx);
    vga_pprintf(5, 62, "EDX %8x", expst->edx);
    vga_pprintf(6, 62, "ESI %8x", expst->esi);
    vga_pprintf(7, 62, "EDI %8x", expst->edi);
    vga_pprintf(8, 62, "EBP %8x", expst->ebp);
    vga_pprintf(9, 62, "ESP %4x:%8x", expst->org_ss, expst->org_esp);
    vga_pprintf(10, 62, "EIP %4x:%8x", expst->org_cs, expst->org_eip);
    vga_pprintf(11, 62, "EFL %8x", expst->eflags);
    vga_pprintf(13, 62, "TR  %8x", rtr());
    vga_pprintf(14, 62, "CR2 %8x", rcr2());
    vga_pprintf(15, 62, "CR3 %8x", rcr3());
    vga_pprintf(16, 62, "PID %8x", current_pid());

    /* BACKTRACE */
    p = (u32_t *) expst->ebp;
    vga_pprintf(18, 0, "Backtrace: Current: %8x", expst->org_eip);
    for (i = 19; i < 19; ++i) {
        address = call_address((u8_t *) *(p - 1));
        vga_pprintf(i, 0, "At %8x: CALL %8x %s",
                    *(p - 1), address, address == 0 ? "<Invalid address>" : "");
        p = (u32_t *) *p;
        if (((u32_t)p & ~0x3FF) != (expst->ebp & ~0x3FF))
            break;
    }

    /* Error */
    vga_pprintf(24, 0, "EXP %2x err: %8x %s",
                expst->nr, expst->errcode, expst->name);
}
