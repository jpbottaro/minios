%define KORG 0x200000

global exit
global _exit
global read
global write
global open
global close
global waitpid
global unlink
global newprocess
global chdir
global lseek
global getpid
global rename
global mkdir
global rmdir
global get_dents

; ------- code

exit:
_exit:
    mov eax, 1
    jmp next
read:
    mov eax, 3
    jmp next
write:
    mov eax, 4
    jmp next
open:
    mov eax, 5
    jmp next
close:
    mov eax, 6
    jmp next
waitpid:
    mov eax, 7
    jmp next
unlink:
    mov eax, 10
    jmp next
newprocess:
    mov eax, 11
    jmp next
chdir:
    mov eax, 12
    jmp next
lseek:
    mov eax, 19
    jmp next
getpid:
    mov eax, 20
    jmp next
rename:
    mov eax, 38
    jmp next
mkdir:
    mov eax, 39
    jmp next
rmdir:
    mov eax, 40
    jmp next
get_dents:
    mov eax, 141
    jmp next

next:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov edx, [ebp + 16]
    mov ecx, [ebp + 12]
    mov ebx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
