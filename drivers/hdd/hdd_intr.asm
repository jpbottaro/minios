global hdd_intr
extern reset_intr_pic1
extern reset_intr_pic2
extern hdd_handler

hdd_intr:
    pushad
    call hdd_handler
    call reset_intr_pic1
    call reset_intr_pic2
    popad
    iret
