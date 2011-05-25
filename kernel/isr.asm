BITS 32

extern reset_intr_pic1
extern kbd_key
extern system_calls

global _isr128  ; sys_call (int 0x80)

_isr128:
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
