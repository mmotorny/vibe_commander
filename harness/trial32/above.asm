; What a program of real mode is told of the memory above the megabyte
; when it is run as a command: by the program of 16 bits, where it is told
; what the BIOS has, and by the program of 32 bits, whose extender has the
; memory then. DOS's MEM goes by these, and the scenario `command` shows
; what it makes of them (TODO.md, "C2"). It writes by DOS, in hex:
;
;   88    AX behind INT 15h function 88h, the KB above the megabyte, and
;         the carry flag, NC or CY
;   E801  AX, BX, CX and DX behind INT 15h with E801h, and the carry
;   E820  for each stretch of the memory that INT 15h with E820h has,
;         where it begins, how long it is, both as their low 32 bits,
;         and its kind, 1 for memory; or the carry, if the call is
;         refused
;   CMOS  the KB above the megabyte that the clock's memory has, at 17h
;         and at 30h
;   XMS   AX behind INT 2Fh with 4300h, 80h in AL where there is a
;         driver
;   A20   1 if the memory above the megabyte is there for real mode: a
;         byte at FFFF:0510 is not the one at 0000:0500
;   DPMI  AX behind INT 2Fh with 1687h, 0 where there is a host
;
; Run it as a command of the program, with the file next to the program:
;
;   build.sh above com
;   run.py --extra VC/ABOVE.COM=build/trial32/above.com \
;       build/portable_c/VC.EXE '"d:\vc\above" Enter' Ctrl+o
;
; and with --himem for a machine that has an XMS driver, where MEM typed
; behind it says how much of that memory a command is left.
	.model	tiny
	.386
	.code
	org	100h
Start:	cld
	mov	dx,offset T88
	call	Say
	mov	ax,8800h
	clc
	int	15h
	pushf
	call	PWord
	popf
	call	Carry
	call	NewLn

	mov	dx,offset TE801
	call	Say
	mov	ax,0E801h
	xor	bx,bx
	xor	cx,cx
	xor	dx,dx
	clc
	int	15h
	pushf
	push	dx
	push	cx
	push	bx
	call	PWord
	pop	ax
	call	PWord
	pop	ax
	call	PWord
	pop	ax
	call	PWord
	popf
	call	Carry
	call	NewLn

	xor	ebx,ebx
E820a:	mov	dx,offset TE820
	call	Say
	mov	eax,0E820h
	mov	edx,534D4150h
	mov	ecx,20
	mov	di,offset Buf
	push	cs
	pop	es
	clc
	int	15h
	pushf
	jc	E820c
	cmp	eax,534D4150h
	jne	E820c
	push	ebx
	mov	eax,dword ptr Buf
	call	PDWord
	mov	eax,dword ptr Buf+8
	call	PDWord
	mov	ax,word ptr Buf+16
	call	PWord
	pop	ebx
	popf
	call	NewLn
	or	ebx,ebx
	jnz	E820a
	jmp	E820d
E820c:	popf
	call	Carry
	call	NewLn
E820d:
	mov	dx,offset TCmos
	call	Say
	mov	al,18h
	call	Cmos
	mov	ah,al
	push	ax
	mov	al,17h
	call	Cmos
	pop	bx
	mov	ah,bh
	call	PWord
	mov	al,31h
	call	Cmos
	mov	ah,al
	push	ax
	mov	al,30h
	call	Cmos
	pop	bx
	mov	ah,bh
	call	PWord
	call	NewLn

	mov	dx,offset TXms
	call	Say
	mov	ax,4300h
	int	2Fh
	call	PWord
	mov	dx,offset TA20
	call	Say
	xor	ax,ax
	mov	es,ax
	mov	ax,0FFFFh
	mov	fs,ax
	mov	al,es:[500h]
	mov	ah,al
	not	ah
	cli
	mov	es:[500h],ah
	cmp	fs:[510h],ah
	mov	es:[500h],al
	sti
	mov	ax,0
	je	A20x
	inc	ax
A20x:	call	PWord
	mov	dx,offset TDpmi
	call	Say
	mov	ax,1687h
	int	2Fh
	call	PWord
	call	NewLn
	mov	ax,4C00h
	int	21h

Cmos:	out	70h,al
	jmp	$+2
	in	al,71h
	ret

Say:	push	cs
	pop	ds
	mov	ah,9
	int	21h
	ret

NewLn:	mov	dx,offset TNl
	jmp	Say

Carry:	mov	dx,offset TNc
	jnc	Say
	mov	dx,offset TCy
	jmp	Say

PDWord:	push	eax
	shr	eax,16
	call	Word0
	pop	eax
PWord:	call	Word0
	mov	dl,' '
	mov	ah,2
	int	21h
	ret
Word0:	push	ax
	mov	al,ah
	call	Byte0
	pop	ax
Byte0:	push	ax
	shr	al,4
	call	Nib
	pop	ax
Nib:	and	al,0Fh
	add	al,90h
	daa
	adc	al,40h
	daa
	mov	dl,al
	mov	ah,2
	int	21h
	ret

T88	db	'88 $'
TE801	db	'E801 $'
TE820	db	'E820 $'
TCmos	db	'CMOS $'
TXms	db	'XMS $'
TA20	db	'A20 $'
TDpmi	db	'DPMI $'
TNc	db	'NC$'
TCy	db	'CY$'
TNl	db	13,10,'$'
Buf	db	24 dup (0)
	end	Start
