;-------------------------------------------------------
;		Version :	14.06.2000
;-------------------------------------------------------

; Boxes, dialogs and texts that C code hands to DialBox and Dialog.
	PUBLIC	ViFl_S, Col_S, RdFil_P, FndEr_B, Srch_B, SrFor_P, SrEr_B

HexLen	EQU	WORD PTR HelpBuf[190]	; Count of hex pattern

ViewDat	STRUC
	 DB	TmpBuf+200 DUP(?)
ViSpace	 DW	?			; Edit buffer size in paragraphs
ViewEdit DB	?			; View / Edit
ViStatus DB	?			; Output status line
ViLoaded DW	?			; Bytes loaded in buffer
ViLen	 DD	?			; Size of file
ViOffset DD	?			; Offset of first loaded byte in buffer
ViKeyBar DW	?			; Saved KeyBar & Clock
ViTxHex	 DB	?			; Text / Hex mode
ViWrap	 DB	?			; Wrap / Unwrap
ViFirst	 DW	?			; Offset of first line of screen
ViHandle DW	?			; File handle
ViCol	 DW	?			; First column displayed on the screen
ViPoint	 DW	?			; Point to address of first line
ViEndTab DW	?			; Point to end of line table
ViBase	 DW	?			; View window base
ViSize	 DW	?			; View window size
ViSrPan	 DB	?			; String was found in Hex/ASCII panel
ViSrLen	 DB	?			; Length of the found string
ViTmp1	 DW	?			; First byte of search string
ViTmp2	 DW	?			; Last byte of search string
ViSearch DD	?			; Saerch offset
Vi100pc	 DB	?			; 100% was displayed
ViewDat	ENDS

	.CONST
ViewE_F	DB	"VCVIEW.EXT",0
EditE_F	DB	"VCEDIT.EXT",0
Col_S	DB	"Col",0
ViFl_S	DB	" View ",0
ViFl1_S	DB	"Loading the viewer:",0
EdFl_S	DB	" Edit ",0
EdFl1_S	DB	"Loading the editor:",0
ViFi0_S	DB	" ^I",0
ViFi1_S	DB	"^Ithe file:",0FFh,LenStr-14,0
RdFil_S	DB	"Reading the file",0
Srch_S	DB	"Search for the string",0FFh,LenStr-21,0
SrFor_S	DB	"Searching for",0
SrEr_S	DB	"Could not find the string",0
RunEr_S	DB	"Not enough memory to load",0
Run1_S	DB	"Can't run commands for some reason.",0
Run2_S	DB	"Try rebooting.",0

	.DATA
ViewPar	DB	0,'Socha',0
	DW	11Fh
RunEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	TmpBuf
	DB	0, 50
	DW	OFFSET Err_S
	DW	OFFSET Inc_S
	DW	OFFSET Var_S
	DW	OFFSET Ok_S
RunEr_B	BOXHD	< RunEr_P, 928h, 0, 2, 0FFh, 0FFh, 2 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpBuf, OkMn_D >
	DB	0
ViFil_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	ViFi0_S
	DW	ViFi1_S
	DW	Empty_S
ViFil_B	DW	ViFil_P, 528h, 0
	DB	1, 2, 0, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, ViewFil, 200h+LenStr
	DB	0
Viewr_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf
	DB	7, 64
	DW	ViFl_S
	DW	ViFl1_S
	DW	Var_S
Editr_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf
	DB	7, 64
	DW	EdFl_S
	DW	EdFl1_S
	DW	Var_S
RdFil_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	ViewFil
	DB	7, 64
	DW	Inc_S
	DW	RdFil_S
	DW	Var_S
Srch_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	Inc_S
	DW	Srch_S
	DW	Empty_S
Srch_B	DW	Srch_P, 528h, 0
	DB	1, 0FFh, 0, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, SrchStr, LenStr
	DB	0
SrFor_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	SrchStr
	DB	0, LenStr
	DW	Inc_S
	DW	SrFor_S
	DW	Kov_S
FndEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	ViewFil
	DB	7, 64
	DW	Inc_S
	DW	NoFnd_S
	DW	Var_S
	DW	Ok_S
FndEr_B	DW	FndEr_P, 928h, 0
	DB	1, 0FFh, 0, 2
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf, OkMn_D
	DB	0
SrEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	WinBuf+2000
	DW	SrchStr
	DB	0, LenStr
	DW	Inc_S
	DW	SrEr_S
	DW	Kov_S
	DW	Ok_S
SrEr_B	DW	SrEr_P, 928h, 0
	DB	1, 0FFh, 0, 0
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf, OkMn_D
	DB	0

	.CODE
F43_00:	XOR	AX,AX			; View...
	JMP	F44_01

F44_00:	MOV	AX,1			; Edit...
F44_01:	PUSH	ES
	POP	DS
	MOV	DX,OFFSET ViFl_S+1
	CMP	AL,0
	JZ	F44_02
	MOV	DX,OFFSET EdFl_S+1
F44_02:	PUSH	AX
	ADD	AL,18
	MOV	DS:HlpPage,AL
	MOV	BP,OFFSET ViFil_B
	CALL	Dialog
	POP	AX
	JNE	F44_03
	CMP	DS:ViewFil,0
	JE	F44_03
	JMP	F34_11
F44_03:	JMP	Func04

F63_00:	MOV	AX,100h			; Internal viewer
	JMP	F34_01

F64_00:	MOV	AX,101h			; Internal editor
	JMP	F34_01

F33_00:	XOR	AX,AX			; View
	JMP	F34_01

F34_00:	MOV	AX,1			; Edit
F34_01:	CMP	DS:Visible,0
	JZ	F44_01
	CMP	DS:WinTyp,0
	JNE	F34_02
	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F44_03
	TEST	DS:[SI].AttrF,10h
	JNZ	F44_03
	MOV	DI,OFFSET ViewFil
	MOV	CX,13
	REP	MOVSB
	JMP	F34_11
F34_02:	CMP	AL,0
	JZ	F44_01
	CMP	DS:WinTyp,2
	JNE	F44_01
F34_10:	CALL	BegBX			; Edit 'dirinfo'
	CALL	SizDX
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET DrIn_F
	MOV	DI,OFFSET ViewFil
	MOV	CX,13
	PUSH	DI
	REP	MOVSB
	POP	SI
	CALL	LetLin
	PUSH	ES
	POP	DS
	ADD	BX,0B02h
	SUB	DX,0C04h
	MOV	DS:ViBase,BX
	MOV	DS:ViSize,DX
	DEC	BH
	ADD	BL,15
	MOV	DX,106h
	CALL	BasAdr
	MOV	SI,OFFSET EdFl_S
	MOV	CX,6
	MOV	AH,Slct_C
	CALL	Color
	CALL	MvsBwc
	MOV	DL,1
	MOV	AL,2
	JMP	V00_02
F34_11:	MOV	BX,OFFSET AltView
	CMP	AL,0
	JE	F34_12
	MOV	BX,OFFSET AltEdit
F34_12:	CMP	BYTE PTR ES:[BX],0
	JNE	F34_13
	OR	AH,AH
	MOV	AH,0
	JNE	F34_13
	MOV	AH,1
F34_13:	MOV	BL,AL
	OR	AH,AH
	JNZ	F34_15
	CALL	HidCur			; Load extension file
	MOV	AL,2
	CALL	PutMnu
	MOV	BP,OFFSET ViewE_F
	OR	BL,BL
	JZ	F34_14
	MOV	BP,OFFSET EditE_F
F34_14:	MOV	DX,OFFSET TmpBuf
	PUSH	CS
	POP	DS
	MOV	SI,BP
	MOV	DI,DX
	MOV	CX,13
	REP	MOVSB
	PUSH	ES
	POP	DS
	CALL	ChgDisk
	JC	F34_17
	MOV	AL,40h
	CALL	OpenFil
	JNC	F34_18
	CMP	AL,2
	JNE	F34_17
	MOV	SI,BP
	MOV	DI,DX
	CALL	MainFil
	CALL	ChgDisk
	JNC	F34_16
F34_15:	MOV	DL,BL
	JMP	V00_00
F34_16:	MOV	AL,40h
	CALL	OpenFil
	JNC	F34_18
	CMP	AL,3
	JBE	F34_15
F34_17:	JMP	Func04
F34_18:	MOV	DI,BX
	MOV	BX,AX
	MOV	DX,OFFSET MenuBuf
	MOV	CX,LenMenu-1
	MOV	AH,3Fh
	CALL	Intr21
	PUSHF
	PUSH	AX
	MOV	AH,3Eh
	CALL	Intr21
	POP	BX
	POP	CX
	JC	F34_17
	PUSH	CX
	POPF
	JC	F34_17
	MOV	BYTE PTR DS:MenuBuf[BX],1Ah
	MOV	BX,DI
	MOV	SI,OFFSET ViewFil	; Test extension file
	CALL	CutPath
	CALL	FindExt
	OR	SI,SI
	JE	F34_15
	CALL	F34_30
	JC	F34_15
	MOV	BP,1
	PUSH	BX
	CALL	F34_40
	POP	BX
	JC	F34_20
	MOV	AL,' '
	STOSB
	JMP	Run_00
F34_20:	CALL	ChgDisk			; Standard viewer
	JC	F34_21
	MOV	CX,27h
	MOV	AH,4Eh
	CALL	Intr21
	JNC	F34_22
	CMP	AX,WORD PTR 53h
	JE	F34_21
	JMP	F34_15
F34_21:	JMP	Func04
F34_22:	MOV	SI,OFFSET ViewFil
	MOV	DI,OFFSET TmpBuf[100+1]
	MOV	CX,LenStr+1
	PUSH	DI
	CALL	MvzLine
	POP	SI
	MOV	AX,DI
	SUB	AX,SI
	ADD	AL,45
	MOV	DS:TmpBuf[100],AL
	MOV	AL,0
	MOV	BP,OFFSET Viewr_P
	CMP	BL,0
	JE	Aexec0
	MOV	BP,OFFSET Editr_P
Aexec0:	PUSH	AX
	MOV	BX,528h
	CALL	DialBox
	CALL	Window
	MOV	WORD PTR DS:[TmpBuf+95],BX
	MOV	WORD PTR DS:[TmpBuf+97],DX
	MOV	BH,0
	MOV	AH,3
	INT	10h
	MOV	WORD PTR DS:[TmpBuf+93],CX
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET ViewPar
	MOV	CX,9
	PUSH	DI
	REP	MOVSB
	MOV	CX,36
	MOV	AL,0
	REP	STOSB
	MOV	AL,0Dh
	STOSB
	PUSH	ES
	POP	DS
	POP	DI
	POP	AX
	MOV	DS:[DI+15],AL
	MOV	AL,DS:ColTyp
	MOV	DS:[DI+9],AL
	CMP	DS:Mouse,0
	JE	Aexec1
	MOV	BYTE PTR DS:[DI+11],0FFh
	MOV	AL,DS:LftMous
	MOV	DS:[DI+12],AL
	MOV	BYTE PTR DS:[DI+13],1
Aexec1:	MOV	BX,DS:ViewSeg
	MOV	AX,ES
	SUB	BX,AX
	ADD	BX,((MinView-1) SHR 4)+1
	MOV	AH,4Ah
	INT	21h
	MOV	BX,OFFSET EPB
	MOV	DS:[BX].CmdLine.Offs,OFFSET TmpBuf+100
	MOV	DS:[BX].CmdLine.Segm,ES
	MOV	DX,OFFSET TmpBuf
	MOV	SI,DX
	CALL	LetLin
	MOV	AX,4B00h
	CALL	Intr21
	PUSHF
	PUSH	AX
	MOV	BX,0FFFFh
	MOV	AH,4Ah
	INT	21h
	MOV	AH,4Ah
	INT	21h
	CALL	HidCur
	PUSH	ES
	POP	DS
	CALL	SavMous
	CALL	IniMous
	POP	AX
	POPF
	JNC	Aexec3
	CMP	AX,WORD PTR 53h
	JE	Aexec3
	MOV	DX,OFFSET RunEr_S
	CMP	AX,WORD PTR 8
	JE	Aexec2
	CMP	AX,WORD PTR 7
	JE	Aexec2
	MOV	DX,OFFSET Run1_S
	MOV	SI,OFFSET Run2_S
	PUSH	CS
	POP	DS
	MOV	DI,OFFSET TmpBuf
	MOV	CX,80
	REP	MOVSB
	PUSH	ES
	POP	DS
Aexec2:	MOV	BP,OFFSET RunEr_B
	CALL	Dialog
Aexec3:	MOV	BX,WORD PTR DS:[TmpBuf+95]
	MOV	DX,WORD PTR DS:[TmpBuf+97]
	MOV	SI,OFFSET WinBuf
	CALL	ResWin
	XOR	BX,BX
	MOV	DL,80
	MOV	DH,DS:Lines
	CALL	Window
	MOV	CX,WORD PTR DS:[TmpBuf+93]
	MOV	AH,1
	INT	10h
	JMP	FncEx1

F34_30	PROC	NEAR
	CALL	FndLet
	CALL	IsCrLfS
	JE	F34_31
	DEC	SI
	CMP	AL,1Ah
	JE	F34_33
	DEC	SI
	CLC
	RET
F34_31:	MOV	AX,DS:[SI]
	CMP	AL,' '
	JE	F34_30
	CMP	AL,9
	JE	F34_30
	CALL	IsCrLf
	JE	F34_32
	CMP	AL,"'"
	JNE	F34_33
F34_32:	CALL	IsCrLfS
	JE	F34_31
	DEC	SI
	CMP	AL,1Ah
	JNE	F34_32
F34_33:	STC
	RET
F34_30	ENDP

F34_40	PROC	NEAR
	MOV	DI,SI
	MOV	AX,DS:[SI]
	CMP	AH,':'
	JNE	F34_41
	CALL	UpCase
	CMP	AL,'A'
	JB	F34_45
	CMP	AL,'Z'
	JA	F34_45
	INC	SI
	INC	SI
F34_41:	LODSW
	DEC	SI
	CMP	AL,'!'
	JE	F34_45
	CALL	IsCrLf
	JE	F34_42
	CMP	AL,1Ah
	JE	F34_42
	CMP	AL,' '
	JE	F34_42
	CMP	AL,9
	JNE	F34_41
F34_42:	DEC	SI
	PUSH	SI
	SUB	SI,4
	MOV	BX,OFFSET Menu_F+1
	MOV	CX,1
	OR	BP,BP
	JNZ	F34_43
	MOV	CX,4
F34_43:	LODSB
	CALL	UpCase
	INC	BX
	CMP	AL,CS:[BX]
	LOOPE	F34_43
	POP	SI
	JNE	F34_45
	PUSH	SI
	CALL	F34_30
	POP	SI
	JNC	F34_45
	MOV	BYTE PTR DS:[SI],0
	SUB	SI,DI
	CMP	SI,LenPath+12
	JAE	F34_45
	MOV	SI,DI
	CALL	CutPath
	CMP	SI,DI
	MOV	DX,OFFSET TmpBuf
	MOV	SI,DX
	JNE	F34_44
	CALL	CutPath
F34_44:	XCHG	SI,DI
	MOV	CX,LenPath+13
	REP	MOVSB
	STC
	RET
F34_45:	DEC	DI
	MOV	SI,DI
	CLC
	RET
F34_40	ENDP

; Opening a file for the viewer or the editor, the viewer and the way back
; to the panels are C: V00_02_c, V00_10_c, V13_00_c and V13_01_c of
; port/view.c. They are jumped to, with ES on the main data.
;	View or edit the file ViewFil
;			   AL - the item of the menu bar to mark
;			   DL - 0 to view, 1 to edit
V00_00:	MOV	AL,2
V00_01:	MOV	BX,1			; In all of the screen
	JMP	V00_03
V00_02:	XOR	BX,BX			; In the window that ViBase and ViSize say
V00_03:	PUSH	ES
	POP	DS
	SUB	SP,8			; The file, and where the box is
	MOV	DI,SP
	CALL	FAR PTR V00_02_c
	MOV	CX,AX			; 0 - nothing, 1 - the editor, 2 - the viewer
	POP	BX
	POP	AX
	POP	DX
	POP	SI
	JCXZ	V00_04
	DEC	CX
	JNZ	V00_10
	JMP	T00_00
V00_04:	JMP	Func04
;	The viewer, also for a file that is too large for the editor
;			   BX - handle
V00_10:	CallC	V00_10_c
	JMP	Init50
;	Close the file and go back to the panels
V13_00:	CallC	V13_00_c
	JMP	Init50
;	Go back to the panels
;			   CL - not 0 if a file was written
V13_01:	MOV	AX,CX
	CallC	V13_01_c
	JMP	Init50
