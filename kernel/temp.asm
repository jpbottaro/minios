global panic
global print_msg

; print msg and panic
panic:
    cli
    mov esi, [esp + 4]
    call len_msg
    call print_msg
    jmp $

; return length of msg in ecx, msg is in esi
len_msg:
    mov ecx, 0
    .loop:
        cmp byte [esi + ecx], 0
        je .end
        inc ecx
        jmp .loop
 .end:
    ret

; print msg in esi, ecx chars
print_msg:
	mov ax, 0x28
	mov es, ax      ; video segment
	mov edi, 0
	mov ah, 0x1c    ; blue background, red letters, blink
	.loop:
		lodsb
		stosw
		loop .loop
	ret
