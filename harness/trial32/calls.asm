; Which calls of DOS does a program make, in which order, and with which
; segments? A program of real mode that takes INT 21h, runs D:\VC.EXE,
; and writes down every call that comes by while that runs, as DOS gets
; it: AX, BX, DS and ES, eight bytes for a call, 4,000 of them at the
; most. When the program has ended, it gives the interrupt back and
; writes what it has to D:\CALLS.BIN: a word with the count, and the
; calls.
;
; One call it does not write down, AH 62h, which the driver of the
; tests makes at every idle call of the program, to know whose call it
; is: it counts those, in the second word of the file. The program of
; the portable C does not make that call.
;
; What it sees of a program of 32 bits is what the extender hands DOS in
; real mode, which is what DOS sees: the extender takes INT 21h of real
; mode when it starts, in front of this, and a call of the program's
; comes down from protected mode to the interrupt as this has it. The
; calls that the extender makes for itself, to read the program's file,
; are among them, at the start.
;
; Run it under the program's name, so that the driver of the tests takes
; the program that it runs for the one to drive, with that program in
; the root of the drive; the steps go to the program, and the last has
; to leave it:
;
;   run.py --name VC.COM --extra VC.EXE=build/portable_c32/VC.EXE \
;       --keep build/trial32/calls build/trial32/calls.com \
;       '"ver" Enter' 'F10 Enter'
;
; and read the file from the disk that was kept, build/trial32/calls.D.img.
; The program finds no setup file next to itself there and starts on its
; defaults.
	.model	tiny
	.386
	.code
	org	100h
Most	equ	4000
Start:	mov	sp,offset Top
	mov	bx,(offset Top - offset Start + 100h + 15) / 16
	mov	ah,4Ah			; Keep that much of the memory: ES is
	int	21h			; the PSP
	cld
	mov	ax,3521h
	int	21h
	mov	word ptr Old21,bx
	mov	word ptr Old21+2,es
	mov	dx,offset New21
	mov	ax,2521h
	int	21h
	push	cs
	pop	es
	mov	word ptr Block+4,cs	; The tail is this program's own
	mov	bx,offset Block
	mov	dx,offset Child
	mov	Going,1
	mov	ax,4B00h
	int	21h

	cli				; Back: DOS may have left another stack
	mov	bx,cs
	mov	ds,bx
	mov	es,bx
	mov	ss,bx
	mov	sp,offset Top
	sti
	cld
	mov	Going,0
	push	ds
	lds	dx,cs:Old21
	mov	ax,2521h
	int	21h
	pop	ds
	mov	dx,offset Kept
	xor	cx,cx
	mov	ah,3Ch
	int	21h
	jc	Done
	mov	bx,ax
	mov	cx,Count
	shl	cx,3
	add	cx,4
	mov	dx,offset Count
	mov	ah,40h
	int	21h
	mov	ah,3Eh
	int	21h
Done:	mov	ax,4C00h
	int	21h

; INT 21h, on its way to whoever had it.
New21:	cmp	cs:Going,0
	je	Pass
	cmp	ah,62h
	jne	Note
	inc	cs:Asked
	jmp	short Pass
Note:	push	di
	mov	di,cs:Count
	cmp	di,Most
	jae	Full
	shl	di,3
	mov	cs:Calls[di],ax
	mov	cs:Calls[di+2],bx
	mov	cs:Calls[di+4],ds
	mov	cs:Calls[di+6],es
	inc	cs:Count
Full:	pop	di
Pass:	jmp	dword ptr cs:Old21

Old21	dd	0
Going	db	0
Child	db	'D:\VC.EXE',0
Kept	db	'D:\CALLS.BIN',0
Block	dw	0, 80h, 0, 0, 0, 0, 0
	align	2
Count	dw	0			; These two and the calls are the file
Asked	dw	0
Calls	dw	4 * Most dup (?)
	dw	256 dup (?)		; The stack
Top	label	word
	end	Start
