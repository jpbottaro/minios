BITS 16

%define KORG 0x1200

global start
global idle
global panic
global print_msg
global KSTACK

extern gdt
extern GDT_DESC
extern IDT_DESC
extern init_idt
extern init_scall
extern init_mmu
extern init_directory
extern init_scheduler
extern init_fs
extern reset_pic
extern enable_pic
extern disable_pic
extern sys_newprocess
extern clear_screen

; ------- code

start:
    lea ebp, [KSTACK + KSTACKSIZE]
    lea esp, [KSTACK + KSTACKSIZE]

    ; enable A20
    call disable_A20
    ;call check_A20
    call enable_A20
    ;call check_A20

    ; disable interruptions
    cli

    ; set GDT e IDT
    lgdt [GDT_DESC]
    lidt [IDT_DESC]

    ; set bit PE in CR0
    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax

    ; jump to protected mode
    jmp 0x08:protectedmode

bits 32
protectedmode:
    mov ax, 0x10    ; data segment (3rd gdt entry) 
    mov ds, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax

    mov ax, 0x18    ; video data segment (4th gdt entry)
    mov es, ax

    ; set stack
    lea ebp, [KSTACK + KSTACKSIZE]
    lea esp, [KSTACK + KSTACKSIZE]

    ; init IDT
    call init_idt

    ; init system calls table
    call init_scall

    ; MMU
    call init_mmu

    ; ident mapping
    call init_directory
    mov  cr3, eax

    ; enable paging
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; pic
    call disable_pic
    call reset_pic
    call enable_pic

    ; init fs
    push dword FS_INITIAL_POS
    call init_fs
    add esp, 4

    ; init scheduler
    call init_scheduler

    ; add cash shell to the process table
    push CASH_PATH
    call sys_newprocess
    add esp, 4

    call clear_screen

    ; enable interruptions (and therefore scheduler)
    sti

    jmp $

idle:
    mov esi, IDLEMSG
    mov ecx, IDLEMSG_len
    call print_msg
    jmp $
    ; end of kernel code

; ------- functions

; print msg and panic
panic:
    cli
    mov esi, [esp + 4]
    call len_msg
    call print_msg
    jmp $

; return length of msg in ecx, msg is in esi
len_msg:
    mov ecx, 0
    .loop:
        cmp byte [esi + ecx], 0
        je .end
        inc ecx
        jmp .loop
 .end:
    ret

; print msg in esi, ecx chars
print_msg:
	mov ax, 0x18
	mov es, ax      ; video segment
	mov edi, 0
	mov ah, 0x1c    ; blue background, red letters, blink
	.loop:
		lodsb
		stosw
		loop .loop
	ret

%include "a20.asm"

; ------- data

KSTACKSIZE: equ 0x1FF0
KSTACK: resb 0x2000

FS_INITIAL_POS: equ 0x20000

CASH_PATH: db "/bin/cash", 0

IDLEMSG: db "Estamos en idleeee"
IDLEMSG_len: equ $-IDLEMSG
