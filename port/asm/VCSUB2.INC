;--------------------- Subroutines ---------------------
;		Version :	16.10.1999
;-------------------------------------------------------

	PUBLIC	PtMnu_S

;------------------- Clear screen line -----------------
;			   BH - line
; C: ClsLine_c of port/bars.c. The routine read the colour in the caller's
; DS, so DS goes to the C as a number. The one caller that could come with
; DS on a panel, what makes the commands of a line of a menu, is C itself
; and hands that segment over.
ClsLine	PROC	NEAR
	MOV	BL,0
	MOV	AX,DS
	CallC	ClsLine_c		; DI: behind the line
	MOV	AH,DS:CrntCol
	MOV	AL,' '
	RET
ClsLine	ENDP

;-------------------- Output key bar -------------------
;			   AL - Case
; C: PutKey_c of port/bars.c, with PutKey0 and the names of the keys.
PutKey	PROC	NEAR
	PUSH	AX
	CallC	PutKey_c
	POP	AX
	RET
PutKey	ENDP

;-------------------- Output menu bar ------------------
;			   AL - Case
; C: PtMnu1_c of port/bars.c. The names of the menus stay here: the menu
; reads them too, to find the one that the mouse is on (F40_50).
	.CONST
PtMnu_S	DB	0FFh,4,'Left',0FFh,4,'Files',0FFh,4,'Commands'
	DB	0FFh,4,'Options',0FFh,4,'Right',0FFh,31,0

	.CODE

;------------------------ Panel ------------------------
;			   DS - WCBseg
Panel	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	Panel_c
	POP	AX
	RET
Panel	ENDP

;------------------- Set directory mask ----------------
;			   DS - WCBseg
SetMask	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	SetMask_c
	POP	AX
	RET
SetMask	ENDP

;----------------------- File sort ---------------------
;			   DS - WCBseg
Sort	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CALL	FAR PTR Sort_c
	POP	AX
	RET
Sort	ENDP

;----------------------- Find file ---------------------
;			ES:DI - File
;			   DS - WCBseg
;	Return :	   SI - Address of file , CF=1
;				Not found ,       CF=0
FnFile	PROC	NEAR
	PUSH	DX
	MOV	DX,DS
	CALL	FAR PTR FnFile_c
	POP	DX
	CMP	SI,DS:EndBuf		; CY if it is among the files
	RET
FnFile	ENDP

;--------------- Output path in the middle -------------
;			   DS - WCBseg
	.CONST
Tree_S	DB	' Tree ',0

	.CODE
PutPath	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	PutPath_c
	POP	AX
	RET
PutPath	ENDP

;	Return :	   SI - the text, at TmpErr+1
;			   DI - its end
PtPt10	PROC	NEAR
	MOV	AX,DS
	CallC	PtPt10_c
	MOV	SI,OFFSET TmpErr+1
	RET
PtPt10	ENDP

;---------------------- Output date --------------------
;			   AX - date
;	Return :	BX, DX, CX - day, month and year in the
;				     country's order, as text
;			   AX - century, as text
PutDate	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	SUB	SP,8
	MOV	BX,SP
	CALL	FAR PTR PutDate_c
	POP	AX
	POP	BX
	POP	DX
	POP	CX
	POP	DS
	RET
PutDate	ENDP

;			AX, BX, CX, DX - from PutDate
;			ES:DI - address of buffer
BufDate	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	PUSH	CX
	PUSH	DX
	PUSH	BX
	PUSH	AX
	MOV	BX,SP
	CALL	FAR PTR BufDate_c
	ADD	SP,8
	POP	DS
	RET
BufDate	ENDP

;---------------------- Output time --------------------
;			   AX - time
;	Return :	BX, DX, AX - hours, minutes, seconds as text
;			   CH - 'a', 'p' or ' '
PutTime	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	SUB	SP,8
	MOV	BX,SP
	CALL	FAR PTR PutTime_c
	POP	BX
	POP	DX
	POP	AX
	POP	CX
	POP	DS
	RET
PutTime	ENDP

;			AX, BX, CH, DX - from PutTime
;			ES:DI - address of buffer
;	Return :	   AL - 'a', 'p' or ' '
BufTime	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	PUSH	CX
	PUSH	AX
	PUSH	DX
	PUSH	BX
	MOV	BX,SP
	CALL	FAR PTR BufTime_c
	ADD	SP,8
	MOV	AL,CH
	POP	DS
	RET
BufTime	ENDP

;			   AL - number below 100
;	Return :	   AX - its two digits as text
HexDec	PROC	NEAR
	CALL	FAR PTR HexDec_c
	RET
HexDec	ENDP

;--------------- Correct first parameter ---------------
;			   DS - WCBseg
CorPar	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	CorPar_c
	POP	AX
	RET
CorPar	ENDP

;--------------- Set cursor in main window -------------
;			   DS - WCBseg
;			 AL=0 - reset cursor
;			 AL=1 - set cursor
SetCur	PROC	NEAR
	PUSH	AX
	PUSH	DX
	MOV	DX,DS
	CallC	SetCur_c
	POP	DX
	POP	AX
	RET
SetCur	ENDP

;----------------------- Test info ---------------------
;			   DS - WCBseg
;			   ES - main data
TstInfo	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	TstInfo_c
	POP	AX
	RET
TstInfo	ENDP

;-------------- BX files and DX directories ------------
;			   BX - number of files
;			   DX - number of directories
;			ES:DI - buffer
FilDir	PROC	NEAR
	CallC	FilDir_c
	RET
FilDir	ENDP

;------------ Find iten in extension file ------------
; C: FndItem_c of port/extfile.c, which only C code calls.

;------------------- Keyboard input --------------------
;			   AL - Key bar case
;	Return:		   AX - Key code
; C: Input_c of port/input.c, which calls the help for F1.
Input	PROC	NEAR
	CallC	Input_c
	RET
Input	ENDP

; Input0, which waits for the key, is C too, and only C code calls it.

; What Input0 and PutKey ask of the BIOS and of DOS, now
; that they are C (see FromC in VC.ASM, and port/input.c for what the
; entries take and return).
	FromC	LeaveTm
	FromC	Blank

LeaveTm	PROC	NEAR
	INT	28h
	CMP	DS:WinLeav,0
	JNZ	Leave1
	MOV	AX,1680h
	INT	2Fh
	CMP	AL,80h
	JNZ	Leave1
	MOV	DS:WinLeav,1
Leave1:	CMP	DS:DESQvie,0
	JZ	LvTm1
	MOV	AX,101Ah
	INT	15h
	MOV	AX,1000h
	INT	15h
	MOV	AX,1025h
	INT	15h
LvTm1:	RET
LeaveTm	ENDP

;--------------------- Blank screen --------------------
Blank	PROC	NEAR
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	AL,0
	CALL	CurMous
	MOV	AH,0Fh
	INT	10h
	MOV	AH,3
	INT	10h
	PUSH	CX
	PUSH	DX
	MOV	CX,2000h
	MOV	AH,1
	INT	10h
	XOR	CX,CX
	MOV	DH,DS:Lines
	DEC	DH
	MOV	DL,79
	MOV	AH,Sky1_C
	CALL	Color
	MOV	BH,AH
	MOV	AX,600h
	INT	10h
	MOV	DI,OFFSET Stars
	MOV	CX,NmbStrs*SIZE STAR/2
	XOR	AX,AX
	REP	STOSW
	XOR	CX,CX
	CALL	GetMous
	MOV	SI,CX
	MOV	AH,2
	INT	16h
	MOV	CL,AL
	XOR	DI,DI
Blnk01:	PUSH	SI
	PUSH	CX
	CALL	Rnd
	MOV	BL,ES:Lines
	MOV	BH,0
	MUL	BX
	MOV	CL,DL
	CALL	Rnd
	MOV	BX,80
	MUL	BX
	MOV	DH,CL
	CALL	Rnd
	MOV	CL,3
	ROR	AX,CL
	CMP	AL,100
	JB	Blnk05
	MOV	BL,AL
	CALL	Rnd
	MOV	CL,5
	ROR	AL,CL
	CMP	AL,50
	JB	Blnk05
	OR	AL,AH
	AND	AL,1
	MOV	BH,AL
	MOV	SI,OFFSET Stars
	MOV	CX,NmbStrs
	XOR	BP,BP
Blnk02:	LODSW
	OR	AX,AX
	LODSW
	JNE	Blnk03
	OR	BP,BP
	JNE	Blnk04
	MOV	BP,SI
	JMP	Blnk04
Blnk03:	CMP	AX,DX
	JE	Blnk05
Blnk04:	LOOP	Blnk02
	OR	BP,BP
	JE	Blnk05
	MOV	DS:[BP-SIZE STAR].StarMod,BX
	MOV	DS:[BP-SIZE STAR].StarPos,DX
	MOV	AH,0Fh
	INT	10h
	MOV	AH,2
	INT	10h
	MOV	AH,Sky1_C
	CALL	Color
	MOV	BL,AH
	MOV	CX,1
	MOV	AL,ES:StarTab[0]
	MOV	AH,9
	INT	10h
Blnk05:	MOV	SI,OFFSET Stars
	MOV	CX,NmbStrs
Blnk06:	PUSH	CX
	LODSW
	MOV	BX,AX
	LODSW
	OR	BX,BX
	JE	Blnk09
	MOV	DX,AX
	DEC	BX
	MOV	DS:[SI-SIZE STAR].StarClk,BL
	MOV	AL,ES:StarTab[1]
	CMP	BL,12
	JE	Blnk08
	MOV	AL,ES:StarTab[2]
	CMP	BL,9
	JNE	Blnk07
	OR	BH,BH
	JE	Blnk08
	MOV	BL,0
Blnk07:	MOV	AL,ES:StarTab[3]
	CMP	BL,6
	JE	Blnk08
	MOV	AL,ES:StarTab[4]
	CMP	BL,3
	JE	Blnk08
	OR	BL,BL
	JNE	Blnk09
	XOR	AX,AX
	MOV	DS:[SI-SIZE STAR].StarMod,AX
	MOV	DS:[SI-SIZE STAR].StarPos,AX
	MOV	AL,' '
Blnk08:	PUSH	AX
	MOV	AH,0Fh
	INT	10h
	MOV	AH,2
	INT	10h
	POP	AX
	MOV	AH,Sky2_C
	CALL	Color
	MOV	BL,AH
	MOV	CX,1
	MOV	AH,9
	INT	10h
Blnk09:	POP	CX
	LOOP	Blnk06
	POP	CX
	POP	SI
Blnk10:	MOV	AH,1
	INT	16h
	JNZ	Blnk13
	MOV	AH,2
	INT	16h
	XCHG	AL,CL
	AND	AL,0F0h
	XOR	AL,CL
	JNZ	Blnk14
	MOV	BX,CX
	XOR	AX,AX
	XOR	CX,CX
	CALL	GetMous
	JNZ	Blnk14
	PUSH	BX
	MOV	BX,76
	MOV	DX,204h
	CALL	InWind
	POP	BX
	JC	Blnk11
	MOV	SI,CX
Blnk11:	CMP	CX,SI
	JNE	Blnk14
	CALL	Timer
	MOV	CX,BX
	XCHG	DX,DI
	CMP	DX,DI
	JE	Blnk12
	JMP	Blnk01
Blnk12:	CALL	LeaveTm
	JMP	Blnk10
Blnk13:	MOV	AH,0
	INT	16h
Blnk14:	MOV	AH,0Fh
	INT	10h
	POP	DX
	MOV	AH,2
	INT	10h
	POP	CX
	MOV	AH,1
	INT	10h
	MOV	AL,1
	CALL	CurMous
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	RET
Blank	ENDP

; C: Rnd_c of port/input.c.
Rnd	PROC	NEAR
	CallC	Rnd_c
	RET
Rnd	ENDP

; EdSub1, which finds where a word begins, is C too: only C code calls it.

