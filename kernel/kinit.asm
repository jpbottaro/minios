bits 16
section .text

global start
global idle_startpoint
global KSTACK
global KSTACKSIZE

extern GDT_DESC
extern IDT_DESC
extern kernel_init

; start MUST be at the very begining of this file
start: 
    call enable_A20

    lgdt [GDT_DESC]
    lidt [IDT_DESC]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x08:modo_protegido

BITS 32
modo_protegido:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov eax, [KSTACKSIZE]
    lea ebp, [KSTACK + eax]
    lea esp, [KSTACK + eax]

    jmp kernel_init

idle_startpoint:
    jmp $

%include "a20.asm"

; ------- data
section .data
KSTACKSIZE:   dw 0x0FF0

section .bss
KSTACK:     resb 0x1000
