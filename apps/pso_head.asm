; PSO head
; Header of a PSO file

extern main
extern _end
extern __pso_end_disk
extern exit

section .text
__pso_start:
    db "PSO",0        ; PSO magic number
    dd 0x400000       ;dd __pso_start ; Start of virtual space
    dd __pso_end_disk ; End of virtual space in disk
    dd __pso_end_disk ;dd _end ; End of virtual space used by the app (eg bss)
    dd _start         ; Entry point (this will call main(void))

_start:
    call main
    push 0
    call exit
