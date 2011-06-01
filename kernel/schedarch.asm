BITS 32

extern reset_intr_pic1
extern reset_intr_pic2
extern sched_schedule

global clock_handler ; clock

clock_handler:
    pushad
    call reset_intr_pic1
    call reset_intr_pic2
    push 0
    call sched_schedule
    add esp, 4
    popad
    iret
