BITS 16

%include "macrosmodoreal.mac"

%define KORG 0x1200

global start
global idle
global panic
global print_msg
extern gdt
extern GDT_DESC
extern IDT_DESC
extern init_idt
extern init_scall
extern init_mmu
extern init_dir_kernel
extern init_dir_usuario
extern init_scheduler
extern init_fs
extern reset_pic
extern enable_pic
extern disable_pic

; ------- code

start:

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

    ; init IDT
    call init_idt

    ; init system calls table
    call init_scall

    ; MMU
    call init_mmu

    ; ident mapping
    call init_dir_kernel
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
    push dword [fs_initial_pos]
    call init_fs
    pop eax

    ; init scheduler
    call init_scheduler

    sti

idle:
    jmp $

    ; end of kernel code

; ------- data

fs_initial_pos: equ 0x20000-0x1200

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
	mov ah, 0x9c    ; blue background, red letters, blink
	.loop:
		lodsb
		stosw
		loop .loop
	ret

%include "a20.asm"
