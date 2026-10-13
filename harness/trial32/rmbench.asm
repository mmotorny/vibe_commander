; What the bench has for a DOS extender to use, written to the third row
; of the screen in hex: AX behind INT 2Fh with 4300h, which has 80h in AL
; where there is XMS; AX behind INT 2Fh with 1687h, 0 where there is
; DPMI; the vector of INT 67h, EMS's; the KB above the megabyte, by INT
; 15h function 88h; the KB below, by INT 12h; and the machine's status
; word, whose lowest bit is clear in real mode.
;
; Run it under the program's name: run.py --name VC.COM.
	.model	tiny
	.386
	.code
	org	100h
Start:	mov	ax,0B800h
	mov	es,ax
	mov	di,2*80*2
	mov	ax,4300h		; XMS: AL 80h if there
	int	2Fh
	mov	dx,ax
	call	Hex4
	mov	ax,1687h		; DPMI: AX 0 if there
	int	2Fh
	mov	dx,ax
	call	Hex4
	push	es
	mov	ax,3567h		; EMS: its vector
	int	21h
	mov	dx,es
	mov	si,bx
	pop	es
	call	Hex4
	mov	dx,si
	call	Hex4
	mov	ah,88h			; KB above the megabyte
	int	15h
	mov	dx,ax
	call	Hex4
	int	12h			; KB below
	mov	dx,ax
	call	Hex4
	smsw	dx			; real mode, or virtual 8086?
	call	Hex4
	mov	ax,4C00h
	int	21h
Hex4:	mov	cx,4
L1:	rol	dx,4
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
	add	di,2
	ret
	end	Start
