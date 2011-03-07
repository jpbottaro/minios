%define KORG 0x200000

global start

section .text

; ------- code

start:
    mov esi, MSG
    mov ecx, MSG_len
    call print_msg
    jmp $

; ------- functions

; print msg in esi, ecx chars
print_msg:
	mov ax, 0x18
	mov es, ax      ; video segment
	mov edi, 0
	mov ah, 0x1c    ; blue background, red letters, blink
	.loop:
		lodsb
		stosw
		loop .loop
	ret

; ------- data

MSG: db "Si esto aparece en la pantalla me muerooooo"
MSG_len: equ $-MSG
