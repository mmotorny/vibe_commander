; DOS ends a program by another's hand: what is left of a program of 32
; bits then, and of its extender? A program of real mode that does what
; p.com of the scenario `ended` does (harness/scenarios.py): it takes
; INT 28h, runs D:\VC\T.EXE with 1111h in SI and 2222h in CX, and at the
; first idle call that comes while another program than itself is the
; one that runs, it has DOS end that program, AH 4Ch with the number 9.
; DOS goes on where the ended program's PSP says, which is here, behind
; the call that ran it, unless the program has taken that.
;
; There it gives INT 28h back and writes, where the cursor is, in hex:
;
;   first row    the registers as they came back: AX, CX, SI, DX, DS, ES
;                and the flags
;   second row   its own PSP; the PSP of the program that was ended; how
;                many idle calls it saw; what DOS says of how the
;                program ended, AH 4Dh, 0009 for ended by AH 4Ch with 9;
;                DOS's largest free block, in paragraphs; the KB above
;                the megabyte, INT 15h function 88h; the machine's
;                status word, whose lowest bit is clear in real mode;
;                and whether DOS looks for Ctrl-C at every call, AX
;                3300h, 0001 if it does: the bench has it on, and the
;                program switches it off while it runs
;   from the     every interrupt of real mode that does not go where it
;   third on     went before the program was run: its number, and where
;                it goes now
;
; Run it under the program's name, with the program to be ended next to
; it:
;
;   run.py --name VC.COM --extra VC/T.EXE=build/trial32/sys-32.exe \
;       build/trial32/ended.com '"ver" Enter' '"d:\vc\t" Enter' q
;
; The steps behind it are typed at DOS's prompt, to see what the machine
; still does. The program of the portable C is run the same way, as it
; stands at its first screen: --extra VC/T.EXE=build/portable_c32/VC.EXE,
; or build/portable_c/VC.EXE for the one of 16 bits, and no step for it.
;
; The word behind the letters SKIP in the file is how many idle calls of
; the other program it lets by before it has DOS end that one: 0 as it
; is assembled, for the first.
	.model	tiny
	.386
	.code
	org	100h
Start:	mov	sp,1000h
	mov	bx,100h			; Keep 4K of the memory: ES is the PSP
	mov	ah,4Ah
	int	21h
	cld
	xor	si,si			; The 256 vectors, kept
	mov	ds,si
	push	cs
	pop	es
	mov	di,offset Before
	mov	cx,512
	rep movsw
	push	cs
	pop	ds
	mov	ax,3528h
	int	21h
	mov	word ptr Old28,bx
	mov	word ptr Old28+2,es
	mov	dx,offset New28
	mov	ax,2528h
	int	21h
	push	cs
	pop	es
	mov	word ptr Block+4,cs	; The tail is this program's own
	mov	bx,offset Block
	mov	dx,offset Child
	mov	si,1111h
	mov	cx,2222h
	mov	ax,4B00h
	int	21h

	pushf				; Back, by whatever way
	push	es
	push	ds
	push	dx
	push	si
	push	cx
	push	ax
	cld
	lds	dx,cs:Old28
	mov	ax,2528h
	int	21h
	push	cs
	pop	ds
	call	NewLine
	mov	Count,7
Back:	pop	dx
	call	Hex4
	dec	Count
	jnz	Back
	call	NewLine
	mov	dx,cs
	call	Hex4
	mov	dx,Ended
	call	Hex4
	mov	dx,Idles
	call	Hex4
	mov	ah,4Dh
	int	21h
	mov	dx,ax
	call	Hex4
	mov	ah,48h
	mov	bx,0FFFFh
	int	21h
	mov	dx,bx
	call	Hex4
	mov	ah,88h
	int	15h
	mov	dx,ax
	call	Hex4
	smsw	dx
	call	Hex4
	mov	ax,3300h
	int	21h
	xor	dh,dh
	call	Hex4
	call	NewLine
	xor	si,si			; The vectors that are others now
	mov	es,si
	mov	Count,0
Vector:	mov	ax,es:[si]
	cmp	ax,word ptr Before[si]
	jne	Other
	mov	ax,es:[si+2]
	cmp	ax,word ptr Before[si+2]
	je	Same
Other:	mov	dx,Count
	xchg	dl,dh
	call	Hex2
	mov	al,':'
	call	Char
	mov	dx,es:[si+2]
	call	Hex4
	mov	dx,es:[si]
	call	Hex4
	call	NewLine
Same:	add	si,4
	inc	Count
	cmp	Count,256
	jb	Vector
	mov	ax,4C00h
	int	21h

; INT 28h: the first time that it comes while another program is the one
; that runs, DOS is told to end that program.
New28:	push	ax
	push	bx
	push	ds
	push	cs
	pop	ds
	inc	Idles
	mov	ah,62h
	int	21h
	mov	ax,cs
	cmp	bx,ax
	je	Ours
	cmp	Skip,0			; Not yet: so many are let by
	je	Now
	dec	Skip
	jmp	short Ours
Now:	mov	Ended,bx
	mov	ax,4C09h
	int	21h
Ours:	pop	ds
	pop	bx
	pop	ax
	jmp	dword ptr cs:Old28

Hex2:	mov	cx,2
	jmp	short Digit
Hex4:	mov	cx,4
Digit:	rol	dx,4
	mov	al,dl
	and	al,15
	add	al,'0'
	cmp	al,'9'
	jbe	Write
	add	al,7
Write:	call	Char
	loop	Digit
	mov	al,' '
Char:	push	bx
	push	cx
	push	dx
	push	si
	mov	ah,0Eh
	xor	bx,bx
	int	10h
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
NewLine:
	mov	al,13
	call	Char
	mov	al,10
	jmp	Char

Old28	dd	0
Idles	dw	0
Ended	dw	0
Count	dw	0
	db	'SKIP'
Skip	dw	0
Child	db	'D:\VC\T.EXE',0
Block	dw	0, 80h, 0, 0, 0, 0, 0
Before	label	word
	end	Start
