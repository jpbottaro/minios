%define KORG 0x200000

global print

section .text

; ------- code

print:
    mov esi, [esp + 4]
    mov ecx, 5
    call print_msg
    ret

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
