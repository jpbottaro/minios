%define KORG 0x200000

global write

section .text

; ------- code

write:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov edx, [ebp + 16]
    mov ecx, [ebp + 12]
    mov ebx, [ebp + 8]
    mov eax, 4
    int 0x80
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
