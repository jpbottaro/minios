BITS 32

extern reset_intr_pic2
extern pf_handler

global _pf_handler

_pf_handler:
    pushad
    call pf_handler
    call reset_intr_pic2
    popad
    add esp, 4 ; remove error code
    iret
