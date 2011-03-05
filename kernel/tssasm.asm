global load_process
extern task_desc

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
