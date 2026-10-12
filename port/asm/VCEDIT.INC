;-------------------------------------------------------
;		Version :	16.10.1999
;-------------------------------------------------------

; Boxes, dialogs and texts that the C code of the editor names: its own,
; and those of other files.
	PUBLIC	BigFi_B, NewFi_B, SavAs_B, SavCn_B, Srch_B, SrFor_P, SrEr_B
	PUBLIC	ViFl_S, Menu_F, ViewE_F, EditE_F, Line_S, Col_S, FrBuf_S
	PUBLIC	EdtH_P, EdtH_S, EdtH1_S, EdtH2_S

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
; The editor is C: T00_00_c of port/edit.c, with all that a key does. The
; viewer's code jumps here when it has opened the file to edit, with the
; handle in BX, and in AX, DX and SI where its box is. The answer says
; where that code goes on.
T00_00:	PUSH	BX
	CALL	FAR PTR T00_00_c
	POP	BX
	PUSH	DS
	POP	ES
	CMP	AX,1
	JE	T00_01
	JA	T00_02
	JMP	V00_10			; View the file instead
T00_01:	JMP	V13_00			; Not read
T00_02:	MOV	CL,DS:EdWrite		; Edited
	JMP	V13_01

T70_00	PROC	NEAR			; Clear line #AL
	PUSH	DX
	PUSH	AX
	CallC	T70_00_c		; AX: the line in ScrBuf, DX: on the screen
	MOV	DI,AX
	MOV	BX,DX
	POP	AX
	POP	DX
	XOR	CX,CX			; As REP left it
	RET
T70_00	ENDP

T71_00	PROC	NEAR			; Scroll screen up / down
	CALL	FAR PTR T71_00_c
	RET
T71_00	ENDP
