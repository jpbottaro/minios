BITS 32

extern reset_intr_pic1
extern system_calls

global scall_handler

scall_handler:
    pushad
    push edx
    push ecx
    push ebx
    sti
    mov eax, [system_calls + eax * 4]
    call eax
    mov [res], eax
    add esp, 12
    call reset_intr_pic1
    popad
    mov eax, [res]
    iret

res: dd 0
