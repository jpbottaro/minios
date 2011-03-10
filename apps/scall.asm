%define KORG 0x200000

global write

section .text

; ------- code

write:
    mov edx, [esp + 12]
    mov ecx, [esp + 8]
    mov ebx, [esp + 4]
    mov eax, 4
    int 0x80
    ret

; ------- functions


