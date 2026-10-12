; The driver of the tests: a program that stays resident in the machine and
; takes Volkov Commander, the original or a build of the port, through the
; steps of a scenario. See harness/driver.py, which writes its script and
; reads what it writes, and harness/scenarios.py.
;
; It feeds the program and watches it; it answers nothing in place of the
; BIOS, of DOS or of a driver. Keys are scan codes handed to the keyboard
; controller as if the keyboard had sent them (command D2h), so the
; BIOS's INT 9, and the keyboard driver of DOS where one is loaded, make
; of them what they make of a keyboard's: the keys in the BIOS's buffer and
; the shift flags. The mouse's packets go to the controller's auxiliary
; port the same way (command D3h), for the mouse driver. Every call of
; INT 16h and of INT 28h goes on to whoever had the interrupt before.
;
; The program is idle when it calls INT 28h, as LeaveTm does each time
; Input0 has gone round without a key, or when it asks INT 16h to wait for
; a key. When it is idle and the BIOS's keyboard buffer is empty, the next
; event of the script is due. A step has ended when the program has gone
; round twice like that with nothing left to take and nothing that it will
; do by itself after a while (AutoTim): the screen is then written out.
;
; The script is on the third hard disk, which has no file system, from
; its first sector on:
;
;	A5 5A		says that it is a script
;	word		where AutoTim is in the program's main data, or FFFFh
;	word		where Clock is there, or FFFFh for not looking at what
;			blinks
;	word, word	where the program's stack begins in its segment, and
;			how long it is, or 0 for not looking at it
;	6 bytes		the code that follows INT 28h in LeaveTm: by it the
;			program is told from DOS, which calls INT 28h too
;
; and from byte 16 the events:
;
;	01 n bytes	n bytes from the keyboard
;	02 n bytes	n bytes from the mouse
;	03		the step ends
;	04 n		nothing for n more rounds of the program
;	anything else	the script ends
;
; What the driver writes goes to a port that QEMU keeps in a file:
;
;	'R'		the driver is loaded and waits for a script
;	'S' 4005 bytes	the screen at the end of a step: 2000 cells, the
;			cursor's column and row, the two lines of its
;			shape, the video mode
;	'K' word word bytes
;			with it, when the program waits in LeaveTm: the
;			length of its stack, its segment, and what the
;			stack holds, from the bottom up
;	'B' 4005 bytes	with the clock on: the screen as it was seen to
;			change within 20 ticks of the timer after 'S'
;	'E'		the script has ended; the machine is then reset
;
; Until the disk holds a script the driver waits, halted between the
; interrupts: the machine's state is saved like that, and the script is
; there when a machine goes on from it.
;
; Assemble with JWasm: jwasm -bin -Fo DRIVER.COM DRIVER.ASM
	.model	tiny
	.186

OutPort	EQU	0E9h			; QEMU's debug console
Cells	EQU	4000			; Bytes of an 80 x 25 text screen
Events	EQU	16			; Where the events begin in a script
Rounds	EQU	2			; Idle rounds that end a step
Blink	EQU	20			; Ticks to look for what blinks: 1.1 s
FileLen	EQU	2048			; Bytes of the driver's file

	.code
	org	100h
Start:	jmp	near ptr Init

; How many paragraphs of memory the driver keeps beyond its own, so that
; what DOS loads after it lies where the tests want it (see PLACE in
; harness/setups.py). The host sets this in the file, at its fourth byte.
Pad	dw	0

Old16	dd	0
Old28	dd	0
SPos	dw	0			; Where in the script the next byte is
SSec	dw	0FFFFh			; Which sector of it SecBuf holds
Ready	db	0			; The script's header has been read
Told	db	0			; 'R' has been written
Done	db	0			; The script has ended
Idles	db	0			; Rounds the program has idled since
					; anything was fed or seen to change
Hold	db	0			; Rounds still to let pass (event 04)
Prog	db	0			; INT 28h came from the program
Watch	db	0			; Looking for what blinks
WTick	dw	0			; ... since this tick of the timer

; The header of the script, from its third byte.
HAuto	dw	0FFFFh
HClock	dw	0FFFFh
HStack	dw	0
HStLen	dw	0
HSig	db	6 dup (0)

; The packet for INT 13h, AH=42h: one sector to SecBuf.
Packet	db	10h, 0
	dw	1
	dw	offset SecBuf
PktSeg	dw	0
PktLba	dw	0, 0, 0, 0

;---------------------- The script ---------------------
; Read sector AX of the script into SecBuf. Returns CY if that failed.
RdSec	proc	near
	pusha
	push	ds
	push	cs
	pop	ds
	mov	PktLba,ax
	mov	SSec,0FFFFh
	mov	si,offset Packet
	mov	dl,82h
	mov	ah,42h
	int	13h
	jc	RdSec1
	mov	ax,PktLba
	mov	SSec,ax
RdSec1:	pop	ds
	popa
	ret
RdSec	endp

; AL = the byte of the script at SPos, which stays where it is.
Peek	proc	near
	push	si
	mov	ax,cs:SPos
	mov	si,ax
	and	si,1FFh
	shr	ax,9
	cmp	ax,cs:SSec
	je	Peek1
	call	RdSec
Peek1:	mov	al,cs:SecBuf[si]
	pop	si
	ret
Peek	endp

; AL = the byte of the script at SPos, and on to the next.
Next	proc	near
	call	Peek
	inc	cs:SPos
	ret
Next	endp

; Look for a script on the disk. Leaves Ready set if there is one.
Load	proc	near
	push	ax
	xor	ax,ax
	call	RdSec
	jc	Load1
	cmp	word ptr cs:SecBuf,5AA5h
	jne	Load1
	pusha
	push	ds
	push	es
	push	cs
	pop	ds
	push	cs
	pop	es
	mov	si,offset SecBuf+2
	mov	di,offset HAuto
	mov	cx,Events-2
	cld
	rep	movsb
	pop	es
	pop	ds
	popa
	mov	cs:SPos,Events
	mov	cs:Ready,1
Load1:	pop	ax
	ret
Load	endp

;------------------- What is written -------------------
; Write the screen out, after the letter in AL.
PutScr	proc	near
	pushf
	push	ax
	push	cx
	push	dx
	push	si
	push	ds
	cld
	mov	dx,OutPort
	out	dx,al
	mov	ax,0B800h
	mov	ds,ax
	xor	si,si
	mov	cx,Cells
	rep	outsb
	mov	ax,40h
	mov	ds,ax
	mov	si,50h			; The cursor of page 0
	outsb
	outsb
	mov	si,60h			; Its shape
	outsb
	outsb
	mov	si,49h			; The video mode
	outsb
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	ax
	popf
	ret
PutScr	endp

; Write the program's stack out, which SS is on.
PutStk	proc	near
	pushf
	push	ax
	push	cx
	push	dx
	push	si
	push	ds
	cld
	mov	dx,OutPort
	mov	al,'K'
	out	dx,al
	mov	cx,cs:HStLen
	mov	al,cl
	out	dx,al
	mov	al,ch
	out	dx,al
	mov	ax,ss
	out	dx,al
	mov	al,ah
	out	dx,al
	mov	si,cs:HStack
	push	ss
	pop	ds
	rep	outsb
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	ax
	popf
	ret
PutStk	endp

; Keep the screen in Last (AL=0), or compare it with what is kept there
; (AL=1): returns ZR if they are the same.
Screen	proc	near
	push	cx
	push	si
	push	di
	push	ds
	push	es
	mov	cx,0B800h
	mov	ds,cx
	push	cs
	pop	es
	xor	si,si
	mov	di,offset Last
	mov	cx,Cells
	cld
	or	al,al
	jnz	Scrn1
	rep	movsb
	jmp	Scrn2
Scrn1:	repe	cmpsb
Scrn2:	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
	ret
Screen	endp

; AX = the timer's ticks, the low word.
Ticks	proc	near
	push	ds
	mov	ax,40h
	mov	ds,ax
	mov	ax,ds:[6Ch]
	pop	ds
	ret
Ticks	endp

;-------------------- What is fed ----------------------
; Hand the controller the next CX bytes of the script as if the keyboard
; (AH=0D2h) or the mouse (AH=0D3h) had sent them, each when the one
; before has been taken from it.
Feed	proc	near
	jcxz	Feed4
Feed1:	in	al,64h
	test	al,2
	jnz	Feed1
	mov	al,ah
	out	64h,al
Feed2:	in	al,64h
	test	al,2
	jnz	Feed2
	push	ax
	call	Next
	out	60h,al
	pop	ax
Feed3:	in	al,64h
	test	al,1
	jnz	Feed3
	loop	Feed1
Feed4:	ret
Feed	endp

;------------------------ Events -----------------------
; The program is idle: take what the script has next. With AL=1 the
; program is about to wait inside the BIOS for a key, and events are taken
; until there is one, whatever they would otherwise wait for.
Pump	proc	near
	push	bx
	push	cx
	push	ds
	mov	bl,al
	sti
	cmp	cs:Ready,0
	jne	Pump0
	call	Load
	cmp	cs:Ready,0
	jne	Pump0
	cmp	cs:Told,0		; No script yet: say so, once, and
	jne	PumpW			; wait, halted and not counting
	mov	cs:Told,1		; through the machine's clock
	mov	al,'R'
	out	OutPort,al
PumpW:	hlt
	jmp	PumpX
Pump0:	mov	ax,40h
	mov	ds,ax
	mov	ax,ds:[1Ah]		; Keys wait in the BIOS's buffer?
	cmp	ax,ds:[1Ch]
	je	Pump1
	mov	cs:Idles,0
	jmp	PumpX
Pump1:	or	bl,bl
	jnz	Pump2
	cmp	cs:Hold,0
	je	Pump2
	dec	cs:Hold
	jmp	PumpX
Pump2:	call	Peek
	cmp	al,1
	je	PumpK
	cmp	al,2
	je	PumpM
	cmp	al,3
	je	PumpE
	cmp	al,4
	je	PumpH
	mov	al,'E'			; The script has ended
	out	OutPort,al
	mov	cs:Done,1
	mov	al,0FEh			; Reset: QEMU ends at that
	out	64h,al
Pump3:	hlt
	jmp	Pump3
PumpK:	mov	bh,0D2h
	jmp	PumpF
PumpM:	mov	bh,0D3h
PumpF:	inc	cs:SPos
	call	Next
	mov	cl,al
	mov	ch,0
	mov	ah,bh
	call	Feed
	mov	cs:Idles,0
	jmp	PumpN
PumpH:	inc	cs:SPos
	call	Next
	mov	cs:Hold,al
	mov	cs:Idles,0
PumpN:	or	bl,bl			; The next event at once only for a
	jz	PumpX			; program that will not come round
	mov	cs:Hold,0
	jmp	Pump0
PumpE:	or	bl,bl
	jnz	Pump4
	cmp	cs:Idles,Rounds
	jb	PumpX
Pump4:	mov	al,'S'
	call	PutScr
	cmp	cs:Prog,0
	je	Pump5
	or	bl,bl
	jnz	Pump5
	cmp	cs:HStLen,0
	je	Pump6
	call	PutStk
Pump6:	mov	bx,cs:HClock		; With the clock on, look for a while
	cmp	bx,0FFFFh		; at what blinks before going on
	je	Pump7
	pop	ds			; The program's DS: see Int28
	push	ds
	cmp	byte ptr [bx],0
	je	Pump7
	mov	al,0
	call	Screen
	call	Ticks
	mov	cs:WTick,ax
	mov	cs:Watch,1
	jmp	PumpX
Pump7:	mov	bl,0
Pump5:	inc	cs:SPos
	jmp	Pump0
PumpX:	pop	ds
	pop	cx
	pop	bx
	ret
Pump	endp

; The program has come round while what blinks is looked for: write the
; screen out if it has changed, and end the step when that has gone on
; for long enough. Returns ZR while it has not.
Look	proc	near
	mov	al,1
	call	Screen
	je	Look1
	mov	al,0
	call	Screen
	mov	al,'B'
	call	PutScr
Look1:	call	Ticks
	sub	ax,cs:WTick
	cmp	ax,Blink
	jae	Look2
	xor	ax,ax			; ZR
	ret
Look2:	mov	cs:Watch,0
	inc	cs:SPos			; The step's event is done with
	or	sp,sp			; NZ
	ret
Look	endp

;--------------------- Interrupts ----------------------
; INT 28h: whoever calls is idle. DOS does while it waits for a key; the
; program does in LeaveTm, and is known by the code that follows the call.
Int28:	push	ax
	cmp	cs:Done,0
	jne	I28X
	mov	cs:Prog,0
	cmp	cs:Ready,0
	je	I28P
	push	bp
	mov	bp,sp
	pushf
	push	cx
	push	si
	push	di
	push	ds
	push	es
	les	di,[bp+4]		; Where the call returns to
	push	cs
	pop	ds
	mov	si,offset HSig
	mov	cx,6
	cld
	repe	cmpsb
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
	jne	I28A
	mov	cs:Prog,1
I28A:	popf
	pop	bp
	cmp	cs:Prog,0
	je	I28I
	push	bx			; The program, with DS on its main
	mov	bx,cs:HAuto		; data: has it something to do by
	cmp	bx,0FFFFh		; itself after a while?
	je	I28B
	cmp	word ptr [bx],0FFFFh
	je	I28B
	pop	bx
	mov	cs:Idles,0
	jmp	I28X
I28B:	pop	bx
I28I:	cmp	cs:Idles,255
	je	I28W
	inc	cs:Idles
I28W:	cmp	cs:Watch,0
	je	I28P
	call	Look
	jz	I28X
I28P:	xor	al,al
	call	Pump
I28X:	pop	ax
	jmp	cs:[Old28]

; INT 16h: with AH 00h or 10h the caller waits for a key inside the BIOS,
; and comes round no more until there is one.
Int16:	cmp	cs:Done,0
	jne	I16X
	test	ah,0EFh
	jnz	I16X
	push	ax
	mov	cs:Prog,0
	mov	cs:Watch,0
	mov	al,1
	call	Pump
	pop	ax
I16X:	jmp	cs:[Old16]

SecBuf	db	512 dup (?)
Last	label	byte			; Cells bytes, and the driver ends

;----------------------- Loading -----------------------
Init:	mov	PktSeg,cs
	mov	ax,3516h
	int	21h
	mov	word ptr Old16,bx
	mov	word ptr Old16+2,es
	mov	ax,3528h
	int	21h
	mov	word ptr Old28,bx
	mov	word ptr Old28+2,es
	mov	dx,offset Int16
	mov	ax,2516h
	int	21h
	mov	dx,offset Int28
	mov	ax,2528h
	int	21h
	mov	dx,offset Last+Cells+15
	shr	dx,4
	add	dx,Pad
	mov	ax,3100h
	int	21h

; The file is as long whatever is changed in the driver: its length shows
; on the screen where a scenario lists the directory it is in.
	db	(Start + FileLen - $) dup (0)
	end	Start
