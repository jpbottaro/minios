BITS 32

extern reset_intr_pic2
extern clock_handler

global _clock_handler

_clock_handler:
    pushad
    call clock_handler
    call reset_intr_pic2
    popad
    iret
