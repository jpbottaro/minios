global save_process_state
global load_process_state
global __pm_switchto
global reip
extern last_process

; this works because save and load have the same stack call size (only the
; process_state pointer and return address)
; this way when we load esp/ebp and the page dir., we get the stack with esp
; pointing to the return addres of _load_.

%define KERN_CODE     0x8
%define KERN_DATA     0x10
%define USER_CODE     0x1B
%define USER_DATA     0x23
%define DATA_RUN      0x08
%define DATA_EIP      0x0C
%define DATA_ESP      0x10
%define DATA_EBP      0x14
%define DATA_DIRPAGES 0x18
%define DATA_UID      0x20
%define EFLAGS_MASK   0x00000202

reip:
    pop eax
    jmp eax

__pm_switchto:
    mov ecx, [last_process]
    cmp ecx, 0
    je load_process_state
save_process_state:
    mov eax, [last_process]
    mov [eax + DATA_EBP], ebp
    mov [eax + DATA_ESP], esp
    mov edx, cr3
    mov [eax + DATA_DIRPAGES], edx
    mov dword [eax + DATA_EIP], end_pm_switchto
load_process_state:
    mov eax, [esp + 4]
    mov ebp, [eax + DATA_EBP]
    mov edx, [eax + DATA_DIRPAGES]
    mov cr3, edx
    cmp dword [eax + DATA_UID], 1
    je root
user:
    mov edx, USER_DATA
    mov ds, edx
    mov es, edx
    mov fs, edx
    mov gs, edx
    cmp dword [eax + DATA_RUN], 1
    je finish_line
    push edx
    push dword [eax + DATA_ESP]
    push dword EFLAGS_MASK
    push dword USER_CODE
    jmp go_on
root:
    mov edx, KERN_DATA
    mov ds, edx
    mov es, edx
    mov fs, edx
    mov gs, edx
    cmp dword [eax + DATA_RUN], 1
    je finish_line
    mov esp, [eax + DATA_ESP]
    push dword EFLAGS_MASK
    push dword KERN_CODE
go_on:
    mov dword [eax + DATA_RUN], 1
    push dword [eax + DATA_EIP]
    iret
finish_line:
    mov esp, [eax + DATA_ESP]
    push dword [eax + DATA_EIP]
end_pm_switchto:
    ret
