global load_process
extern task_desc

; load new process jumping to its tss entry in the gdt
load_process:
    mov ebx, [esp + 4]
    push ebx
    call task_desc
    pop ebx
    mov [desc], ax
    jmp far [addr]
    ret

addr: dd 0
desc: dw 0
