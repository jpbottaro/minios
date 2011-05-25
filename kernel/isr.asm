BITS 32

extern reset_intr_pic1
extern sched_schedule
extern kbd_key
extern system_calls

global _isr32	; clock
global _isr128  ; sys_call (int 0x80)

_isr32:
    pushad
    call reset_intr_pic1
    push 0
    call sched_schedule
    add esp, 4
    popad
    iret
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
