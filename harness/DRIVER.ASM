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
; port the same way (command D3h), for the BIOS and the mouse driver that
; DOS has loaded. Every call of INT 16h, of INT 28h and of INT 33h goes on
; to whoever had the interrupt before.
;
; The program is idle when it calls INT 28h, as it does each time it has
; gone round waiting for a key (LeaveTm), or when it asks INT 16h to wait
; for a key. When it is idle and the BIOS's keyboard buffer is empty, the next
; event of the script is due. A step has ended when the program has gone
; round twice like that with nothing left to take and nothing that it will
; do by itself after a while (AutoTim): the screen is then written out.
;
; AutoTim is a time of day, or FFFFh in its low word for none: the program
; sets it when the cursor of a tree has moved, to 0.6 seconds from then as
; it is distributed, and goes to the directory when that time has come. It
; says nothing until the main loop has set it once. The main data is new
; memory when the program starts and after every command of DOS, and a box
; that comes up while the panels are drawn for the first time, for a
; diskette that is to be put in, finds in AutoTim what that memory held:
; as a rule a time that will not come. So the driver believes a time for
; 2 seconds from when it first saw it there, and after that takes it for
; none, also when it finds the same there again later: new memory holds
; after the next command what it held after the last, and the program
; does not come to set the same hundredth of a second twice.
;
; The script is on a hard disk of its own, which has no file system and is
; known by the two bytes it begins with, from its first sector on. It may
; be as long as its disk, a megabyte, and the driver would read one of 32
; megabytes: its place in the script has 32 bits, and a sector's number
; 16.
;
;	A5 5A		says that it is a script
;	word		where AutoTim is in the program's main data, or FFFFh
;	word		where Clock is there, or FFFFh for not looking at what
;			blinks
;	word, word	where the program's stack begins in its segment, and
;			how long it is, or 0 for not looking at it
;	8 bytes		the program's name as DOS has it in the header of
;			its memory, ended by a 0 if it is shorter: by it
;			the program is told from DOS, which calls INT 28h
;			too, and from what the program runs
;
; and from byte 18 the events:
;
;	01 n bytes	n bytes from the keyboard
;	02 word bytes	as many bytes from the mouse as the word says: its
;			packets of three, the buttons and how far it moved
;	03		the step ends
;	04 n		nothing for n more rounds of the program
;	05 n		nothing until the program has asked the mouse
;			driver n more times where the mouse is and which
;			buttons are down (INT 33h, function 3): a program
;			that waits for a button to come up asks that, and
;			is not idle meanwhile. Behind an event 02 it counts
;			from that event on
;	anything else	the script ends
;
; What the driver writes goes to a port that QEMU keeps in a file:
;
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
; While no disk holds a script the driver has nothing to go by, and stops
; the machine, through the port that QEMU has for a program to do that
; (pvpanic). There the machine's state is saved: at an instruction of the
; machine's, not at a moment of the host's, so every machine that is
; booted has executed the same by then. One that goes on from the state
; has a script.
;
; A machine that goes on first waits for two ticks of the timer, halted.
; QEMU's clock for the machine is the instructions executed, and while the
; machine is halted it is put straight on to whatever is due next. As the
; machine stops itself QEMU sometimes takes it for halted, and puts the
; clock on to the next tick, and sometimes not: of twelve boots, six had
; the timer 4.9 ms short of its tick in the saved state and six had it at
; the tick. Halted until that tick and then until the one behind it, both
; are at the same instruction at the same time.
;
; Assemble with JWasm: jwasm -bin -Fo DRIVER.COM DRIVER.ASM
	.model	tiny
	.386				; The machine is a 486

OutPort	EQU	0E9h			; QEMU's debug console
StopPort EQU	505h			; QEMU's port that stops the machine
Cells	EQU	4000			; Bytes of an 80 x 25 text screen
Events	EQU	18			; Where the events begin in a script
Rounds	EQU	2			; Idle rounds that end a step
Blink	EQU	20			; Ticks to look for what blinks: 1.1 s
Soon	EQU	37			; Ticks for which AutoTim is believed
FileLen	EQU	2048			; Bytes of the driver's file
SecBuf	EQU	100h + FileLen		; Behind it in memory: a sector of
Last	EQU	SecBuf + 512		; the script, and Cells bytes of screen

	.code
	org	100h
Start:	jmp	near ptr Init

; How many paragraphs of memory the driver keeps beyond its own, so that
; what DOS loads after it lies where the tests want it (see PLACE in
; harness/setups.py). The host sets this in the file, at its fourth byte.
Pad	dw	0

Old16	dd	0
Old28	dd	0
Old33	dd	0
SPos	dd	0			; Where in the script the next byte is
SSec	dw	0FFFFh			; Which sector of it SecBuf holds
SDisk	db	80h			; Which disk it is on, as the BIOS
					; numbers the hard disks
Ready	db	0			; The script's header has been read
Done	db	0			; The script has ended
Idles	db	0			; Rounds the program has idled since
					; anything was fed or seen to change
Hold	db	0			; Rounds still to let pass (event 04)
Asks	db	0			; Times the program still has to ask
					; the mouse driver (event 05)
Prog	db	0			; INT 28h came from the program
Watch	db	0			; Looking for what blinks
WTick	dw	0			; ... since this tick of the timer

; The header of the script, from its third byte.
HAuto	dw	0FFFFh
HClock	dw	0FFFFh
HStack	dw	0
HStLen	dw	0
HName	db	8 dup (0)

; The time that AutoTim was last seen to hold, and the tick at which it
; was first seen to hold that.
PTime	dw	0FFFFh, 0FFFFh
PTick	dw	0

; The packet for INT 13h, AH=42h: one sector to SecBuf.
Packet	db	10h, 0
	dw	1
	dw	SecBuf
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
	mov	dl,SDisk
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
	mov	si,word ptr cs:SPos
	and	si,1FFh
	mov	ax,word ptr cs:SPos+1	; The sector: bits 9 to 24
	shr	ax,1
	cmp	ax,cs:SSec
	je	Peek1
	call	RdSec
Peek1:	mov	al,cs:[si+SecBuf]
	pop	si
	ret
Peek	endp

; AL = the byte of the script at SPos, and on to the next.
Next	proc	near
	call	Peek
	inc	cs:SPos
	ret
Next	endp

; Look for a script on the hard disks. Leaves Ready set if there is one.
Load	proc	near
	push	ax
	push	ds
	mov	ax,40h
	mov	ds,ax
	mov	ah,ds:[75h]		; How many hard disks the BIOS has
	pop	ds
	mov	al,80h
	add	ah,al
Load2:	cmp	al,ah
	jae	Load1
	mov	cs:SDisk,al
	push	ax
	xor	ax,ax
	call	RdSec
	pop	ax
	jc	Load3
	cmp	word ptr cs:[SecBuf],5AA5h
	je	Load4
Load3:	inc	al
	jmp	Load2
Load4:	pusha
	push	ds
	push	es
	push	cs
	pop	ds
	push	cs
	pop	es
	mov	si,SecBuf+2
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
	mov	di,Last
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
	push	dx			; No script: the machine stops here,
	mov	dx,StopPort		; and one that goes on from here has
	mov	al,1			; a script
	cli
	out	dx,al
	sti				; Two ticks of the timer, halted, with
	hlt				; no interrupt taken before the first
	hlt				; halt: see the top
	pop	dx
	call	Load
	cmp	cs:Ready,0
	jne	Pump0
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
	cmp	cs:Asks,0
	jne	PumpX
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
	cmp	al,5
	je	PumpA
	mov	al,'E'			; The script has ended
	out	OutPort,al
	mov	cs:Done,1
	mov	al,0FEh			; Reset: QEMU ends at that
	out	64h,al
Pump3:	hlt
	jmp	Pump3
PumpK:	inc	cs:SPos
	call	Next
	mov	cl,al
	mov	ch,0
	mov	ah,0D2h
	jmp	PumpF
PumpM:	inc	cs:SPos
	call	Next
	mov	cl,al
	call	Next
	mov	ch,al
	mov	ah,0D3h
	call	Feed
	mov	cs:Idles,0
	call	Peek			; An event 05 behind this one counts
	cmp	al,5			; from now: the program may not be
	je	PumpA			; idle again before it is over
	jmp	PumpN
PumpF:	call	Feed
	mov	cs:Idles,0
	jmp	PumpN
PumpA:	inc	cs:SPos
	call	Next
	mov	cs:Asks,al
	mov	cs:Idles,0
	jmp	PumpN
PumpH:	inc	cs:SPos
	call	Next
	mov	cs:Hold,al
	mov	cs:Idles,0
PumpN:	or	bl,bl			; The next event at once only for a
	jz	PumpX			; program that will not come round
	mov	cs:Hold,0
	mov	cs:Asks,0
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
; INT 28h: whoever calls is idle. DOS does while it waits for a key, for
; the program that asked it; the program does by itself, and is known by
; that: the call comes from the memory of the program that DOS has as the
; current one, and that has the script's name.
Int28:	push	ax
	cmp	cs:Done,0
	jne	I28X
	mov	cs:Prog,0
	cmp	cs:Ready,0
	je	I28P
	push	bp
	mov	bp,sp
	pushf
	push	bx
	push	cx
	push	si
	push	di
	push	ds
	push	es
	mov	ah,62h			; The current program's PSP, which
	int	21h			; DOS may be asked for at any time
	cmp	[bp+6],bx		; Does the call come from above it?
	jb	I28N
	dec	bx			; The header of its memory, with its
	mov	es,bx			; name from the ninth byte on
	mov	di,8
	push	cs
	pop	ds
	mov	si,offset HName
	mov	cx,8
	cld
I28C:	lodsb
	scasb
	jne	I28N
	or	al,al
	loopnz	I28C
	mov	cs:Prog,1
I28N:	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
	pop	bx
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
	mov	ax,[bx]			; A time: one that was seen there?
	cmp	ax,cs:PTime
	jne	I28G
	mov	ax,[bx+2]
	cmp	ax,cs:PTime+2
	jne	I28G
	call	Ticks			; For how long has it been?
	sub	ax,cs:PTick
	cmp	ax,Soon
	ja	I28B			; Too long to be a time that comes
	jmp	I28H
I28G:	mov	ax,[bx]			; Another: it is believed from now
	mov	cs:PTime,ax
	mov	ax,[bx+2]
	mov	cs:PTime+2,ax
	call	Ticks
	mov	cs:PTick,ax
I28H:	pop	bx
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

; INT 33h, function 3: the program asks where the mouse is. When it has
; asked as often as the script says, the next event is due at once: the
; program may be waiting for nothing else.
Int33:	cmp	ax,3
	jne	I33X
	cmp	cs:Asks,0
	je	I33X
	dec	cs:Asks
	jnz	I33X
	push	ax
	xor	al,al
	call	Pump
	pop	ax
I33X:	jmp	cs:[Old33]

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
	mov	ax,3533h		; A mouse driver, if one is loaded
	int	21h
	mov	ax,es
	or	ax,bx
	jz	Init1
	mov	word ptr Old33,bx
	mov	word ptr Old33+2,es
	mov	dx,offset Int33
	mov	ax,2533h
	int	21h
Init1:
	mov	dx,Last+Cells+15
	shr	dx,4
	add	dx,Pad
	mov	ax,3100h
	int	21h

; The file is as long whatever is changed in the driver: its length shows
; on the screen where a scenario lists the directory it is in. And so is
; what the driver keeps of the memory, which shows in the program's map
; of it and decides where the program is loaded: behind the file, a
; sector of the script and a screen.
	db	(Start + FileLen - $) dup (0)
	end	Start
