global serial_intr
extern reset_intr_pic1
extern serial_handler

serial_intr:
    pushad
    call serial_handler
    call reset_intr_pic1
    popad
    iret
