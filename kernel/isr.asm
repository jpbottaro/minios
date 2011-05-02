BITS 32

extern reset_intr_pic1
extern sched_schedule
extern kbd_key
extern print_msg
extern system_calls

global _isr0
global _isr1
global _isr2
global _isr3
global _isr4
global _isr5
global _isr6
global _isr7
global _isr8
global _isr9
global _isr10
global _isr11
global _isr12
global _isr13
global _isr14
global _isr15
global _isr16
global _isr17
global _isr18
global _isr19
global _isr32	; clock
global _isr33   ; keyboard
global _isr128  ; sys_call (int 0x80)

_isr0:
    mov esi, DE
    mov ecx, DE_len
    call print_msg
    jmp $
_isr1:
    mov esi, DB
    mov ecx, DB_len
    call print_msg
    jmp $
_isr2:
    mov esi, NMI
    mov ecx, NMI_len
    call print_msg
    jmp $
_isr3:
    mov esi, _BP
    mov ecx, _BP_len
    call print_msg
    jmp $
_isr4:
    mov esi, OF
    mov ecx, OF_len
    call print_msg
    jmp $
_isr5:
    mov esi, BR
    mov ecx, BR_len
    call print_msg
    jmp $
_isr6:
    mov esi, UD
    mov ecx, UD_len
    call print_msg
    jmp $
_isr7:
    mov esi, NM
    mov ecx, NM_len
    call print_msg
    jmp $
_isr8:
    mov esi, DF
    mov ecx, DF_len
    call print_msg
    jmp $
_isr9:
    mov esi, CSO
    mov ecx, CSO_len
    call print_msg
    jmp $
_isr10:
    mov esi, TS
    mov ecx, TS_len
    call print_msg
    jmp $
_isr11:
    mov esi, NP
    mov ecx, NP_len
    call print_msg
    jmp $
_isr12:
    mov esi, _SS
    mov ecx, _SS_len
    call print_msg
    jmp $
_isr13:
    mov esi, GP
    mov ecx, GP_len
    call print_msg
    jmp $
_isr14:
    mov esi, PF
    mov ecx, PF_len
    call print_msg
    jmp $
_isr15:
    mov esi, IR
    mov ecx, IR_len
    call print_msg
    jmp $
_isr16:
    mov esi, MF
    mov ecx, MF_len
    call print_msg
    jmp $
_isr17:
    mov esi, AC
    mov ecx, AC_len
    call print_msg
    jmp $
_isr18:
    mov esi, MC
    mov ecx, MC_len
    call print_msg
    jmp $
_isr19:
    mov esi, XM
    mov ecx, XM_len
    call print_msg
    jmp $
_isr32:
    pushad
    call reset_intr_pic1
    push 0
    call sched_schedule
    add esp, 4
    popad
    iret
_isr33:
    pushad
    xor eax, eax
    in al, 0x60
    push eax
    call kbd_key
    add esp, 4
    call reset_intr_pic1
    popad
    iret
_isr128:
    pushad
    push edx
    push ecx
    push ebx
    mov eax, [system_calls + eax * 4]
    call eax
    mov [res], eax
    add esp, 12
    call reset_intr_pic1
    popad
    mov eax, [res]
    iret

res: dd 0

; Protected Mode Exceptions and Interrupts (messages)
DE: db "(*) Fault: Divide Error. #DE"
DE_len: equ $-DE

DB: db "(*) Fault/Trap: Vector 1. #DB (Reserved intel)"
DB_len: equ $-DB

NMI: db "(*) Interrupt: NMI Interrupt. #--"
NMI_len: equ $-NMI

_BP: db "(*) Trap: Breakpoint INT 3. #BP"
_BP_len: equ $-_BP

OF: db "(*) Trap: Overflow. #OF"
OF_len: equ $-OF

BR: db "(*) Fault: BOUND range exceeded. #BR"
BR_len: equ $-BR

UD: db "(*) Fault: Invalid Opcode #UD"
UD_len: equ $-UD

NM: db "(*) Fault: Device Not Available. #NM"
NM_len: equ $-NM

DF: db "(*) Abort: Double Fault."
DF_len: equ $-DF

CSO: db "(*) Fault: Coprocessor Segment Overrun #--."
CSO_len: equ $-CSO

TS: db "(*) Fault: Invalid TTS. #TS"
TS_len: equ $-TS

NP: db "(*) Fault: Segment Not Present. #NP"
NP_len: equ $-NP

_SS: db "(*) Fault: Stack-Segment Fault. #SS"
_SS_len: equ $-_SS

GP: db "(*) Fault: General Protection. #GP"
GP_len equ $-GP

PF: db "(*) Fault: Page Fault. #PF" 
PF_len: equ $-PF

IR: db "(*) ??: Vector 15 - Intel Reserved."
IR_len: equ $-IR

MF: db "(*) Fault: Math Fault. #MF"
MF_len: equ $-MF

AC: db "(*) Fault: Aligment Check. #AC"
AC_len: equ $-AC

MC: db "(*) Abort: Machine Check. #MC"
MC_len: equ $-MC

XM: db "(*) Fault: SIMD Floating-point Exception. #XM"
XM_len: equ $-XM
