global handler_void
global handler_ignore

handler_void:
    iret

; default handler for unused hw interrupts: acknowledge both PICs and return
handler_ignore:
    push eax
    mov al, 0x20
    out 0x20, al
    out 0xA0, al
    pop eax
    iret
