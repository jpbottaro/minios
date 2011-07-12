BITS 32

extern reset_intr_pic1
extern reset_intr_pic2
extern sched_schedule

global clock_handler ; clock

clock_handler:
    pushad
    push 0
    call sched_schedule
    add esp, 4
    call reset_intr_pic2
    popad
    iret
