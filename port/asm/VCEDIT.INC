;-------------------------------------------------------
;		Version :	16.10.1999
;-------------------------------------------------------

EditBuf	EQU	MenuBuf			; Edit line buffer
UndoBuf	EQU	ErrBuf			; Undo buffer
LenEdit	EQU	LenMenu			; Length of edit buffer
	; The length of undo buffer must be greate then edit buffer

EdData	STRUC
	DB	TmpBuf+100 DUP(?)
EdHelp	DB	0 DUP(?)		; Mini help
EdOffst	DD	?			; Offset of begin of the current line
EdEndBf	DD	?			; Offset of end of part of current line
EdSizLn	DW	?			; Length of part of line in buffer
EdUpdat	DB	?			; Update line buffer
EdWhole	DB	?			; The whole line allocates buffer
EdCursr	DW	?			; Offset of the cursor in line buffer
EdCol1	DW	?			; Cursor position for Up & Down
EdUpDwn	DB	?			; Up & Down flag
EdHex1	DB	?			; Cursor digit pointer for Up & Down
EdScrn	DB	?			; Output all edit window
EdCurLn	DB	?			; It doesn't need to output line
EdUndo	DW	?			; Length of line in undo buffer
	DB	77 DUP(?)
EdWrite	DB	?			; File was written
EdSpace	DW	?			; Edit buffer size in paragraphs
	DB	?
EdStat	DB	?			; Status line
EdHexAs	DB	?			; Hex / ASCII field in HEX mode
EdHxCur	DB	?			; Cursor points 1 or 2 digit in HEX
EdLen	DD	?			; Size of file in edit buffer
EdCol	DW	?			; Cursor position
EdLine	DW	?			; Cursor line
	DW	?
EdTxHex	DB	?			; Text / Hex mode
EdChng	DB	?			; File was changed
	DD	?
EdCol0	DW	?			; First column displayed on the screen
EdLin0	DW	?			; First line displayed on the screen
	DW	?
EdBase	DW	?			; The base of the edit window
EdSize	DW	?			; The size of the edit window
EdFirst	DD	?			; Offset of first line on the screen
EdShape	DW	?			; Original cursor shape
EdData	ENDS

	.CONST
Line_S	DB	"Line",0
FrBuf_S	DB	"Free",0
BgFi1_S	DB	"The file ^V",0
BgFi2_S	DB	"is too large for Edit.",0
BgFi3_S	DB	"View",0FFh,3,"Cancel",0
NewFi_S	DB	"New-file",0FFh,3,"Cancel",0
SvCn1_S	DB	"You've made changes since the last save.",0
SvCn2_S	DB	"Save",0FFh,3,"Don't save",0FFh,3,"Continue editing",0
SavAs_S	DB	"Save this file as",0FFh,LenStr-17,0
SavFi_S	DB	"Saving the file",0
SavEr_S	DB	"There was an error writing the file",0
EdtH_S	DB	"³  !.!  Filename with extension",0
	DB	"³  !:",0FFh,3,"Drive letter",0
	DB	"³  !\",0FFh,3,"Path name",0
	DB	"³  !!",0FFh,3,"!",0
	DB	"³  !",0FFh,4,"Filename w/o extension",0
	DB	"³  !@",0FFh,3,"File list",0
EdtH1_S DB	" User Menu Help ",0
	DB	"File format for user defined menus:",0
	DB	0
	DB	"' comment",0FFh,7,"Comment line",0
	DB	"m: Menu Label",0FFh,3,"Appears in the pop-up menu",0
	DB	0FFh,5,"edit !.!",0FFh,3,"Any DOS command",0
	DB	0FFh,5,"cls",0FFh,8,"Any additional commands",0
EdtH2_S	DB	" Extension File Help ",0
	DB	"Format of the extension file:",0
	DB	0
	DB	"' comment",0FFh,10,"Comment line",0
	DB	"txt: edit !.!",0FFh,6,"Any DOS command",0
	DB	" ",0FFh,3,"cls",0FFh,11,"Any additional commands",0
	DB	" À",0FFh,80h+16,"Ä File extension",0

	.DATA
EdtH_P	DB	HlpMn_C
Edit_T	DW	0009h, T03_00		; Tab
	DW	0011h, T04_00		; ^Q
	DW	4B00h, T05_00		; Left
	DW	0013h, T05_00		; ^S
	DW	4D00h, T06_00		; Right
	DW	0004h, T06_00		; ^D
	DW	7300h, T07_00		; ^Left
	DW	0001h, T07_00		; ^A
	DW	7400h, T08_00		; ^Right
	DW	0006h, T08_00		; ^F
	DW	4800h, T09_00		; Up
	DW	0005h, T09_00		; ^E
	DW	5000h, T10_00		; Down
	DW	0018h, T10_00		; ^X
	DW	4900h, T11_00		; PgUp
	DW	0012h, T11_00		; ^R
	DW	5100h, T12_00		; PgDn
	DW	0003h, T12_00		; ^C
	DW	4700h, T13_00		; Home
	DW	4F00h, T14_00		; End
	DW	8400h, T15_00		; ^PgUp
	DW	7600h, T16_00		; ^PgDn
	DW	7700h, T15_00		; ^Home
	DW	7500h, T16_00		; ^End
	DW	5300h, T20_00		; Del
	DW	0007h, T20_00		; ^G
	DW	0008h, T21_00		; BackSpace
	DW	007Fh, T22_00		; ^BS
	DW	0017h, T22_00		; ^W
	DW	0014h, T23_00		; ^T
	DW	000Bh, T24_00		; ^K
	DW	0019h, T25_00		; ^Y
	DW	0015h, T26_00		; ^U
	DW	3E00h, T30_00		; F4
	DW	4100h, T31_00		; F7
	DW	5A00h, T32_00		; Shift F7
	DW	5500h, T33_00		; Shift F2
	DW	3C00h, T34_00		; F2
	DW	001Bh, T35_00		; Esc
	DW	4400h, T35_00		; F10
	DW	5D00h, T35_01		; Shift F10
	DW	0
SavFi_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	ViewFil
	DB	7, 64
	DW	EdFl_S
	DW	SavFi_S
	DW	Var_S
SavAs_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	EdFl_S
	DW	SavAs_S
	DW	Empty_S
SavAs_B	BOXHD	< SavAs_P, 528h, 0, 1, 0FFh, 0, 0 >
	BOXMN	< 1, 0,0,0,0, 305h, ViewFil, 200h+LenStr >
	DB	0
SavCn_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	3, 0
	DW	WinBuf
	DW	EdFl_S
	DW	SvCn1_S
	DW	SvCn2_S
SavCn_B	BOXHD	< SavCn_P, 528h, 0, 1, 0FFh, 0, 1 >
	BOXMN	< 4, 0,0,0,0, 315h, TmpErr, SavCn_D >
	DB	0
BigFi_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	ViewFil
	DB	7, 64-9
	DW	EdFl_S
	DW	BgFi1_S
	DW	BgFi2_S
	DW	BgFi3_S
BigFi_B	BOXHD	< BigFi_P, 928h, 0, 1, 0FFh, 0, 0 >
	BOXMN	< 4 ,0,0,0,0, 420h, TmpErr, BigFi_D >
	DB	0
NewFi_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	ViewFil
	DB	7, 64
	DW	EdFl_S
	DW	NoFnd_S
	DW	Var_S
	DW	NewFi_S
NewFi_B	BOXHD	< NewFi_P, 928h, 0, 1, 0FFh, 0, 0 >
	BOXMN	< 4 ,0,0,0,0, 41Eh, TmpErr, NewFi_D >
	DB	0
SavEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	WinBuf+2000
	DW	ViewFil
	DB	7, 64
	DW	EdFl_S
	DW	SavEr_S
	DW	Var_S
	DW	Ok_S
SavEr_B	BOXHD	< SavEr_P, 928h, 0, 1, 0FFh, 0, 2 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpErr, OkMn_D >
	DB	0
SavCn_D	DB	 6, 0
	DB	12, 7
	DB	18,20
	DB	 0
BigFi_D	DB	 6, 0
	DB	 8, 7
	DB	 0
NewFi_D	DB	10, 0
	DB	 8,11
	DB	 0

	.CODE
T00_00:	MOV	BP,BX
	MOV	DI,AX
	MOV	AX,ES
	ADD	AX,DS:FreMem2
	SUB	AX,DS:ViewSeg
	MOV	DS:EdSpace,AX
	MOV	BL,AH
	MOV	BH,0
	XCHG	BX,DI
	MOV	CL,4
	SHL	AX,CL
	SHR	DI,CL
	CMP	DI,DS:EdLen.Hi
	JB	T00_01
	JA	T00_03
	CMP	AX,DS:EdLen.Lo
	JA	T00_03
T00_01:	PUSH	BP			; File too large
	MOV	BP,OFFSET BigFi_B
	CALL	Dialog
	POP	BP
	JE	T00_02
	JMP	T00_10
T00_02:	PUSH	CS
	POP	DS
	INC	BH
	MOV	BL,37
	MOV	SI,OFFSET ViFl_S
	CALL	MovDz
	PUSH	ES
	POP	DS
	MOV	DX,106h
	CALL	Window
	MOV	BX,BP
	JMP	V00_10
T00_03:	MOV	SI,OFFSET ViewFil	; Extension or menu file ?
	CALL	CutPath
	MOV	DI,OFFSET TmpBuf
	MOV	CX,13
	PUSH	DI
	CALL	MvzLine
	STOSB
	POP	SI
	CALL	PathLin
	MOV	CX,DI
	SUB	CX,SI
	MOV	DI,SI
	PUSH	CS
	POP	DS
	MOV	AX,0A01h
	MOV	SI,OFFSET Menu_F
	CALL	T64_00
	JE	T00_04
	MOV	AL,2
	MOV	SI,OFFSET Ext_F
	CALL	T64_00
	JE	T00_04
	MOV	SI,OFFSET ViewE_F
	CALL	T64_00
	JE	T00_04
	MOV	SI,OFFSET EditE_F
	CALL	T64_00
	JE	T00_04
	XOR	AX,AX
T00_04:	PUSH	ES
	POP	DS
	MOV	DS:EdHelp,AL
	SUB	DS:EdSize.Y,AH
	CMP	BP,0FFFFh		; New file ?
	JNE	T00_06
	MOV	AL,DS:Lines
	SUB	AL,2
	CMP	AL,DS:EdSize.Y
	JA	T00_05
	MOV	BP,OFFSET NewFi_B
	CALL	Dialog
	JNE	T00_10
T00_05:	JMP	T00_13
T00_06:	PUSH	DX			; Read file
	PUSH	BX
	MOV	BX,BP
	MOV	SI,DS:EdLen.Lo
	MOV	DI,DS:EdLen.Hi
	MOV	DS,DS:ViewSeg
T00_07:	MOV	CX,0FF00h
	OR	DI,DI
	JNE	T00_08
	CMP	SI,CX
	JAE	T00_08
	MOV	CX,SI
T00_08:	XOR	DX,DX
	MOV	AH,3Fh
	CALL	Intr21
	JC	T00_09
	CALL	TstBrk
	JC	T00_09
	CMP	AX,CX
	JNE	T00_11
	MOV	AX,DS
	ADD	AX,0FF0h
	MOV	DS,AX
	SUB	SI,CX
	SBB	DI,0
	MOV	AX,SI
	OR	AX,DI
	JNE	T00_07
	JMP	T00_12
T00_09:	POP	BX
	POP	DX
T00_10:	MOV	SI,OFFSET WinBuf
	CALL	ResWin
	CALL	Window
	JMP	V13_00
T00_11:	SUB	SI,AX
	SBB	DI,0
	SUB	ES:EdLen.Lo,SI
	SBB	ES:EdLen.Hi,DI
T00_12:	MOV	AH,3Eh
	CALL	Intr21
	POP	BX
	POP	DX
T00_13:	MOV	SI,OFFSET WinBuf
	CALL	ResWin
	CALL	Window
	PUSH	ES
	POP	DS
	MOV	AL,0
	CMP	DS:EdSize.X,79
	JB	T00_14
	MOV	DI,OFFSET ScrBuf	; Status line
	MOV	AH,Menu_C
	CALL	Color
	MOV	AL,' '
	MOV	CX,80
	REP	STOSW
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET EdFl_S+1
	XOR	BX,BX
	CALL	MovDz
	DEC	DI
	DEC	DI
	MOV	AL,':'
	STOSB
	MOV	SI,OFFSET Line_S
	MOV	BX,41
	CALL	MovDz
	MOV	SI,OFFSET Col_S
	MOV	BL,52
	CALL	MovDz
	MOV	SI,OFFSET FrBuf_S
	MOV	BL,70
	CALL	MovDz
	PUSH	ES
	POP	DS
	CALL	T74_00
	MOV	AL,1
T00_14:	MOV	DS:EdStat,AL
	CMP	DS:EdHelp,0		; Mini Help
	JE	T00_16
	MOV	BP,OFFSET EdtH_P
	MOV	BX,DS:EdBase
	ADD	BH,DS:EdSize.Y
	CALL	BasAdr
	MOV	AH,HlBrd_C
	CALL	Color
	MOV	CX,80
	PUSH	CX
	PUSH	AX
	MOV	AL,'Ü'
	REP	STOSW
	MOV	AH,CS:[BP]
	CALL	Color
	MOV	CX,1
	MOV	DX,76
	MOV	SI,6
	CALL	BoxDubl
	POP	AX
	POP	CX
	MOV	AL,'ß'
	REP	STOSW
	MOV	SI,OFFSET EdtH1_S
	CMP	DS:EdHelp,1
	JE	T00_15
	MOV	SI,OFFSET EdtH2_S
T00_15:	PUSH	CS
	POP	DS
	PUSH	BX
	INC	BH
	MOV	BL,40
	CALL	CntLine
	INC	BH
	MOV	BL,3
	CALL	T76_00
	MOV	SI,OFFSET EdtH_S
	MOV	BL,46
	CALL	T76_00
	PUSH	ES
	POP	DS
	POP	BX
	MOV	DX,0A50h
	CALL	Window
T00_16:	XOR	AX,AX			; Init vars
	MOV	DS:Quote,AL
	MOV	DS:EdWrite,AL
	MOV	DS:EdHexAs,AL
	MOV	DS:EdHxCur,AL
	MOV	DS:EdTxHex,AL
	MOV	DS:EdChng,AL
	MOV	DS:EdUpDwn,AL
	MOV	DS:EdCurLn,AL
	MOV	DS:EdLine,AX
	MOV	DS:EdCol0,AX
	MOV	DS:EdLin0,AX
	MOV	DS:EdFirst.Lo,AX
	MOV	DS:EdFirst.Hi,AX
	MOV	DS:EdOffst.Lo,AX
	MOV	DS:EdOffst.Hi,AX
	MOV	DS:EdCursr,AX
	MOV	DS:EdUndo,AX
	MOV	DS:Clock,AL
	MOV	DS:KeyBar,1
	MOV	DS:EdScrn,1
	MOV	DS:HlpPage,5
	CALL	T50_00
	CALL	ChgCur
	MOV	DS:EdShape,CX

T00_20:	CALL	T57_02			; Screen
	PUSH	ES
	POP	DS
	MOV	BH,0
	MOV	DS:EdCol,AX
	CMP	DS:EdUpDwn,BH
	JNZ	T00_21
	MOV	DS:EdCol1,AX
	MOV	DL,DS:EdHxCur
	MOV	DS:EdHex1,DL
T00_21:	MOV	DS:EdUpDwn,BH
	MOV	DX,DS:EdLine
	MOV	SI,DS:EdCol0
	MOV	DI,DS:EdLin0
	CMP	DS:EdTxHex,BH
	JNZ	T00_24
	CMP	AX,SI
	JB	T00_22
	JA	T00_23
	OR	SI,SI
	JE	T00_24
T00_22:	MOV	SI,AX
	OR	AX,AX
	JE	T00_24
	DEC	SI
	JMP	T00_24
T00_23:	MOV	BL,DS:EdSize.X
	DEC	BX
	LEA	CX,[BX+SI]
	CMP	AX,CX
	JB	T00_24
	DEC	BX
	MOV	SI,AX
	SUB	SI,BX
T00_24:	CMP	DX,DI
	JAE	T00_25
	MOV	DI,DX
	JMP	T00_26
T00_25:	MOV	BL,DS:EdSize.Y
	LEA	CX,[BX+DI]
	CMP	DX,CX
	JB	T00_26
	DEC	BX
	MOV	DI,DX
	SUB	DI,BX
	JAE	T00_26
	XOR	DI,DI
T00_26:	MOV	CX,DI
	XCHG	SI,DS:EdCol0
	XCHG	DI,DS:EdLin0
	SUB	CX,DI
	JE	T00_33
	PUSH	SI
	PUSH	CX
	PUSHF
	MOV	AX,DS:EdLine
	SUB	AX,DS:EdLin0
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	XOR	DI,DI
	POPF
	JB	T00_28
	CMP	CX,AX
	JA	T00_29
	CALL	T62_00
T00_27:	CALL	T53_00
	LOOP	T00_27
	JMP	T00_31
T00_28:	XOR	BX,BX
	XOR	DX,DX
	CMP	ES:EdLin0,BX
	JE	T00_32
	NEG	CX
	CMP	CX,AX
	JA	T00_29
	MOV	AX,CX
	CALL	T62_00
T00_29:	MOV	CX,AX
	JCXZ	T00_31
	DEC	DI
T00_30:	CALL	T54_00
	LOOP	T00_30
T00_31:	CALL	T60_00
T00_32:	PUSH	ES
	POP	DS
	MOV	DS:EdFirst.Lo,BX
	MOV	DS:EdFirst.Hi,DX
	POP	CX
	POP	SI
T00_33:	CMP	DS:EdScrn,0
	JNZ	T00_38
	CMP	SI,DS:EdCol0
	JNE	T00_40
	JCXZ	T00_36
	MOV	BX,DS:EdBase
	CMP	CX,1
	JNE	T00_34
	MOV	AH,6
	MOV	DX,2*80
	JMP	T00_35
T00_34:	CMP	CX,-1
	JNE	T00_40
	ADD	BH,DS:EdSize.Y
	DEC	BH
	MOV	AH,7
	MOV	DX,-2*80
T00_35:	MOV	AL,0
	CALL	T71_00
	MOV	DS:EdCurLn,0
T00_36:	CMP	DS:EdCurLn,0
	JNZ	T00_37
	CALL	T73_00
T00_37:	JMP	T00_49

T00_38:	MOV	CL,ES:EdSize.Y		; Clear edit window
	MOV	CH,0
	MOV	AL,0
T00_39:	PUSH	CX
	CALL	T70_00
	POP	CX
	INC	AX
	LOOP	T00_39

T00_40:	CALL	ClrKbd			; Output screen
	MOV	CL,DS:EdSize.Y
	MOV	CH,0
	MOV	AL,0
	CALL	T62_00
	MOV	BP,SI
T00_41:	MOV	SI,BP
	CALL	T63_00
	JE	T00_45
	JC	T00_46
	CALL	T53_10
	JNC	T00_43
	MOV	SI,BP
	CALL	T60_00
	CALL	T61_00
	MOV	BP,SI
	CALL	T53_10
	JNC	T00_43
	MOV	SI,BP
	MOV	BP,0FFFFh
	CALL	T72_00
T00_42:	MOV	SI,DS
	ADD	SI,0FF0h
	MOV	DS,SI
	XOR	SI,SI
	CALL	T53_10
	JC	T00_42
	XCHG	BP,SI
	JMP	T00_44
T00_43:	XCHG	BP,SI
	CALL	T72_00
T00_44:	INC	AX
	LOOP	T00_41
	JMP	T00_48
T00_45:	CMP	ES:EdTxHex,0
	JNZ	T00_46
	CALL	T60_00
	SUB	BX,2
	SBB	DX,0
	JB	T00_48
	CALL	T61_00
	PUSH	AX
	CALL	IsCrLfS
	POP	AX
	JMP	T00_47
T00_46:	CMP	ES:EdTxHex,0
	JZ	T00_48
	TEST	SI,0Fh
T00_47:	JNE	T00_48
	XOR	BP,BP
	CALL	T72_00
T00_48:	PUSH	ES
	POP	DS
	CALL	T73_10
	MOV	BX,DS:EdBase
	MOV	DX,DS:EdSize
	CALL	Window
T00_49:	MOV	DS:EdScrn,0
	MOV	DS:EdCurLn,0

	CMP	DS:EdStat,0		; Status line
	JNZ	T00_51
	JMP	T00_60
T00_51:	MOV	SI,OFFSET ScrBuf
	MOV	DI,OFFSET TmpErr
	MOV	CX,80
	PUSH	SI
	PUSH	DI
	PUSH	CX
	REP	MOVSW
	MOV	AL,' '
	CMP	DS:EdChng,CL
	JZ	T00_52
	MOV	AL,'*'
T00_52:	MOV	BYTE PTR DS:ScrBuf[2*38],AL
	MOV	AL,' '
	CMP	DS:Quote,CL
	JZ	T00_53
	MOV	AL,'"'
T00_53:	MOV	BYTE PTR DS:ScrBuf[2*39],AL
	MOV	AH,BYTE PTR DS:ScrBuf[1]
	MOV	CX,DS:EdLine
	MOV	DI,OFFSET ScrBuf+2*46
	CALL	T75_00
	MOV	CX,DS:EdCol
	MOV	DI,OFFSET ScrBuf+2*56
	CALL	T75_00
	PUSH	AX
	MOV	AX,DS:EdSpace
	ADD	AX,DS:ViewSeg
	MOV	DS,AX
	XOR	SI,SI
	CALL	T60_00
	PUSH	ES
	POP	DS
	SUB	BX,DS:EdLen.Lo
	SBB	DX,DS:EdLen.Hi
	CMP	DS:EdTxHex,CL
	JNZ	T00_54
	MOV	AX,DS:EdEndBf.Lo
	MOV	CX,DS:EdEndBf.Hi
	SUB	AX,DS:EdOffst.Lo
	SBB	CX,DS:EdOffst.Hi
	SUB	AX,DS:EdSizLn
	SBB	CX,SI
	ADD	BX,AX
	ADC	DX,CX
	CMP	DX,10h
	JB	T00_54
	MOV	BX,SI
	MOV	DX,SI
T00_54:	MOV	CX,BX
	MOV	BX,DX
	MOV	DI,OFFSET TmpBuf
	MOV	AL,3
	CALL	Number
	POP	AX
	MOV	SI,OFFSET TmpBuf+6
	MOV	DI,OFFSET ScrBuf+2*62
	MOV	CX,7
	CALL	MvsBwe
	POP	CX
	POP	SI
	POP	DI
	REPE	CMPSW
	JE	T00_60
	XOR	BX,BX
	MOV	DX,150h
	CALL	Window

T00_60:	MOV	CL,0			; Cursor
	MOV	AX,DS:EdCol
	SUB	AX,DS:EdCol0
	CMP	DS:EdTxHex,CL
	JZ	T00_61
	ADD	AL,63
	CMP	DS:EdHexAs,CL
	JNZ	T00_61
	SUB	AL,63
	MOV	DL,AL
	ADD	AL,DL
	ADD	AL,DL
	SHR	DL,1
	SHR	DL,1
	ADD	AL,DL
	ADD	AL,11
	CMP	DS:EdHxCur,CL
	JZ	T00_61
	INC	AX
T00_61:	MOV	BX,DS:EdLine
	SUB	BX,DS:EdLin0
	MOV	AH,BL
	ADD	AX,DS:EdBase
	CALL	Cursor

T00_70:	CALL	GetMous			; Mouse
	JNZ	T00_71
	MOV	DS:WinMous,0
	JMP	T00_80
T00_71:	CALL	ClrKbd
	MOV	DS:EdCurLn,1
	MOV	BX,DS:EdBase
	MOV	DX,DS:EdSize
	CMP	DS:WinMous,0
	JZ	T00_73
	CMP	CH,BH
	JAE	T00_72
	JMP	T09_00
T00_72:	PUSH	BX
	ADD	BH,DH
	CMP	CH,BH
	POP	BX
	JB	T00_73
	JMP	T10_00
T00_73:	CALL	InWind
	JC	T00_70
	MOV	DS:WinMous,1
	MOV	DS:EdHex1,0
	SUB	CX,BX
	MOV	AL,CL
	CBW
	CMP	DS:EdTxHex,0
	JNZ	T00_74
	ADD	AX,DS:EdCol0
	MOV	BX,DS:EdLine
	SUB	BX,DS:EdLin0
	CMP	CH,BL
	JNE	T00_77
	DEC	DX
	CMP	CL,DL
	JB	T00_77
	MOV	CX,100h
	JMP	T06_10
T00_74:	CMP	AL,63
	JAE	T00_76
	MOV	DS:EdHexAs,0
	SUB	AL,11
	JB	T00_75
	MOV	BL,13
	DIV	BL
	SHL	AL,1
	SHL	AL,1
	PUSH	AX
	MOV	AL,AH
	CMP	AL,12
	CMC
	SBB	AL,0
	CBW
	MOV	BL,3
	DIV	BL
	POP	BX
	MOV	DS:EdHex1,AH
	OR	AL,BL
	CBW
	JMP	T00_77
T00_75:	XOR	AX,AX
	JMP	T00_77
T00_76:	MOV	DS:EdHexAs,1
	SUB	AL,63
	CMP	AL,16
	JB	T00_77
	MOV	AL,15
T00_77:	MOV	DS:EdCol1,AX
	MOV	CL,CH
	MOV	CH,0
	MOV	AX,DS:EdLine
	SUB	AX,DS:EdLin0
	SUB	CX,AX
	MOV	AL,1
	JAE	T00_78
	NEG	CX
	MOV	AL,0
T00_78:	JMP	T10_02

T00_80:	MOV	CL,0			; Input
	MOV	AL,10
	CMP	DS:EdStat,CL
	JZ	T00_81
	MOV	AL,6
	CMP	DS:EdTxHex,CL
	JNZ	T00_81
	INC	AX
T00_81:	CALL	Input
	CMP	DS:DesqVie,CL
	JNZ	T00_82
	CALL	ClrKbd
T00_82:	CALL	HidCur
	CMP	AH,0FCh
	JE	T01_02
	CMP	DS:Quote,CL
	JZ	T00_83
	MOV	DS:Quote,CL
	CMP	AL,0
	JE	T01_02
	CMP	DS:EdTxHex,CL
	JZ	T01_03
	JMP	T01_10
T00_83:	MOV	SI,OFFSET Edit_T
	CALL	Case

T01_00:	CMP	AL,0			; Edit
	JE	T01_02
	CMP	DS:EdTxHex,CL
	JNZ	T01_10
	CMP	AL,0Dh			; ASCII Edit
	JNE	T01_01
	JMP	T02_00
T01_01:	OR	AH,AH
	JE	T01_03
	CMP	AL,' '
	JAE	T01_03
T01_02:	JMP	T00_20
T01_03:	CMP	DS:EdSizLn,LenEdit
	JB	T01_04
	CALL	T51_00
	CALL	T50_00
T01_04:	INC	DS:EdSizLn
	MOV	BX,DS:EdCursr
	ADD	BX,OFFSET EditBuf
	MOV	SI,OFFSET EditBuf[LenEdit-2]
	MOV	DI,OFFSET EditBuf[LenEdit-1]
	MOV	CX,SI
	SUB	CX,BX
	INC	CX
	JCXZ	T01_05
	STD
	REP	MOVSB
	CLD
T01_05:	MOV	DS:[BX],AL
	MOV	DS:EdUpdat,1
	JMP	T01_20
T01_10:	MOV	BX,DS:EdOffst.Lo	; Hex Edit
	MOV	DX,DS:EdOffst.Hi
	ADD	BX,DS:EdCursr
	ADC	DX,0
	CALL	T61_00
	CMP	ES:EdHexAs,CL
	JNZ	T01_12
	CALL	TstHex
	JC	T01_02
	MOV	AH,0Fh
	CMP	ES:EdHxCur,CL
	JNZ	T01_11
	MOV	CL,4
	SHL	AX,CL
T01_11:	NOT	AH
	AND	AH,DS:[SI]
	OR	AL,AH
T01_12:	MOV	DS:[SI],AL
	CALL	T63_01
	JNC	T01_15
	CMP	ES:EdHexAs,0
	JNZ	T01_13
	AND	BYTE PTR DS:[SI],0F0h
T01_13:	MOV	AX,DS
	PUSH	ES
	POP	DS
	CMP	SI,0Fh
	JB	T01_14
	SUB	AX,DS:ViewSeg
	INC	AX
	CMP	AX,DS:EdSpace
	JB	T01_14
	CALL	Beep
	JMP	T01_15
T01_14:	ADD	DS:EdLen.Lo,1
	ADC	DS:EdLen.Hi,0
	TEST	DS:EdLen.Lo,0Fh
	JNE	T01_15
	MOV	AX,DS:EdLine
	SUB	AX,DS:EdLin0
	INC	AX
	CMP	AL,DS:EdSize.Y
	JAE	T01_15
	INC	DS:EdLine
	ADD	DS:EdOffst.Lo,10h
	CALL	T73_00
	SUB	DS:EdOffst.Lo,10h
	DEC	DS:EdLine
T01_15:	CALL	T73_00
T01_20:	PUSH	ES
	POP	DS
	MOV	DS:EdChng,1
	JMP	T06_00

T02_00:	CMP	DS:EdSizLn,LenEdit-1	; Enter
	JB	T02_01
	CALL	T51_00
	CALL	T50_00
T02_01:	ADD	DS:EdSizLn,2
	MOV	BX,DS:EdCursr
	ADD	BX,OFFSET EditBuf
	MOV	SI,OFFSET EditBuf[LenEdit-3]
	MOV	DI,OFFSET EditBuf[LenEdit-1]
	MOV	CX,SI
	SUB	CX,BX
	INC	CX
	JCXZ	T02_02
	STD
	REP	MOVSB
	CLD
T02_02:	MOV	WORD PTR DS:[BX],0A0Dh
	MOV	DS:EdUpdat,1
	CALL	T51_00
T02_03:	CALL	T50_00
	CALL	T73_00
	MOV	AX,DS:EdLine
	SUB	AX,DS:EdLin0
	INC	AX
	INC	AX
	CMP	AL,DS:EdSize.Y
	JAE	T02_04
	DEC	AX
	MOV	BX,DS:EdBase
	ADD	BH,DS:EdSize.Y
	DEC	BH
	MOV	AH,7
	MOV	DX,-2*80
	CALL	T71_00
T02_04:	JMP	T06_14

T03_00:	CMP	DS:EdTxHex,0		; Tab
	JZ	T03_10
	MOV	BX,OFFSET EdHexAs
	CALL	InvVar
	JMP	T00_20
T03_10:	JMP	T01_03

T04_00:	MOV	DS:Quote,1		; Quote
	JMP	T00_20

T05_00:	XOR	CX,CX			; Left
	CMP	DS:EdTxHex,CL
	JZ	T05_10
	CMP	DS:EdHexAs,CL
	JNZ	T05_01
	CMP	DS:EdHxCur,CL
	JZ	T05_01
	MOV	DS:EdHxCur,CL
	JMP	T00_20
T05_01:	MOV	AL,0
	CMP	DS:EdHexAs,CL
	JNZ	T05_02
	MOV	AL,1
T05_02:	CMP	DS:EdCursr,CX
	JE	T05_05
	MOV	DS:EdHxCur,AL
T05_03:	DEC	DS:EdCursr
T05_04:	JMP	T00_20
T05_05:	MOV	AX,DS:EdOffst.Lo
	OR	AX,DS:EdOffst.Hi
	JE	T05_04
	MOV	DS:EdHxCur,1
	JMP	T05_11
T05_10:	CMP	DS:EdCursr,CX
	JNE	T05_03
T05_11:	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T61_00
	CALL	T54_00
	JC	T05_04
	CALL	T51_00
	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdLine,DI
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	JMP	T14_11

T06_00:	XOR	CX,CX			; Right
	CMP	DS:EdTxHex,CL
	JZ	T06_10
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdCursr
	ADD	BX,DI
	ADC	DX,CX
	CALL	T63_01
	JC	T06_02
	CMP	DS:EdHexAs,CL
	JNZ	T06_01
	CMP	DS:EdHxCur,CL
	JNZ	T06_01
	INC	DS:EdHxCur
	JMP	T00_20
T06_01:	MOV	DS:EdHxCur,CL
	INC	DI
	CMP	DI,10h
	JAE	T06_14
	MOV	DS:EdCursr,DI
T06_02:	JMP	T00_20
T06_10:	MOV	AX,DS:EdCursr
	CMP	AX,DS:EdSizLn
	JB	T06_11
	CMP	DS:EdWhole,CL
	JNZ	T06_13
	CALL	T51_00
	CALL	T50_00
T06_11:	CALL	T57_02
	CMP	AX,LenEdit-100-1
	JAE	T06_12
	INC	DS:EdCursr
T06_12:	JMP	T00_20
T06_13:	OR	CH,CH
	JNZ	T06_12
T06_14:	CALL	T51_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T61_00
	CALL	T53_00
	JC	T06_15
	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdLine,DI
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	MOV	DS:EdCursr,0
T06_15:	CALL	T50_00
	JMP	T00_20

T07_00:	CMP	DS:EdTxHex,0		; ^Left
	JNZ	T08_03
	MOV	DI,OFFSET EditBuf
	MOV	DX,DS:EdCursr
	CALL	EdSub1
	JMP	T08_02

T08_00:	CMP	DS:EdTxHex,0		; ^Right
	JNZ	T08_03
	MOV	DI,OFFSET EditBuf
	MOV	DX,DS:EdCursr
	CALL	T55_00
	MOV	DS:EdCursr,DX
T08_01:	CALL	T57_00
	MOV	DX,DS:EdSizLn
	SUB	DX,CX
	CMP	DX,DS:EdCursr
	JAE	T08_03
T08_02:	MOV	DS:EdCursr,DX
T08_03:	JMP	T00_20

T09_00:	MOV	AL,0			; Up
	JMP	T10_01

T10_00:	MOV	AL,1			; Down
T10_01:	MOV	CX,1
T10_02:	CALL	T51_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T61_00
	JCXZ	T10_06
T10_03:	CMP	AL,0
	JNZ	T10_04
	CALL	T54_00
	JMP	T10_05
T10_04:	CALL	T53_00
T10_05:	JC	T10_06
	LOOP	T10_03
T10_06:	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdLine,DI
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	MOV	DS:EdUpDwn,1
	MOV	DX,DS:EdCol1
	XOR	CX,CX
	CMP	DS:EdTxHex,CL
	JNZ	T10_10
	INC	DX
	JMP	T14_12
T10_10:	MOV	DS:EdCursr,DX
	MOV	AL,DS:EdHex1
	MOV	DS:EdHxCur,AL
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	CALL	T53_10
	PUSH	ES
	POP	DS
	JC	T10_11
	TEST	SI,0Fh
	JZ	T10_12
T10_11:	AND	SI,0Fh
	CMP	SI,DS:EdCursr
	JA	T10_12
	MOV	DS:EdCursr,SI
	MOV	DS:EdHxCur,CL
T10_12:	JMP	T00_20

T11_00:	MOV	AL,0			; PgUp
	JMP	T12_01

T12_00:	MOV	AL,1			; PgDn
T12_01:	CALL	T51_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	MOV	CL,DS:EdSize.Y
	MOV	CH,0
	DEC	CX
	CALL	T61_00
	MOV	BX,ES:EdLine
	SUB	BX,ES:EdLin0
	CMP	AL,0
	JNZ	T12_02
	OR	BX,BX
	JE	T12_03
	MOV	CX,BX
	JMP	T12_03
T12_02:	CMP	CX,BX
	JBE	T12_03
	SUB	CX,BX
T12_03:	CMP	AL,0
	JNZ	T12_04
	CALL	T54_00
	JMP	T12_05
T12_04:	CALL	T53_00
T12_05:	JC	T12_06
	LOOP	T12_03
T12_06:	XOR	CX,CX
	CMP	DI,ES:EdLine
	JE	T12_07
	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdLine,DI
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	MOV	DS:EdCursr,CX
	MOV	DS:EdHxCur,CL
T12_07:	CALL	T50_00
	JMP	T00_20

T13_00:	MOV	DS:EdCursr,0		; Home
	MOV	DS:EdHxCur,0
	JMP	T00_20

T14_00:	XOR	CX,CX			; End
	CMP	DS:EdTxHex,CL
	JZ	T14_10
	MOV	DS:EdHxCur,CL
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	CALL	T53_10
	JC	T14_01
	AND	SI,0Fh
	JNZ	T14_01
	MOV	SI,0Fh
	INC	ES:EdHxCur
T14_01:	MOV	ES:EdCursr,SI
T14_02:	JMP	T00_20
T14_10:	CALL	T57_00
	JAE	T14_13
	CMP	DS:EdWhole,0
	JNZ	T14_13
	CALL	T51_00
T14_11:	CMP	DS:EdTxHex,0
	JNZ	T14_00
	MOV	DX,LenEdit-100
T14_12:	CALL	T50_00
	CALL	T57_01
T14_13:	MOV	AX,DS:EdSizLn
	SUB	AX,CX
	MOV	DS:EdCursr,AX
	JMP	T00_20

T15_00:	XOR	CX,CX			; ^Home, ^PgUp
	CALL	T51_00
	MOV	DS:EdOffst.Lo,CX
	MOV	DS:EdOffst.Hi,CX
	CALL	T50_00
	MOV	DS:EdCursr,CX
	MOV	DS:EdLine,CX
	MOV	DS:EdHxCur,CL
	JMP	T00_20

T16_00:	CMP	DS:EdTxHex,0		; ^End, ^PgDn
	JNZ	T16_10
	CALL	T51_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T61_00
T16_01:	CALL	T53_00
	JNC	T16_01
	CALL	T60_00
	JMP	T16_11
T16_10:	MOV	BX,DS:EdLen.Lo
	MOV	DX,DS:EdLen.Hi
	CALL	T61_00
	MOV	DI,DS
	SUB	DI,ES:ViewSeg
	AND	BX,0FFF0h
T16_11:	PUSH	ES
	POP	DS
	MOV	DS:EdLine,DI
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	JMP	T14_11

T20_00:	CMP	DS:EdTxHex,0		; Del
	JNZ	T20_04
	MOV	AL,1
	MOV	BX,DS:EdCursr
	CMP	BX,DS:EdSizLn
	JAE	T20_05
T20_01:	LEA	DI,EditBuf[BX]
	LEA	SI,[DI+1]
T20_02:	PUSH	SI
	PUSH	DI
	MOV	CX,SI
	SUB	CX,DI
	MOV	DS:EdUndo,CX
	MOV	AX,CX
	MOV	SI,DI
	MOV	DI,OFFSET UndoBuf
	REP	MOVSB
	POP	DI
	POP	SI
	MOV	CX,DS:EdSizLn
	ADD	CX,OFFSET EditBuf
	SUB	CX,SI
	JBE	T20_03
	REP	MOVSB
T20_03:	SUB	DS:EdSizLn,AX
	MOV	DS:EdUpdat,1
	MOV	DS:EdChng,1
T20_04:	JMP	T00_20
T20_05:	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	CALL	T53_00
	JC	T20_04
	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdUndo,2
	MOV	WORD PTR DS:UndoBuf[0],0A0Dh
T20_06:	MOV	DS:EdEndBf.Lo,BX
	MOV	DS:EdEndBf.Hi,DX
	MOV	DS:EdUpdat,1
	CALL	T51_00
	CALL	T50_00
	OR	AX,AX
	JZ	T20_10
	CALL	T73_00
	MOV	AX,DS:EdLine
	PUSH	AX
	SUB	AX,DS:EdLin0
	PUSH	AX
	INC	AX
	CMP	AL,DS:EdSize.Y
	JAE	T20_07
	MOV	BX,DS:EdBase
	ADD	BH,AL
	MOV	AH,6
	MOV	DX,2*80
	CALL	T71_00
T20_07:	MOV	CL,DS:EdSize.Y
	MOV	CH,0
	DEC	CX
	POP	AX
	POP	DI
	SUB	CX,AX
	JBE	T20_10
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	MOV	BX,ES:EdLen.Lo
	MOV	DX,ES:EdLen.Hi
T20_08:	CALL	T53_00
	JC	T20_09
	LOOP	T20_08
	CALL	T60_00
T20_09:	PUSH	ES
	POP	DS
	ADD	DI,CX
	XCHG	BX,DS:EdOffst.Lo
	XCHG	DX,DS:EdOffst.Hi
	XCHG	DI,DS:EdLine
	CALL	T50_00
	CALL	T73_00
	XCHG	BX,DS:EdOffst.Lo
	XCHG	DX,DS:EdOffst.Hi
	XCHG	DI,DS:EdLine
	CALL	T50_00
T20_10:	JMP	T00_20

T21_00:	XOR	CX,CX			; BackSpace
	CMP	DS:EdTxHex,CL
	JZ	T21_10
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	ADD	BX,DS:EdCursr
	ADC	DX,CX
	MOV	AX,BX
	OR	AX,DX
	JE	T20_10
	CALL	T63_01
	JNC	T21_02
	MOV	AX,1
	MOV	DS:EdChng,AL
	SUB	DS:EdLen.Lo,AX
	SBB	DS:EdLen.Hi,CX
	TEST	BL,0Fh
	JNE	T21_01
	MOV	AX,DS:EdLine
	SUB	AX,DS:EdLin0
	CALL	T70_00
	MOV	DL,DS:EdSize.X
	MOV	DH,1
	CALL	Window
T21_01:	MOV	AL,0
	JMP	T05_02
T21_02:	JMP	T05_00
T21_10:	MOV	BX,DS:EdCursr
	OR	BX,BX
	JE	T21_11
	DEC	BX
	MOV	DS:EdCursr,BX
	JMP	T20_01
T21_11:	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T61_00
	CALL	T54_00
	JC	T21_13
	CALL	T51_00
	CALL	T60_00
	PUSH	ES
	POP	DS
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	MOV	DS:EdLine,DI
	CALL	T50_00
	CALL	T57_00
	MOV	AX,DS:EdSizLn
	SUB	AX,CX
	MOV	DS:EdCursr,AX
	MOV	AX,DS:EdLine
	MOV	BX,AX
	INC	AX
	SUB	AX,DS:EdLin0
	JNE	T21_12
	MOV	DS:EdLin0,BX
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DS:EdFirst.Lo,BX
	MOV	DS:EdFirst.Hi,DX
T21_12:	JMP	T20_05
T21_13:	JMP	T00_20

T22_00:	CMP	DS:EdTxHex,0		; ^BS
	JNZ	T21_13
	MOV	DI,OFFSET EditBuf
	MOV	DX,DS:EdCursr
	CALL	EdSub1
	JE	T21_13
	MOV	DS:EdCursr,DX
	JMP	T20_02

T23_00:	CMP	DS:EdTxHex,0		; ^T
	JNZ	T21_13
	MOV	DI,OFFSET EditBuf
	MOV	DX,DS:EdCursr
	CALL	T55_00
	JE	T21_13
	JMP	T20_02

T24_00:	CMP	DS:EdTxHex,0		; ^K
	JNZ	T24_03
	CMP	DS:EdWhole,0
	JNZ	T24_02
	CALL	T51_00
	CALL	T50_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	CALL	T53_00
	MOV	BX,ES:EdLen.Lo
	MOV	DX,ES:EdLen.Hi
	JC	T24_01
	CALL	T60_00
	SUB	BX,2
	SBB	DX,0
T24_01:	PUSH	ES
	POP	DS
	MOV	DS:EdEndBf.Lo,BX
	MOV	DS:EdEndBf.Hi,DX
	MOV	DS:EdWhole,1
T24_02:	MOV	SI,DS:EdSizLn
	MOV	DI,DS:EdCursr
	CMP	DI,SI
	JAE	T24_03
	ADD	SI,OFFSET EditBuf
	ADD	DI,OFFSET EditBuf
	JMP	T20_02
T24_03:	JMP	T00_20

T25_00:	CMP	DS:EdTxHex,0		; ^Y
	JNZ	T24_03
	CMP	DS:EdWhole,0
	JNZ	T25_01
	CALL	T51_00
	CALL	T50_00
T25_01:	MOV	SI,OFFSET EditBuf
	MOV	DI,OFFSET UndoBuf
	MOV	CX,DS:EdSizLn
	PUSH	CX
	REP	MOVSB
	POP	SI
	LEA	AX,[SI+2]
	MOV	DS:EdUndo,AX
	MOV	WORD PTR DS:UndoBuf[SI],0A0Dh
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	CALL	T61_00
	CALL	T53_00
	MOV	BX,ES:EdLen.Lo
	MOV	DX,ES:EdLen.Hi
	JC	T25_02
	CALL	T60_00
T25_02:	PUSH	ES
	POP	DS
	MOV	ES:EdSizLn,0
	MOV	ES:EdCursr,0
	MOV	AL,1
	JMP	T20_06

T26_00:	CMP	DS:EdTxHex,0		; ^U
	JNZ	T26_03
	MOV	CX,DS:EdUndo
	JCXZ	T26_03
	CMP	CX,2
	JB	T26_01
	MOV	BX,CX
	MOV	AX,WORD PTR DS:UndoBuf[BX-2]
	CALL	IsCrLf
	JE	T26_04
T26_01:	MOV	AX,LenEdit+1
	SUB	AX,CX
	CMP	DS:EdSizLn,AX
	MOV	AL,0
	JAE	T26_04
	ADD	DS:EdSizLn,CX
	MOV	SI,OFFSET EditBuf[LenEdit-1]
	MOV	DI,SI
	SUB	SI,CX
	PUSH	CX
	MOV	BX,DS:EdCursr
	ADD	BX,OFFSET EditBuf-1
	MOV	CX,SI
	SUB	CX,BX
	JBE	T26_02
	STD
	REP	MOVSB
	CLD
T26_02:	POP	CX
	ADD	DS:EdCursr,CX
	LEA	DI,[SI+1]
	MOV	SI,OFFSET UndoBuf
	REP	MOVSB
	MOV	DS:EdUpdat,1
	JMP	T08_01
T26_03:	JMP	T00_20
T26_04:	CALL	T51_00
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	PUSH	DX
	PUSH	BX
	ADD	BX,DS:EdCursr
	ADC	DX,0
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	MOV	DS:EdEndBf.Lo,BX
	MOV	DS:EdEndBf.Hi,DX
	MOV	SI,OFFSET UndoBuf
	MOV	DI,OFFSET EditBuf
	MOV	DS:EdSizLn,CX
	PUSH	CX
	REP	MOVSB
	POP	CX
	MOV	DS:EdUpdat,1
	CALL	T51_00
	POP	BX
	POP	DX
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	CMP	AL,0
	JZ	T26_05
	JMP	T02_03
T26_05:	CALL	T50_00
	ADD	DS:EdCursr,CX
	JMP	T08_01

T30_00:	CMP	DS:EdSize.X,80		; F4 - ASCII / Hex
	JB	T30_12
	CALL	T51_00
	XOR	DX,DX
	MOV	DS:EdCol0,DX
	MOV	DS:EdScrn,1
	MOV	BX,OFFSET EdTxHex
	CALL	InvVar
	JZ	T30_10
	MOV	DS:EdHxCur,DL		; ASCII to Hex
	MOV	BX,DS:EdCursr
	ADD	BX,DS:EdOffst.Lo
	ADC	DX,DS:EdOffst.Hi
	CALL	T61_00
	MOV	ES:EdCursr,SI
	XOR	SI,SI
	CALL	T60_00
	MOV	AX,DS
	PUSH	ES
	POP	DS
	SUB	AX,DS:ViewSeg
	MOV	DS:EdLine,AX
	MOV	DS:EdLin0,AX
	MOV	DS:EdOffst.Lo,BX
	MOV	DS:EdOffst.Hi,DX
	JMP	T30_11
T30_10:	MOV	AX,DS:EdOffst.Lo	; Hex to ASCII
	MOV	CX,DS:EdOffst.Hi
	ADD	AX,DS:EdCursr
	ADC	CX,DX
	XOR	BX,BX
	XOR	DI,DI
	CALL	T56_00
	MOV	AX,DS:EdLine
	MOV	DS:EdLin0,AX
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
T30_11:	MOV	DS:EdFirst.Lo,BX
	MOV	DS:EdFirst.Hi,DX
T30_12:	JMP	T00_20

T31_00:	MOV	DX,OFFSET EdFl_S	; F7 - search
	MOV	BP,OFFSET Srch_B
	CALL	Dialog
	JNE	T30_12

T32_00:	CMP	DS:SrchStr,0		; Shift F7 - continue search
	JE	T30_12
	CALL	T51_00
	MOV	DX,OFFSET EdFl_S
	CALL	V80_00
	PUSH	SI
	PUSH	DX
	PUSH	BX
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	ADD	BX,DS:EdCursr
	ADC	DX,0
T32_01:	CALL	TstBrk
	JC	T32_04
	MOV	AX,ES:EdLen.Lo
	MOV	CX,ES:EdLen.Hi
	SUB	AX,BX
	SBB	CX,DX
	MOV	CX,8000h
	JB	T32_03
	JA	T32_02
	CMP	AX,CX
	JAE	T32_02
	MOV	CX,AX
T32_02:	CALL	T61_00
	CALL	V82_00
	JNC	T32_05
	SUB	CX,WORD PTR ES:HelpBuf[90]
	JB	T32_03
	INC	CX
	ADD	BX,CX
	ADC	DX,0
	JMP	T32_01
T32_03:	PUSH	ES
	POP	DS
	MOV	DX,OFFSET EdFl_S
	MOV	BP,OFFSET SrEr_B
	CALL	Dialog
T32_04:	JMP	T32_07
T32_05:	CALL	TstBrk
	JC	T32_04
	CALL	T60_00
	PUSH	ES
	POP	DS
	CMP	DS:EdTxHex,0
	JZ	T32_06
	MOV	DS:EdHexAs,AL
	MOV	DS:EdHxCur,0
	MOV	AX,BX
	AND	AX,WORD PTR 0FFF0h
	MOV	DS:EdOffst.Lo,AX
	MOV	DS:EdOffst.Hi,DX
	CALL	T61_00
	MOV	AX,DS
	PUSH	ES
	POP	DS
	SUB	AX,DS:ViewSeg
	MOV	DS:EdLine,AX
	AND	BX,0Fh
	MOV	DS:EdCursr,BX
	JMP	T32_07
T32_06:	MOV	AX,BX
	MOV	CX,DX
	MOV	BX,DS:EdOffst.Lo
	MOV	DX,DS:EdOffst.Hi
	MOV	DI,DS:EdLine
	CALL	T56_00
T32_07:	CALL	T50_00
	POP	BX
	POP	DX
	POP	SI
	CALL	ResWin
	CALL	Window
	JMP	T00_20

T33_00:	MOV	BP,OFFSET SavAs_B	; Shift F2 - save...
	CALL	Dialog
	JE	T34_00
	JMP	T00_20

T34_00:	CALL	T80_00			; F2 - save
	JC	T34_01
	MOV	DS:EdChng,0
T34_01:	CMP	DS:EdStat,0
	JZ	T34_02
	CALL	T74_00
T34_02:	JMP	T00_20

T35_00:	CMP	DS:EdChng,0		; F10, Esc - quit
	JZ	T35_02
	MOV	BP,OFFSET SavCn_B
	CALL	Dialog
	CMP	AL,1
	JE	T35_02
	JA	T35_03
T35_01:	CALL	T80_00			; Shift F10 - save & quit
	JC	T35_03
T35_02:	MOV	CX,DS:EdShape
	MOV	AH,1
	INT	10h
	MOV	CL,DS:EdWrite
	JMP	V13_01
T35_03:	JMP	T00_20

T50_00	PROC	NEAR			; Read string in buffer
	PUSH	AX
	CallC	T50_00_c
	POP	AX
	RET
T50_00	ENDP

T51_00	PROC	NEAR			; Updata line from buffer
	PUSH	AX
	CallC	T51_00_c
	POP	AX
	RET
T51_00	ENDP

T53_00	PROC	NEAR			; Find next line
	PUSH	BX
	PUSH	AX
	PUSH	DI			; The line, as struct Where has it
	PUSH	DS
	PUSH	SI
	MOV	BX,SP
	CallC	T53_00_c		; AX: not 0 if there is none
	POP	SI
	POP	DS
	POP	DI
	NEG	AX			; CY then
	POP	AX
	POP	BX
	RET
T53_00	ENDP

T53_10	PROC	NEAR
	PUSH	DX
	PUSH	AX
	MOV	DX,DS
	CallC	T53_10_c		; AX: where the line ends, DX: not 0 if
	MOV	SI,AX			; it does not end in the segment
	NEG	DX			; CY then
	POP	AX
	POP	DX
	RET
T53_10	ENDP

T54_00	PROC	NEAR			; Find previous line
	PUSH	BX
	PUSH	AX
	PUSH	DI			; The line, as struct Where has it
	PUSH	DS
	PUSH	SI
	MOV	BX,SP
	CallC	T54_00_c		; AX: not 0 if there is none
	POP	SI
	POP	DS
	POP	DI
	NEG	AX			; CY then
	POP	AX
	POP	BX
	RET
T54_00	ENDP

T55_00	PROC	NEAR			; Next word
	MOV	SI,DI
	ADD	DI,DX			; Where the cursor is in the buffer
	MOV	AX,DX
	CallC	T55_00_c		; AX: where it gets to
	MOV	DX,AX
	ADD	SI,AX
	CMP	SI,DI			; ZR if that is the same
	RET
T55_00	ENDP

; C code asks whether a character stands between words: CmpSym for AL.
	PUBLIC	CmpSym_a
CmpSym_a PROC	FAR
	CALL	CmpSym
	MOV	AX,0			; ZR as a number
	JNE	CmSy_a
	INC	AX
CmSy_a:	RET
CmpSym_a ENDP

T56_00	PROC	NEAR			; Calculte coordinats
	PUSH	ES
	POP	DS
	CALL	FAR PTR T56_00_c
	RET
T56_00	ENDP

T57_00	PROC	NEAR			; Calculate cursor position
	CallC	T57_00_c		; AX: the column, DX: characters left
	MOV	CX,DX
	CMP	CX,1			; CY for a line that is not empty and
	JNC	T57_09			; ends before the last column: T14_10
	CMP	ES:EdSizLn,1		; goes by that
	CMC
T57_09:	RET
T57_00	ENDP

T57_01	PROC	NEAR			; The same, as far as the column DX
	MOV	AX,DX
	CallC	T57_01_c
	MOV	CX,DX
	RET
T57_01	ENDP

T57_02	PROC	NEAR			; The column of the cursor
	CallC	T57_02_c
	RET
T57_02	ENDP

T60_00	PROC	NEAR			; DS:SI -> DX BX
	PUSH	AX
	MOV	AX,DS
	CallC	T60_00_c
	MOV	BX,AX
	POP	AX
	RET
T60_00	ENDP

T62_00	PROC	NEAR			; Address of first line of screen
	PUSH	DX
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR T62_00_c
	MOV	SI,AX
	MOV	DS,DX
	POP	AX
	POP	DX
	RET
T62_00	ENDP

T61_00	PROC	NEAR			; DX BX -> DS:SI
	PUSH	DX
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR T61_00_c
	MOV	SI,AX
	MOV	DS,DX
	POP	AX
	POP	DX
	RET
T61_00	ENDP

T63_00	PROC	NEAR			; Last byte ?
	CALL	T60_00
T63_01:	PUSH	AX
	CallC	T63_01_c		; 0 before the end, 1 at it, 2 behind
	CMP	AX,1
	CMC				; CY from the end on, ZR at it
	POP	AX
	RET
T63_00	ENDP

T64_00	PROC	NEAR			; Compare strings
	PUSH	DX
	PUSH	AX
	MOV	DX,DS			; The one at SI is here
	CallC	T64_00_c
	CMP	AX,1			; ZR if they are the same
	POP	AX
	POP	DX
	RET
T64_00	ENDP

T70_00	PROC	NEAR			; Clear line #AL
	PUSH	DX
	PUSH	AX
	CallC	T70_00_c		; AX: the line in ScrBuf, DX: on the screen
	MOV	DI,AX
	MOV	BX,DX
	POP	AX
	POP	DX
	XOR	CX,CX			; As REP left it: T21_00 goes on with that
	RET
T70_00	ENDP

T71_00	PROC	NEAR			; Scroll screen up / down
	CALL	FAR PTR T71_00_c
	RET
T71_00	ENDP

T72_00	PROC	NEAR			; Output line #AL
	PUSH	DX
	PUSH	BX
	PUSH	AX
	MOV	DX,DS
	MOV	BX,BP
	CallC	T72_00_c
	POP	AX
	POP	BX
	POP	DX
	RET
T72_00	ENDP

T73_00	PROC	NEAR			; Output current line
	PUSH	AX
	CallC	T73_00_c
	POP	AX
	RET
T73_00	ENDP

T73_10	PROC	NEAR
	PUSH	DX
	PUSH	AX
	CallC	T73_10_c
	POP	AX
	POP	DX
	RET
T73_10	ENDP

T74_00	PROC	NEAR			; Put file name to status line
	CallC	T74_00_c
	RET
T74_00	ENDP

T75_00	PROC	NEAR			; Output number
	PUSH	AX
	CallC	T75_00_c
	POP	AX
	XOR	CX,CX			; As REP left it: T00_53 goes on with that
	RET
T75_00	ENDP

T76_00	PROC	NEAR
	PUSH	DX
	PUSH	AX
	MOV	DX,DS			; The texts and the colours are here
	MOV	AX,BP
	CallC	T76_00_c
	POP	AX
	POP	DX
	RET
T76_00	ENDP

T80_00	PROC	NEAR			; Save file
	CALL	FAR PTR T80_00_c
	NEG	AX			; CY if it is not saved
	RET
T80_00	ENDP
