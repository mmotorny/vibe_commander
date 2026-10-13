; When DOS enters a program of real mode on the bench: the BIOS's ticks,
; the timer's counter 0 latched with its status, and the ACPI timer,
; written to the third row of the screen. start.c writes the same for a
; program that an extender starts; the ACPI timer is the one to hold
; against it, since the extender sets the other anew.
;
; Run it under the program's name: run.py --name VC.COM.
	.model	tiny
	.386
	.code
	org	100h
Start:	mov	dx,608h
	in	eax,dx
	mov	esi,eax
	cli
	mov	al,0C2h
	out	43h,al
	in	al,40h
	mov	bl,al
	shl	ebx,16
	in	al,40h
	mov	bl,al
	in	al,40h
	mov	bh,al			; EBX = the status and the count
	xor	ax,ax
	mov	es,ax
	mov	edx,es:[46Ch]
	sti
	mov	ax,0B800h
	mov	es,ax
	mov	di,2*80*2
	call	Hex8
	mov	edx,ebx
	call	Hex8
	mov	edx,esi
	call	Hex8
	mov	ax,4C00h
	int	21h
Hex8:	mov	cx,8
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
	add	di,2
	ret
	end	Start
