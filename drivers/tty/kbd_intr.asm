global kbd_intr
extern reset_intr_pic1
extern kbd_key

kbd_intr:
    pushad
    xor eax, eax
    in al, 0x60
    push eax
    call kbd_key
    add esp, 4
    call reset_intr_pic1
    popad
    iret
