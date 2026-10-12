;--------------------- Subroutines ---------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;----------------------- Timer -------------------------
Timer	PROC	NEAR
	PUSH	DS
	XOR	CX,CX
	MOV	DS,CX
	CLI
	MOV     DX,DS:[46Ch].Lo
	MOV     CX,DS:[46Ch].Hi
	STI
	POP	DS
	RET
Timer	ENDP

;----------------- Clear keyboard buffer ---------------
ClrKbd	PROC	NEAR
	PUSH	AX
	CMP	ES:ClrBuf,0
	JZ	ClrKb3
	JMP	ClrKb2
ClrKb1:	MOV	AH,0
	INT	16h
ClrKb2:	MOV	AH,1
	INT	16h
	JNZ	ClrKb1
ClrKb3:	POP	AX
	RET
ClrKbd	ENDP
	FromC	ClrKbd

;---------------------- Break test ---------------------
;	Return :	CY=1 - break
TstBrk	PROC	NEAR
	PUSH	BX
	PUSH	AX
	MOV	BL,1
TstBr1:	MOV	AH,1
	INT	16h
	JZ	TstBr3
	OR	AX,AX
	JE	TstBr2
	CMP	AX,2E03h
	JE	TstBr2
	CMP	AX,011Bh
	JE	TstBr2
	CMP	AX,4400h
	JNE	TstBr3
TstBr2:	MOV	AH,0
	INT	16h
	MOV	BL,0
	JMP	TstBr1
TstBr3:	CMP	BL,1
	POP	AX
	POP	BX
	RET
TstBrk	ENDP
	FromC	TstBrk, <SBB AX,AX>	; CY as a number

;----------------------- Go to CASE --------------------
;			CS:SI - address of case table
;			   AX - key code
Case	PROC	NEAR
	PUSH	SI
	PUSH	CX
	OR	AH,AH
	JE	Case3
Case1:	MOV	CX,CS:[SI]
	ADD	SI,4
	JCXZ	Case3
	CMP	AL,CL
	JNE	Case1
	OR	CH,CH
	JE	Case2
	CMP	AH,CH
	JNE	Case1
Case2:	POP	CX
	ADD	SP,4
	JMP	WORD PTR CS:[SI-2]
Case3:	POP	CX
	POP	SI
	RET
Case	ENDP

;------------------ Check video mode -------------------
;			   ZR - ok
ChkMode	PROC	NEAR
	MOV	AH,0Fh
	INT	10h
	CMP	AH,80
	JNE	ChkMd1
	AND	AL,7Fh
	CMP	AL,2
	JE	ChkMd1
	CMP	AL,3
	JE	ChkMd1
	CMP	AL,7
ChkMd1:	RET
ChkMode	ENDP

;----------------------- Set cursor --------------------
;			AL,AH - x, y
Cursor	PROC	NEAR
	PUSH	DX
	PUSH	BX
	PUSH	AX
	MOV	DX,AX
	MOV	AH,0Fh
	INT	10h
	MOV	AH,2
	INT	10h
	POP	AX
	POP	BX
	POP	DX
	RET
Cursor	ENDP
	FromC	Cursor

;------------------ Set cursor on prompt ---------------
Cursor1	PROC	NEAR
	PUSH	AX
	MOV	AL,ES:DosCol
	SUB	AL,ES:OutCol
	ADD	AL,ES:BegCol
	MOV	AH,ES:DosLin
	CALL	Cursor
	POP	AX
	RET
Cursor1	ENDP
	FromC	Cursor1

;-------------- Get cursor size & set it ---------------
;	Return :	CX - cursor size
ChgCur	PROC	NEAR
	PUSH	BP
	PUSH	DX
	PUSH	BX
	MOV	BH,0
	MOV	AH,3
	INT	10h
	PUSH	CX
	MOV	CX,0607h
	CMP	ES:ScrMod,7
	JNE	ChCur1
	MOV	CX,0B0Ch
ChCur1:	CMP	ES:ColTyp,2
	JNE	ChCur2
	MOV	CH,0
ChCur2:	MOV	AH,1
	INT	10h
	POP	CX
	POP	BX
	POP	DX
	POP	BP
	RET
ChgCur	ENDP
	FromC	ChgCur, <MOV AX,CX>	; The shape it had, in AX

;------------------- Make hidden cursor ----------------
HidCur	PROC	NEAR
	PUSH	AX
	MOV	AX,7F00h
	CALL	Cursor
	POP	AX
	RET
HidCur	ENDP
	FromC	HidCur

;----------------------- Beep sound --------------------
Beep	PROC	NEAR
	CMP	ES:ErrSnd,0
	JE	Beep1
	PUSH	BP
	PUSH	AX
	MOV	AX,0E07h
	INT	10h
	POP	AX
	POP	BP
Beep1:	RET
Beep	ENDP
	FromC	Beep

;---------------------- Init mouse ---------------------
IniMous	PROC	NEAR
	MOV	AX,21h
	INT	33h
	CMP	AX,WORD PTR 21h
	JNE	IniMo1
	XOR	AX,AX
	INT	33h
IniMo1:	CMP	AX,WORD PTR 0FFFFh
	CMC
	MOV	AL,0
	ADC	AL,AL
	MOV	ES:Mouse,AL
	OR	AL,AL
	JNE	IniMo2
	MOV	AX,0FFFFh
	MOV	ES:MouseX,AX
	MOV	ES:MouseY,AX
	RET
IniMo2:	MOV	AL,0
	CALL	TypMous
	MOV	DL,ES:Lines
	CMP	DL,25
	JE	IniMo3
	MOV	DH,0
	MOV	CL,3
	SHL	DX,CL
	DEC	DX
	XOR	CX,CX
	MOV	AX,8
	INT	33h
IniMo3:	MOV	CX,ES:MouseX
	MOV	DX,ES:MouseY
	MOV	AX,CX
	AND	AX,DX
	INC	AX
	JE	IniMo4
	MOV	AX,4
	INT	33h
IniMo4:	MOV	AL,1
	CALL	CurMous
	RET
IniMous	ENDP

;--------------- Set type of mouse sursor --------------
;			   AL - cursor
TypMous	PROC	NEAR
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	CX,0FFFFh
	MOV	DX,7700h
	CMP	AL,0
	JE	TypMo1
	MOV	CL,0
	MOV	DL,AL
TypMo1:	XOR	BX,BX
	MOV	AX,0Ah
	INT	33h
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	RET
TypMous	ENDP
	FromC	TypMous

;------------------ Save mouse cursor ------------------
SavMous	PROC	NEAR
	CMP	ES:Mouse,0
	JE	SavMo1
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	AX,3
	INT	33h
	MOV	ES:MouseX,CX
	MOV	ES:MouseY,DX
	POP	AX
	POP	BX
	POP	CX
	POP	DX
SavMo1:	RET
SavMous	ENDP

;------------------- Set mouse cursor ------------------
;			   AL - set/reset
CurMous	PROC	NEAR
	CMP	ES:Mouse,0
	JE	CurMo1
	PUSH	AX
	CMP	AL,1
	MOV	AX,1
	ADC	AL,0
	INT	33h
	POP	AX
CurMo1:	RET
CurMous	ENDP

;------------------- Get mouse cursor ------------------
;	Return :	CH,CL - mouse cursor
;			   AX - mouse buttons
GetMous	PROC	NEAR
	PUSH	DX
	PUSH	BX
	XOR	AX,AX
	CMP	ES:Mouse,0
	STC
	JZ	GetMo1
	MOV	AX,3
	INT	33h
	MOV	AX,CX
	MOV	CL,3
	SHR	AX,CL
	SHR	DX,CL
	MOV	CL,AL
	MOV	CH,DL
	MOV	AX,BX
	CMP	ES:LftMous,0
	JE	GetMo1
	MOV	BH,BL
	SHR	BL,1
	SHL	BH,1
	AND	BX,201h
	AND	AL,4
	OR	AL,BL
	OR	AL,BH
GetMo1:	AND	AX,WORD PTR 7
	POP	BX
	POP	DX
	RET
GetMous	ENDP
	FromC	GetMous, <MOV DX,CX>	; Where the mouse is, with AX

;----------------- Mouse double click ------------------
;			BH,BL - base of selected point
;			DH,DL - size of selected point
;			   SI - cursor
;	Return :	   CY - continue
;			   NZ - action
DoublMo	PROC	NEAR
	PUSH	DX
	CMP	ES:MousEnt,0
	JNE	DblMo3
	MOV	ES:MousEnt,1
	MOV	AH,2Ch
	INT	21h
	CMP	ES:MousTim.Hi,0FFFFh
	JE	DblMo1
	CMP	SI,ES:MousPos
	JE	DblMo4
DblMo1:	ADD	DL,MoDelay
	CMP	DL,100
	JB	DblMo2
	INC	DH
	CMP	DH,60
	JB	DblMo2
	INC	CL
	CMP	CL,60
	JB	DblMo2
	INC	CH
DblMo2:	MOV	ES:MousTim.Lo,DX
	MOV	ES:MousTim.Hi,CX
	MOV	ES:MousPos,SI
DblMo3:	POP	DX
	STC
	RET
DblMo4:	CMP	CX,ES:MousTim.Hi
	JA	DblMo1
	JB	DblMo5
	CMP	DX,ES:MousTim.Lo
	JA	DblMo1
DblMo5:	POP	DX
	MOV	CH,0
DblMo6:	PUSH	CX
	CALL	GetMous
	CALL	InWind
	POP	CX
	PUSH	AX
	MOV	AL,0
	JC	DblMo7
	MOV	AL,'û'
DblMo7:	CMP	AL,CH
	JE	DblMo8
	MOV	CH,AL
	CALL	TypMous
DblMo8:	POP	AX
	TEST	AL,7
	JNZ	DblMo6
	OR	CH,CH
	RET
DoublMo	ENDP

; C code asks about a double click: DoublMo, and in AX 2 for CY, to go on,
; 1 for NZ, to act, and 0 for neither.
	PUBLIC	DoublMo_a
DoublMo_a PROC	FAR
	PUSH	DS
	POP	ES
	CALL	DoublMo
	MOV	AX,2
	JC	DblMoA
	MOV	AX,1
	JNE	DblMoA
	DEC	AX
DblMoA:	RET
DoublMo_a ENDP

;--------------- Check cursor in window ----------------
;			CH,CL - cursor
;			BH,BL - begin of window
;			DH,DL - size of window
;	Return :	CY=0  - cursor inside the window
InWind	PROC	NEAR
	PUSH	DX
	PUSH	CX
	SUB	CH,BH
	JB	InWin1
	SUB	CL,BL
	JB	InWin1
	DEC	DH
	CMP	DH,CH
	JB	InWin1
	DEC	DL
	CMP	DL,CL
InWin1:	POP	CX
	POP	DX
	RET
InWind	ENDP
	FromC	InWind, <SBB AX,AX>	; CY as a number

;------------------ INT 10h with mouse -----------------
Int10m	PROC	NEAR
	PUSH	BP
	PUSH	AX
	MOV	AL,0
	CALL	CurMous
	POP	AX
	INT	10h
	PUSH	AX
	MOV	AL,1
	CALL	CurMous
	POP	AX
	POP	BP
	RET
Int10m	ENDP
	FromC	Int10m

;------------- Convert coordinars to address -----------
;			BH,BL - coordinats
;	Return :	   DI - address
BasAdr	PROC	NEAR
	CALL	FAR PTR BasAdr_c
	RET
BasAdr	ENDP

;-------------------- Screen Offset --------------------
ScrOffs	PROC	NEAR
	PUSH	DS
	XOR	DI,DI
	MOV	AH,0Fh
	INT	10h
	OR	BH,BH
	JE	ScrOf1
	MOV	DS,DI
	MOV	DI,DS:[44Eh]
ScrOf1:	POP	DS
	MOV	DS:Screen.Offs,DI
	RET
ScrOffs	ENDP

;--------------- Shift windows in EGA lines ------------
SftEGA	PROC	NEAR
	PUSH	AX
	MOV	AL,ES:Lines
	SUB	AL,25
	JBE	SfEGA1
	SHR	AL,1
	ADD	BH,AL
SfEGA1:	POP	AX
	RET
SftEGA	ENDP
	FromC	SftEGA

;---------------------- Save window --------------------
;			BH,BL - begin
;			DH,DL - size
;			   SI - address of buffer
SavWin	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	CLD
	CALL	BasAdr
	XCHG	SI,DI
	MOV	CL,DH
	MOV	CH,0
	MOV	DH,0
	PUSH	ES
	POP	DS
SavWn1:	PUSH	CX
	PUSH	SI
	MOV	CX,DX
	REP	MOVSW
	POP	SI
	POP	CX
	ADD	SI,2*80
	LOOP	SavWn1
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
SavWin	ENDP
	FromC	SavWin

;--------------------- Restore window ------------------
;			BH,BL - begin
;			DH,DL - size
;			   SI - address of buffer
ResWin	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	CLD
	CALL	BasAdr
	MOV	CL,DH
	MOV	CH,0
	MOV	DH,0
	PUSH	ES
	POP	DS
ResWn1:	PUSH	CX
	PUSH	DI
	MOV	CX,DX
	REP	MOVSW
	POP	DI
	POP	CX
	ADD	DI,2*80
	LOOP	ResWn1
	MOV	DS:Quote,0
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
ResWin	ENDP
	FromC	ResWin

;----------------- Restore user screen -----------------
;			BH,BL - begin
;			DH,DL - size
ResUser	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	CLD
	PUSH	ES
	POP	DS
	CALL	BasAdr
	LEA	SI,[DI-ScrBuf+UserScr]
	MOV	CL,DH
	MOV	CH,0
	MOV	DH,0
ResUs1:	PUSH	SI
	PUSH	DI
	PUSH	CX
	MOV	CX,DX
	REP	MOVSW
	POP	CX
	POP	DI
	POP	SI
	ADD	SI,2*80
	ADD	DI,2*80
	LOOP	ResUs1
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
ResUser	ENDP

;--------------------- Output window -------------------
;			BH,BL - begin
;			DH,DL - size
Window	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	PUSH	ES
	MOV	AL,0
	CALL	CurMous
	PUSH	ES
	POP	DS
	MOV	ES,DS:Screen.Segm
	CALL	BasAdr
	MOV	SI,DI
	SUB	DI,OFFSET ScrBuf
	ADD	DI,DS:Screen.Offs
	MOV	AL,DS:Snow
	CMP	DH,1
	JE	Wind1
	MOV	BH,0
	ADD	BX,BX
	SUB	SI,BX
	SUB	DI,BX
	PUSH	DI
	PUSH	DX
	CALL	MovScr
	POP	AX
	MOV	AL,80
	MUL	AH
	MOV	CX,AX
	POP	DI
	JMP	Wind6
Wind1:	MOV	CL,DL
	MOV	CH,0
	PUSH	DI
	PUSH	CX
	MOV	AH,AL
	MOV	DX,03DAh
Wind2:	OR	AH,AH
	JNZ	Wind5
Wind3:	IN	AL,DX
	TEST	AL,1
	JNZ	Wind3
	CLI
Wind4:	IN	AL,DX
	TEST	AL,1
	JZ	Wind4
Wind5:	MOVSW
	STI
	LOOP	Wind2
	POP	CX
	POP	DI
Wind6:	POP	ES
	CMP	ES:TopView,0
	JZ	Wind7
	MOV	AH,0FFh
	INT	10h
Wind7:	MOV	AL,1
	CALL	CurMous
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
Window	ENDP
	FromC	Window

;------------------------ Save screen ------------------
SavScr	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	DS,ES:Screen.Segm
	MOV	SI,ES:Screen.Offs
	MOV	DI,OFFSET ScrBuf
	MOV	DH,ES:Lines
	MOV	AL,ES:Snow
	CALL	MovScr
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
SavScr	ENDP

;-------------------- Move screen bytes ----------------
;			   DH - number of lines
;			   AL - snow
;			DS:SI => ES:DI
MovScr	PROC	NEAR
	CLD
	MOV	BL,AL
	MOV	DL,7			; Const1 ???
MovSc1:	CMP	DH,DL
	JAE	MovSc2
	MOV	DL,DH
MovSc2:	MOV	AX,80
	MUL	DL
	PUSH	AX
	OR	BL,BL
	JNZ	MovSc6
	PUSH	DX
	MOV	DX,03DAh
MovSc3:	IN	AL,DX
	TEST	AL,1
	JNZ	MovSc3
MovSc4:	STI
	MOV	CX,10			; Const2 ???
	CLI
MovSc5:	IN	AL,DX
	TEST	AL,1
	JZ	MovSc4
	LOOP	MovSc5
	POP	DX
MovSc6:	POP	CX
	REP	MOVSW
	STI
	SUB	DH,DL
	JNE	MovSc1
	RET
MovScr	ENDP

;---------------------- Zooming box --------------------
;			BH,BL - begin
;			DH,DL - size
;			   AH - color
Diskret	EQU	5
MinBox	PROC	NEAR
	PUSH	DI
	PUSH	CX
	PUSH	AX
	MOV	AL,0
	MOV	ES:Quote,AL
	CMP	AL,ES:ZoomWin
	JZ	MinBx2
	PUSH	AX
	MOV	AH,ShadFl
	CALL	ColAddr
	POP	AX
	XCHG	AL,ES:[DI]
	PUSH	DI
	PUSH	AX
	XOR	DI,DI
	CALL	MinBx3
	MOV	CX,Diskret-1
MinBx1:	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	AL,DL
	MOV	AH,BL
	MOV	CH,11
	CALL	MinBx5
	MOV	DL,AL
	MOV	BL,AH
	MOV	AL,DH
	MOV	AH,BH
	MOV	CH,5
	CALL	MinBx5
	MOV	DH,AL
	MOV	BH,AH
	POP	AX
	CALL	ShadBox
	CALL	MinBx3
	CALL	Window
	POP	BX
	POP	CX
	POP	DX
	LOOP	MinBx1
	CALL	MinBx3
	POP	AX
	POP	DI
	MOV	ES:[DI],AL
MinBx2:	CALL	ShadBox
	POP	AX
	POP	CX
	POP	DI
	RET
MinBox	ENDP
	FromC	MinBox

MinBx3	PROC	NEAR
	PUSH	DX
	PUSH	CX
	PUSH	AX
MinBx4:	STI
	CALL	Timer
	CMP	DX,DI
	JE	MinBx4
	MOV	DI,DX
	POP	AX
	POP	CX
	POP	DX
	RET
MinBx3	ENDP

MinBx5	PROC	NEAR
	PUSH	AX
	SUB	AL,CH
	SHR	AL,1
	MUL	CL
	MOV	CH,Diskret-1
	DIV	CH
	MOV	CH,AL
	POP	AX
	ADD	AH,CH
	SUB	AL,CH
	SUB	AL,CH
	RET
MinBx5	ENDP

;-------------------- Box with shadow ------------------
;			BH,BL - begin
;			DH,DL - size
;			   AH - color
ShadBox	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	CALL	BasAdr
	MOV	CL,DH
	MOV	CH,0
	MOV	SI,CX
	SUB	DL,10
	MOV	DH,0
	MOV	CL,3
	MOV	BX,'  '
	MOV	AL,' '
	CALL	BoxLine
	MOV	BX,'É»'
	MOV	AL,'Í'
	CALL	ShadLin
	SUB	SI,5
	JBE	ShdBx2
	MOV	BX,'ºº'
	MOV	AL,' '
ShdBx1:	CALL	ShadLin
	DEC	SI
	JNE	ShdBx1
ShdBx2:	MOV	BX,'È¼'
	MOV	AL,'Í'
	CALL	ShadLin
	MOV	BX,'  '
	MOV	AL,' '
	CALL	ShadLin
	ADD	DI,2*2
	MOV	CX,DX
	ADD	CL,8
ShdBx3:	CALL	Shadow
	LOOP	ShdBx3
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	RET
ShadBox	ENDP

;-------------------------- Box ------------------------
;			ES:DI - screen buffer
;			   AH - color
;		    ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
;		    º                 º   ¿
;		    º                 º   Ã SI
;		    º                 º   Ù
;		    ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
;		 ÀÂÙ ÀÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÙ ÀÂÙ
;		  CX         DX         CX
BoxDubl	PROC	NEAR
	PUSH	SI
	PUSH	BX
	PUSH	AX
	MOV	AL,'Í'
	MOV	BX,'É»'
	CALL	BoxLine
	MOV	AL,' '
	MOV	BX,'ºº'
BoxDb1:	CALL	BoxLine
	DEC	SI
	JNE	BoxDb1
	MOV	AL,'Í'
	MOV	BX,'È¼'
	CALL	BoxLine
	POP	AX
	POP	BX
	POP	SI
	RET
BoxDubl	ENDP
	FromC	BoxDubl

;-------------------- One line of box ------------------
;			ES:DI - screen buffer
;			   AH - color
;		³   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   ³
;		 ÀÂÙ|ÀÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÙ|ÀÂÙ
;		 CX BH         DX(AL)      BL CX
ShadLin	PROC	NEAR
	PUSH	DI
	CALL	BoxLn0
	CALL	Shadow
	CALL	Shadow
	POP	DI
	ADD	DI,2*80
	RET
ShadLin	ENDP

BoxLine	PROC	NEAR
	PUSH	DI
	CALL	BoxLn0
	POP	DI
	ADD	DI,2*80
	RET
BoxLine	ENDP
	FromC	BoxLine

BoxLn0	PROC	NEAR
	PUSH	CX
	PUSH	AX
	PUSH	CX
	PUSH	AX
	JCXZ	BoxLn1
	MOV	AL,' '
	REP	STOSW
BoxLn1:	MOV	AL,BH
	STOSW
	POP	AX
	MOV	CX,DX
	REP	STOSW
	MOV	AL,BL
	STOSW
	POP	CX
	JCXZ	BoxLn2
	MOV	AL,' '
	REP	STOSW
BoxLn2:	POP	AX
	POP	CX
	RET
BoxLn0	ENDP

StdLine	PROC	NEAR
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	CALL	BasAdr
	SUB	DL,10
	MOV	DH,0
	MOV	CX,3
	MOV	BX,SI
	CALL	BoxLine
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	INC	BH
	RET
StdLine	ENDP
	FromC	StdLine

;------------------- Zoom box only once ----------------
OncZoom	PROC	NEAR
	PUSH	AX
	MOV	AL,ES:ZoomWin
	CMP	ES:ZoomOne,0
	JZ	OncZo1
	MOV	ES:ZoomWin,0
OncZo1:	CALL	DialBox
	CALL	Window
	MOV	ES:ZoomWin,AL
	MOV	ES:ZoomOne,1
	POP	AX
	RET
OncZoom	ENDP

; C code puts a box on the screen, zoomed once: OncZoom for the box at
; CS:AX, which puts where the box is, BX, DX and SI, into the three words
; at SS:DI.
	PUBLIC	OncZoom_a
OncZoom_a PROC	FAR
	PUSH	BP
	PUSH	DS
	POP	ES
	MOV	BP,AX
	CALL	OncZoom
	MOV	BP,DI
	MOV	[BP],BX
	MOV	[BP+2],DX
	MOV	[BP+4],SI
	POP	BP
	RET
OncZoom_a ENDP

;---------------------- Put window ---------------------
;			BH,BL - begin-center
;			CS:DX - address of include line
;			CS:BP - pointer:
;				DB  NormCol,RevCol,BoldCol,UndrCol
;				DB  NumLns,NumVars
;				DW  SaveBuf
;				DW  Variable
;				DB  VarMode,VarLimit
;				. . .
;				DW  OFFSET Line1
;				. . .
;	Return:		BH,BL - begin
;			DH,DL - size
;			   SI - address of save buffer
; A box is C: DialBox_c of port/dialog.c, with DiBx30. The box goes there
; as its offset, and in CX the caller's DS, where the texts are that it
; puts into its lines. Where the box is comes back in three words on the
; stack: BX, DX and SI.
DialBox	PROC	NEAR
	PUSH	DI
	PUSH	CX
	PUSH	AX
	SUB	SP,6
	MOV	DI,SP
	MOV	CX,DS
	MOV	AX,BP
	CallC	DialBox_c
	POP	BX
	POP	DX
	POP	SI
	POP	AX
	POP	CX
	POP	DI
	RET
DialBox	ENDP

;--------------- Calculate string length ---------------
;			DS:SI - address of line
;			CS:DX - address of include line
;	Return :	   AX - length
LnStr	PROC	NEAR
	PUSH	CX
	PUSH	BX
	PUSH	ES
	CallLine LnStr_c, AX
	TakeLine BX, BX
	POP	ES
	POP	BX
	POP	CX
	RET
LnStr	ENDP

;-------------------- Output NG line -------------------
;			DS:SI - address of line
;			ES:DI - address of screen buffer
;			CS:DX - address of include line
;			CS:BP - address of color table
;			   AL - limit
NgLine	PROC	NEAR
	PUSH	CX
	PUSH	BX
	PUSH	ES
	CallLine NgLine_c, AX
	TakeLine AX, BX
	POP	ES
	POP	BX
	POP	CX
	RET
NgLine	ENDP

;--------------------- Center line ---------------------
;			BH,BL - center
;			DS:SI - address of line
;			CS:DX - address of include line
;			CS:BP - address of color table
CntLine	PROC	NEAR
	PUSH	CX
	PUSH	AX
	PUSH	ES
	CallLine CntLine_c, BX
	TakeLine BX, AX
	POP	ES
	POP	AX
	POP	CX
	RET
CntLine	ENDP

;------------------------ Dialog -----------------------
;			CS:DX - address of include line
;			CS:BP - pointer:
;				DW  BoxPoint, BegCenter, SecondBox
;				DB  KeyBar, Menu, HlpPage, Status
;					DB  Type
;					DB  Up, Down, Left, Right
;					DW  Begin, Point, Control
;					...
;				DB  0
;				Status:	b0 - Beep
;					b1 - ClrKbd
;					b2 - SftEGA
;					b3 - F10-Tree
;					b4 - disable DialBox
; A dialog is C: Dialog_c of port/dialog.c, with the routines of its
; fields, InpStr, Select and FlipFl. The dialog goes there as its offset,
; and in CX the caller's DS, where the fields have what they set. The
; answer comes back in AX, and callers go by the zero flag for AL.
Dialog	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	MOV	CX,DS
	MOV	AX,BP
	CallC	Dialog_c
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	CMP	AL,0
	RET
Dialog	ENDP

; Entries for the C code of the dialogs to routines of other files (see
; FromC in VC.ASM, and port/dialog.c for what they take and return).
	FromC	PutMnu
	FromC	PutKey

; EdLin for C code: where the cursor is comes back in AX, and DX is not 0
; if the routine set the carry flag, for a line or a cursor that changed.
	PUBLIC	EdLin_a
EdLin_a	PROC	FAR
	PUSH	DS
	POP	ES
	CALL	EdLin
	MOV	AX,DX
	SBB	DX,DX
	RET
EdLin_a	ENDP

; The code segment of the assembly, where the boxes and the dialogs are
; that C code reads.
	PUBLIC	AsmSeg_a
AsmSeg_a PROC	FAR
	MOV	AX,CS
	RET
AsmSeg_a ENDP

; The cursor in the shape CX, as the BIOS takes it.
	PUBLIC	CurShp_a
CurShp_a PROC	FAR
	PUSH	BP
	MOV	AH,1
	INT	10h
	POP	BP
	RET
CurShp_a ENDP

; The routine that a dialog has for Enter and F10: the near routine at
; SI, with AX, BX and DI as Dialog has them, in BP the dialog, which
; comes in DX, and in DS what comes in CX, the DS of whoever asked for
; the dialog. AX comes back as the routine leaves it, and DX is not 0 if
; it set the carry flag.
	PUBLIC	DialSub_a
DialSub_a PROC	FAR
	PUSH	BP
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	BP,DX
	MOV	DS,CX
	CALL	SI
	SBB	DX,DX
	POP	DS
	POP	BP
	RET
DialSub_a ENDP

;--------------------- Input string --------------------
;			BH,BL - begin
;			ES:SI - string
;			   CX - length
;			   AH - number of color
; Showing a line that is typed into is C: InSt10_c of port/dialog.c. It
; leaves AX changed, as the routine did.
InSt10	PROC	NEAR
	CallC	InSt10_c
	RET
InSt10	ENDP

;------------------------ Menu -------------------------
;			CS:SI - control line
;			BH,BL - begin
;			DH,DL - size
;			   BP - color table
;			   AL - key bar
;			   CL - case
;	Return :	   CL - case
;			   AX - key code
; A line of buttons is C: MinMnu_c of port/dialog.c, with MnMn10 to
; MnMn60. The colours go there as their offset, in DI. CX comes back in
; DX: the case, and above it the key bar.
MinMnu	PROC	NEAR
	PUSH	DI
	PUSH	DX
	MOV	DI,BP
	CallC	MinMnu_c
	MOV	CX,DX
	POP	DX
	POP	DI
	RET
MinMnu	ENDP

;----------------------- Get color ---------------------
;			   AH - color number
;	Return :	   AH - color
Color	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BL,AL
	MOV	AL,AH
	CALL	FAR PTR Color_c
	MOV	AH,AL
	MOV	AL,BL
	POP	BX
	POP	DS
	RET
Color	ENDP

ColAddr	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	MOV	AL,AH
	CALL	FAR PTR ColAddr_c
	POP	AX
	POP	DS
	RET
ColAddr	ENDP

;------------------------- Shadow ----------------------
;			ES:DI - address of screen buffer
Shadow	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Shadow_c
	POP	DS
	RET
Shadow	ENDP

;-------------------- Moving strings -------------------
;			CS:SI(B) -> ES:DI(W)
MvsBwc	PROC	NEAR
	PUSH	DX
	MOV	DX,CS
	CALL	FAR PTR MvsBw_c
	POP	DX
	ADD	SI,CX
	XOR	CX,CX
	MOV	AL,CS:[SI-1]
	RET
MvsBwc	ENDP
;			ES:SI(B) -> ES:DI(W)
MvsBwe	PROC	NEAR
	PUSH	DX
	MOV	DX,ES
	CALL	FAR PTR MvsBw_c
	POP	DX
	ADD	SI,CX
	XOR	CX,CX
	MOV	AL,ES:[SI-1]
	RET
MvsBwe	ENDP
;			(Color) -> ES:DI(W)
MvsColr	PROC	NEAR
	CALL	FAR PTR MvsColr_c
	XOR	CX,CX
	RET
MvsColr	ENDP
;			CS:SI(B) -> ES:DI(W)
MovDz	PROC	NEAR
	CALL	BasAdr
MovDz0:	PUSH	DX
	PUSH	DI
	MOV	DX,DS
	CALL	FAR PTR MovDz0_c
	POP	DX			; Where the text begins on the screen
	SUB	DX,DI
	NEG	DX
	SHR	DX,1			; Its length
	ADD	SI,DX
	INC	SI
	POP	DX
	MOV	AL,0
	RET
MovDz	ENDP
;			Add file name
AddFile	PROC	NEAR
	CALL	FAR PTR AddFile_c
	MOV	AL,'\'
	RET
AddFile	ENDP
;			DS:SI(B) -> ES:DI(B)
MvzLin0	PROC	NEAR
	MOV	CX,LenPath
MvzLin0	ENDP

MvzLine	PROC	NEAR
	PUSH	DX
	PUSH	DI
	MOV	DX,DS
	CALL	FAR PTR MvzLine_c
	POP	DX			; Where the copy begins
	ADD	SI,CX
	ADD	CX,DX			; What the search for the end
	SUB	CX,DI			; left of the count
	DEC	CX
	POP	DX
	MOV	AL,0
	RET
MvzLine	ENDP
;			Clear rest of line DS:SI, legth CX
FilZero	PROC	NEAR
	PUSH	ES
	PUSH	AX
	PUSH	DS
	POP	ES
	CALL	FAR PTR FilZero_c
	POP	AX
	POP	ES
	MOV	AL,0
	XOR	CX,CX
	RET
FilZero	ENDP

;--------------------- Check CrLf ----------------------
;			   AX - word
;	Return :	   ZR - yes
IsCrLf0	PROC	NEAR
	DEC	SI
	DEC	SI
IsCrLf0	ENDP

IsCrLfS	PROC	NEAR
	LODSW
IsCrLfS	ENDP

IsCrLf	PROC	NEAR
	CMP	AX,0D0Ah
	JE	IsCrL1
	CMP	AX,0A0Dh
IsCrL1:	RET
IsCrLf	ENDP

;-------------------- String length --------------------
;			ES:DI - string
;	Return :	   CX - length
StrLen	PROC	NEAR
	CALL	FAR PTR StrLen_c
	MOV	AL,0
	RET
StrLen	ENDP

;--------------------- Upper letter --------------------
;			   AL - letter
;	Return :	   AL - letter
UpCase	PROC	NEAR
	CMP	AL,'a'
	JB	UpCas1
	CMP	AL,'z'
	JA	UpCas1
	SUB	AL,20h
UpCas1:	CALL	ES:Country.CaseMap
	RET
UpCase	ENDP
	FromC	UpCase

;--------------------- Lower letter --------------------
;			   AL - letter
;	Return :	   AL - letter
LowCase	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BH,AH
	CALL	FAR PTR LowCase_c
	MOV	AH,BH
	POP	BX
	POP	DS
	RET
LowCase	ENDP

;----------------------- Find text ---------------------
;			DS:SI - string
FndLet	PROC	NEAR
	PUSH	ES
	PUSH	DS
	POP	ES
	CALL	FAR PTR FndLet_c
	POP	ES
	MOV	AL,DS:[SI]
	RET
FndLet	ENDP

;------------------------ Cut path ---------------------
;			DS:SI - path
CutPath	PROC	NEAR
	PUSH	ES
	PUSH	DS
	POP	ES
	CALL	FAR PTR CutPath_c
	POP	ES
	RET
CutPath	ENDP

;--------------------- Normal string -------------------
;			ES:SI - address of string
NormStr	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR NormStr_c
	POP	AX
	RET
NormStr	ENDP

;---------------------- Higher letter ------------------
;			ES:SI - address of string
PathLin	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR PathLin_c
	POP	AX
	POP	DS
	RET
PathLin	ENDP

;----------------- Higher & lower letters --------------
;			ES:SI - address of string
LetLin	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR LetLin_c
	POP	AX
	POP	DS
	RET
LetLin	ENDP

;-------------------- Shirter string -------------------
;			ES:SI - address of string
;			   AX - limit
ShrtLin	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR ShrtLin_c
	POP	AX
	RET
ShrtLin	ENDP

;----------------------- Main file ---------------------
;			NcPath + CS:SI -> ES:DI
MainFil	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	ES
	POP	DS
	MOV	DX,CS			; The name is in this segment
	CALL	FAR PTR MainFil_c
	POP	DX
	POP	DS
	ADD	SI,13
	XOR	CX,CX
	MOV	AL,0
	RET
MainFil	ENDP

;-------------- Output number into buffer --------------
;			ES:DI - address of buffer
;			BX CX - number
;			   AL - mode: b0-coma, b1-spaces
Number	PROC	NEAR
	CallC	Number_c
	RET
Number	ENDP

;-------------------- Output hex byte ------------------
;			   AL - code
HexByt	PROC	NEAR
	CALL	FAR PTR HexByt_c
	RET
HexByt	ENDP

;-------------------- Text hex number ------------------
;			   AL - ASCII byte
;	Return :	   AL - hex number, CY=0
;				     error, CY=1
TstHex	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BH,AH
	CALL	FAR PTR TstHex_c
	NEG	AH			; CY if it is no digit
	MOV	AH,BH
	POP	BX
	POP	DS
	RET
TstHex	ENDP

;---------------------- Number input -------------------
;			ES:SI - string
;	Return :	   AX - number
;			 CY=1 - error
TxtNum	PROC	NEAR
	PUSH	BX
	PUSH	AX			; Room for the number
	MOV	BX,SP
	PUSH	SI
	CALL	FAR PTR TxtNum_c
	POP	AX			; Where the text begins
	CMP	AX,SI
	CMC				; CY if no digit is there
	POP	AX
	POP	BX
	RET
TxtNum	ENDP

;--------------- Convert name & extension --------------
;			DS:SI - address of file
;			ES:DI - convert buffer
Convert	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	AX
	MOV	DX,DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Convert_c
	POP	AX
	POP	DX
	POP	DS
	RET
Convert	ENDP

;-------------- Unconvert name & extension -------------
;			DS:SI - convert buffer
;			ES:DI - address of file
UnConv	PROC	NEAR
	PUSH	DX
	MOV	DX,DS
	CALL	FAR PTR UnConv_c
	POP	DX
	ADD	SI,11
	RET
UnConv	ENDP

;--------------------- Test mask file ------------------
;			DS:SI - address of file
;			ES:DI - converted mask
;	Return :	 CY=1 - yes
;			 CY=0 - no
TstMas	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	AX
	MOV	DX,DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR TstMas_c
	SHR	AX,1			; CY if it fits
	POP	AX
	POP	DX
	POP	DS
	RET
TstMas	ENDP

;------------------------ Set DTA ----------------------
SetDTA	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	AX
	PUSH	ES
	POP	DS
	MOV	DX,OFFSET DTA
	MOV	AH,1Ah
	INT	21h
	POP	AX
	POP	DX
	POP	DS
	RET
SetDTA	ENDP
	FromC	SetDTA

;------------------- Test drive letter -----------------
;			   AL - drive letter
;	Return :	   ZR - yes
;			   NZ - no
TstDrv	PROC	NEAR
	PUSH	DX
	PUSH	AX
	SUB	AL,'A'
	MOV	DL,AL
	MOV	AH,19h
	INT	21h
	PUSH	AX
	MOV	AH,0Eh
	INT	21h
	MOV	AH,19h
	INT	21h
	CMP	AL,DL
	POP	DX
	PUSHF
	MOV	AH,0Eh
	INT	21h
	POPF
	POP	AX
	POP	DX
	RET
TstDrv	ENDP

; C code asks whether there is a drive of the letter AL: 1 in AX for ZR.
	PUBLIC	TstDrv_a
TstDrv_a PROC	FAR
	CALL	TstDrv
	MOV	AX,0			; This leaves the flags
	JNE	TstDrA
	INC	AX
TstDrA:	RET
TstDrv_a ENDP

;--------------- Int 21h with fatal error --------------
Intr21	PROC	NEAR
	PUSH	DS
	MOV	DS,CS:SecDat
	MOV	BYTE PTR DS:[ErrDat],0
	POP	DS
	INT	21h
	PUSHF
	PUSH	DS
	MOV	DS,CS:SecDat
	CMP	BYTE PTR DS:[ErrDat],0
	POP	DS
	JNE	I21_02
	POPF
	JNC	I21_01
	PUSH	DS
	PUSH	ES
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	XOR	BX,BX
	MOV	AH,59h
	INT	21h
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	ES
	POP	DS
	STC
I21_01:	RET
I21_02:	POPF
	MOV	AX,53h
	STC
	RET
Intr21	ENDP

; C code calls DOS: Intr21 with the registers that struct DosRegs of
; port/vc.h holds, at SS:BX. AX, BX, CX and DX are put back there as they
; return, and AX is not 0 afterwards if the carry flag was set.
	PUBLIC	Intr21_a
; Intr21f_a is the same with the carry flag set first if AX is not 0, and
; clear if it is. A call of DOS that leaves the flag as it finds it, as
; those for an FCB do, comes back with what the caller had, and Intr21
; takes that for an error of DOS: F38_70 goes by it. Nothing from here to
; the INT of Intr21 changes the flag.
	PUBLIC	Intr21f_a
Intr21f_a LABEL	FAR
	NEG	AX
Intr21_a PROC	FAR
	PUSH	BP
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	BP,BX
	MOV	AX,[BP]
	MOV	BX,[BP+2]
	MOV	CX,[BP+4]
	MOV	SI,[BP+10]
	MOV	DI,[BP+12]
	LDS	DX,[BP+6]
	CALL	Intr21
	MOV	[BP],AX
	MOV	[BP+2],BX
	MOV	[BP+4],CX
	MOV	[BP+6],DX
	SBB	AX,AX
	POP	DS
	POP	BP
	RET
Intr21_a ENDP

;------------------- Invert variable -------------------
;			ES:BX - address of variable
InvVar	PROC	NEAR
	CMP	BYTE PTR ES:[BX],1
	MOV	AL,0
	ADC	AL,0
	MOV	ES:[BX],AL
	RET
InvVar	ENDP

;----------------------- Check sum ---------------------
;			DS:SI - address
;			   CX - length
;	Return :	   AX - check sum
ChkSum	PROC	NEAR
	PUSH	SI
	PUSH	CX
	PUSH	BX
	XOR	BX,BX
	MOV	AH,0
ChkSm1:	LODSB
	ADD	BX,AX
	LOOP	ChkSm1
	LODSW
	CMP	AX,BX
	MOV	AX,BX
	POP	BX
	POP	CX
	POP	SI
	RET
ChkSum	ENDP

;----------------- CALL to PSP segment -----------------
;			   AX - offset in PSP
CallPSP	PROC	NEAR
	POP	BP
	PUSH	CS
	PUSH	BP
	PUSH	SS
	PUSH	AX
	RETF
CallPSP	ENDP

;----------------------- Page Up -----------------------
PgUp	PROC	NEAR
	OR	DI,DI
	JNE	PgUp1
	XOR	SI,SI
	JMP	PgUp3
PgUp1:	DEC	AX
	SUB	DI,AX
	JAE	PgUp2
	XOR	DI,DI
PgUp2:	ADD	AX,DI
	CMP	SI,AX
	JBE	PgUp3
	MOV	SI,AX
PgUp3:	RET
PgUp	ENDP

;---------------------- Page Down ----------------------
PgDn	PROC	NEAR
	PUSH	SI
	MOV	SI,BP
	SUB	BP,AX
	JAE	PgDn1
	XOR	BP,BP
PgDn1:	DEC	AX
	CMP	DI,BP
	JB	PgDn2
	POP	AX
	OR	SI,SI
	JE	PgDn5
	DEC	SI
	JMP	PgDn5
PgDn2:	POP	SI
	ADD	DI,AX
	JC	PgDn3
	CMP	DI,BP
	JBE	PgDn4
PgDn3:	MOV	DI,BP
PgDn4:	CMP	SI,DI
	JAE	PgDn5
	MOV	SI,DI
PgDn5:	RET
PgDn	ENDP

; C code turns a page: PgUp and PgDn, which hand back SI in AX and DI in
; DX. PgDn takes in BX what it has in BP.
	PUBLIC	PgUp_a, PgDn_a
PgUp_a	PROC	FAR
	CALL	PgUp
	MOV	AX,SI
	MOV	DX,DI
	RET
PgUp_a	ENDP

PgDn_a	PROC	FAR
	PUSH	BP
	MOV	BP,BX
	CALL	PgDn
	MOV	AX,SI
	MOV	DX,DI
	POP	BP
	RET
PgDn_a	ENDP
