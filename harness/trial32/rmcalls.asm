; What a call of the BIOS or of DOS costs from real mode on the bench: the
; counts of the ACPI timer around 64 calls of each, written to the third
; row of the screen in hex, for INT 16h functions 11h and 02h, INT 10h
; functions 03h and 0Fh, INT 33h function 3, and INT 21h functions 2Ch,
; 19h, 0Bh and 62h. A count is 279.4 ns and an instruction of the bench's
; machine 256 ns: instructions a call are the count * 2794 / 2560 / 64.
; The ninth count is on the next row. realcalls.c writes the same for the
; calls made from protected mode.
;
; Run it under the program's name: run.py --name VC.COM.
	.model	tiny
	.386
	.code
	org	100h
Start:	mov	di,2*80*2
	mov	si,offset Table
Next:	lodsw				; AX of the call
	or	ax,ax
	jz	Done
	mov	bp,ax
	lodsw				; which interrupt
	mov	Which,al
	call	Clock
	push	eax
	mov	cx,64
Again:	push	cx
	push	si
	push	di
	mov	ax,bp
	xor	bx,bx
	xor	cx,cx
	xor	dx,dx
	db	0CDh
Which	db	0
	pop	di
	pop	si
	pop	cx
	loop	Again
	call	Clock
	pop	edx
	sub	eax,edx
	mov	edx,eax
	push	es
	mov	ax,0B800h
	mov	es,ax
	mov	cx,8
L1:	rol	edx,4
	mov	al,dl
	and	al,15
	add	al,'0'
	cmp	al,'9'
	jbe	D1
	add	al,7
D1:	stosb
	mov	al,7
	stosb
	loop	L1
	add	di,4
	pop	es
	jmp	Next
Done:	mov	ax,4C00h
	int	21h
; EAX = the ACPI timer: 3,579,545 to a second
Clock:	push	dx
	mov	dx,608h
	in	eax,dx
	and	eax,0FFFFFFh
	pop	dx
	ret
Table	dw	1100h, 16h, 0200h, 16h, 0300h, 10h, 0F00h, 10h, 3, 33h
	dw	2C00h, 21h, 1900h, 21h, 0B00h, 21h, 6200h, 21h, 0
	end	Start
