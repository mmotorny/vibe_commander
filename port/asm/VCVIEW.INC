;-------------------------------------------------------
;		Version :	14.06.2000
;-------------------------------------------------------

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
Byts_S	DB	" Bytes"
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
View_T	DW	001Bh, V13_00
	DW	3D00h, V13_00
	DW	4400h, V13_00
	DW	4B00h, V01_00
	DW	0013h, V01_00
	DW	4D00h, V03_00
	DW	0004h, V03_00
	DW	7300h, V02_00
	DW	0001h, V02_00
	DW	7400h, V04_00
	DW	0006h, V04_00
	DW	4800h, V05_00
	DW	0005h, V05_00
	DW	5000h, V06_00
	DW	0018h, V06_00
	DW	4900h, V07_00
	DW	0012h, V07_00
	DW	5100h, V08_00
	DW	0003h, V08_00
	DW	4700h, V09_00
	DW	7700h, V09_00
	DW	8400h, V09_00
	DW	4F00h, V10_00
	DW	7500h, V10_00
	DW	7600h, V10_00
	DW	3E00h, V11_00
	DW	000Dh, V11_00
	DW	3C00h, V12_00
	DW	4100h, V14_00
	DW	5A00h, V15_00
	DW	0
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

V00_00:	MOV	AL,2
V00_01:	MOV	ES:ViBase,100h
	MOV	BL,80
	MOV	BH,ES:Lines
	SUB	BH,2
	MOV	ES:ViSize,BX
V00_02:	MOV	ES:WinMous,0
	CALL	PutMnu
	PUSH	ES
	POP	DS
	MOV	DS:ViewEdit,DL
	OR	DL,DL
	MOV	DX,OFFSET ViFl_S
	JZ	V00_03
	MOV	DX,OFFSET EdFl_S
V00_03:	MOV	BX,528h
	MOV	BP,OFFSET RdFil_P
	CALL	DialBox
	CALL	Window
	MOV	CX,DX
	MOV	DX,OFFSET ViewFil
	CALL	ChgDisk
	JC	V00_05
	MOV	AL,40h
	CALL	OpenFil
	JNC	V00_06
	CMP	AX,WORD PTR 53h
	JE	V00_05
	MOV	DX,OFFSET ViFl_S
	CMP	DS:ViewEdit,0
	JZ	V00_04
	CMP	AX,WORD PTR 2
	MOV	AX,0FFFFh
	JE	V00_06
	MOV	DX,OFFSET EdFl_S
V00_04:	MOV	BP,OFFSET FndEr_B
	CALL	Dialog
V00_05:	MOV	DX,CX
	CALL	ResWin
	CALL	Window
	JMP	Func04
V00_06:	CALL	TstBrk
	JC	V00_05
	PUSH	SI
	PUSH	CX
	PUSH	BX
	MOV	DS:ViHandle,AX
	MOV	BX,AX
	CMP	BX,0FFFFh
	JE	V00_07
	XOR	CX,CX
	XOR	DX,DX
	MOV	AX,4202h
	CALL	Intr21
	JNC	V00_08
V00_07:	XOR	DX,DX
	XOR	AX,AX
V00_08:	MOV	DS:ViLen.Lo,AX
	MOV	DS:ViLen.Hi,DX
	XOR	CX,CX
	XOR	DX,DX
	MOV	AX,4200h
	CALL	Intr21
	MOV	AL,DS:Clock
	MOV	AH,DS:KeyBar
	MOV	DS:ViKeyBar,AX
	POP	AX
	POP	DX
	POP	SI
	CMP	DS:ViewEdit,0
	JZ	V00_10
	JMP	T00_00
V00_10:	MOV	AX,ES
	ADD	AX,DS:FreMem2
	SUB	AX,DS:ViewSeg
	CMP	AX,0FFEh
	JBE	V00_11
	MOV	AX,0FFEh
V00_11:	MOV	CL,4
	SHL	AX,CL
	MOV	DS:ViSpace,AX
	MOV	CX,AX
	MOV	DS,DS:ViewSeg
	XOR	DX,DX
	CALL	V55_00
	PUSH	DS
	PUSH	ES
	POP	DS
	XOR	AX,AX
	CMP	AX,DS:ViLen.Hi
	JNE	V00_12
	MOV	CX,DS:ViLen.Lo
V00_12:	MOV	DS:ViLoaded,CX
	MOV	DS:ViOffset.Lo,AX
	MOV	DS:ViOffset.Hi,AX
	MOV	DS:ViFirst,AX
	MOV	DS:ViCol,AX
	MOV	DS:ViTxHex,AL
	MOV	DS:Clock,AL
	MOV	DS:KeyBar,1
	MOV	DS:ViWrap,1
	MOV	DS:ViSearch.Lo,0FFF0h
	MOV	DS:ViSearch.Hi,0FFFFh
	MOV	DS:HlpPage,3
	CMP	DS:ViSize.X,79
	JNB	$+5			; JB V00_14
	JMP	NEAR PTR V00_14
	MOV	DI,OFFSET ScrBuf[0]	; Status line
	MOV	AH,Menu_C
	CALL	Color
	MOV	CX,80
	MOV	AL,' '
	REP	STOSW
	MOV	SI,OFFSET ViFl_S+1
	MOV	DI,OFFSET ScrBuf[0]
	MOV	CX,4
	CALL	MvsBwc
	MOV	AL,':'
	STOSW
	MOV	SI,OFFSET Col_S
	MOV	DI,OFFSET ScrBuf[2*40]
	MOV	CX,3
	CALL	MvsBwc
	PUSH	AX
	MOV	SI,OFFSET ViewFil
	MOV	DI,OFFSET TmpBuf
	MOV	CX,LenPath+13
	PUSH	DI
	REP	MOVSB
	POP	SI
	CALL	LetLin
	MOV	AX,33
	CALL	ShrtLin
	MOV	DI,OFFSET ScrBuf[2*6]
	CALL	MovDz0
	MOV	DI,OFFSET TmpBuf
	MOV	SI,DI
	MOV	CX,DS:ViLen.Lo
	MOV	BX,DS:ViLen.Hi
	MOV	AL,3
	CALL	Number
	MOV	DI,OFFSET ScrBuf[2*51]
	CALL	MovDz0
	POP	AX
	MOV	SI,OFFSET Byts_S
	MOV	CX,6
	CMP	DS:ViLen.Lo,1
	JNE	V00_13
	CMP	DS:ViLen.Hi,0
	JNE	V00_13
	DEC	CX
V00_13:	CALL	MvsBwc
	MOV	BYTE PTR DS:ScrBuf[2*78],'%'
	MOV	AL,1
V00_14:	MOV	DS:ViStatus,AL
	POP	DS
V00_15:	XOR	SI,SI
	MOV	DI,ES:ViSpace
	CALL	V60_00

V00_20:	CALL	V25_00			; Output screen
	CALL	V30_00
	JNC	V00_30
V00_21:	CALL	V40_00
	JNC	V00_30
	MOV	AX,ES:ViSpace
	SHR	AX,1
	CMP	ES:ViFirst,AX
	JB	V00_30
	CALL	V50_00			; Continue read
V00_22:	CALL	V30_00

V00_30:	CALL	GetMous			; Mouse
	JNZ	V00_31
	MOV	ES:WinMous,0
	JMP	V00_40
V00_31:	CALL	ClrKbd
	MOV	BX,ES:ViBase
	MOV	DX,ES:ViSize
	CMP	ES:WinMous,0
	JNZ	V00_32
	CALL	InWind
	JC	V00_30
V00_32:	MOV	ES:WinMous,1
	PUSH	BX
	MOV	AL,DH
	CBW
	MOV	BL,3
	DIV	BL
	ADD	BH,AL
	CMP	CH,BH
	POP	BX
	JAE	V00_33
	JMP	V05_00
V00_33:	ADD	BH,DH
	SUB	BH,AL
	CMP	CH,BH
	JB	V00_34
	JMP	V06_00
V00_34:	MOV	AL,DL
	SHR	AL,1
	SHR	AL,1
	ADD	BL,AL
	CMP	CL,BL
	JAE	V00_35
	JMP	V01_00
V00_35:	SUB	BL,AL
	ADD	BL,DL
	SUB	BL,AL
	CMP	CL,BL
	JB	V00_40
	JMP	V03_00

V00_40:	MOV	AL,3			; Input
	CMP	ES:ViTxHex,0
	JNZ	V00_41
	INC	AX
	CMP	ES:ViWrap,0
	JZ	V00_41
	INC	AX
V00_41:	CALL	Input
	CALL	ClrKbd
	CMP	AH,0FCh
	JE	V00_42
	MOV	SI,OFFSET View_T
	CALL	Case
V00_42:	JMP	V00_30

V01_00:	MOV	BX,1			; Left
	JMP	V02_01

V02_00:	MOV	BX,40			; ^Left
V02_01:	CMP	ES:ViTxHex,0
	JNZ	V00_42
	MOV	AX,ES:ViCol
	OR	AX,AX
	JE	V00_42
	SUB	AX,BX
	JAE	V02_02
	XOR	AX,AX
V02_02:	CMP	AX,ES:ViCol
	JE	V00_42
	MOV	ES:ViCol,AX
	JMP	V00_20

V03_00:	MOV	BX,1			; Right
	JMP	V04_01

V04_00:	MOV	BX,40			; ^Right
V04_01:	CMP	ES:ViTxHex,0
	JNZ	V00_42
	MOV	AX,ES:ViCol
	MOV	CX,0FFFFh-80
	CMP	AX,CX
	JAE	V00_42
	ADD	AX,BX
	CMP	AX,CX
	JB	V02_02
	MOV	AX,CX
	JMP	V02_02

V05_00:	CALL	V20_00			; Up
	MOV	AX,SI
	OR	AX,ES:ViOffset.Lo
	OR	AX,ES:ViOffset.Hi
	JE	V00_42
	CALL	PrevLin
	JC	V05_01
	OR	SI,SI
	JNE	V05_02
	MOV	AX,ES:ViSpace
	SHR	AX,1
	CMP	AX,ES:ViFirst
	JBE	V05_02
V05_01:	MOV	AX,ES:ViOffset.Lo
	OR	AX,ES:ViOffset.Hi
	JE	V05_02
	CALL	V51_00
	CALL	V20_00
	CALL	PrevLin
	MOV	ES:ViFirst,SI
	MOV	ES:ViPoint,BX
	CALL	V25_00
	JMP	V00_22
V05_02:	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	DS:ViFirst,SI
	MOV	DS:ViPoint,BX
	CALL	V25_00
	MOV	BX,DS:ViBase
	PUSH	BX
	ADD	BH,DS:ViSize.Y
	DEC	BH
	MOV	DX,-2*80
	MOV	AX,700h
	CALL	T71_00
	POP	BX
	CALL	V31_00
	POP	DS
	CALL	V20_00
	MOV	BP,SI
	CALL	NextLin
	XCHG	BP,SI
	JNC	V05_03
	MOV	BP,ES:ViLoaded
V05_03:	MOV	AL,0
	CALL	V32_00
	PUSH	BX
	MOV	BX,ES:EdBase
	MOV	DL,ES:EdSize.X
	MOV	DH,1
	CALL	Window
	POP	BX
	MOV	CL,ES:ViSize.Y
	MOV	CH,0
V05_04:	CALL	NextLin
	JC	V06_05
	LOOP	V05_04
V05_05:	JMP	V00_30

V06_00:	CMP	ES:Vi100pc,0		; Down
	JNZ	V05_05
	CALL	V20_00
	CALL	NextLin
	JNC	V06_01
	CALL	V50_00
	XOR	SI,SI
	MOV	BX,ES:ViSpace
	DEC	BX
	DEC	BX
	STC
V06_01:	MOV	ES:ViFirst,SI
	MOV	ES:ViPoint,BX
	JNC	V06_02
	CALL	V25_00
	JMP	V00_22
V06_02:	CALL	V25_00
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	BX,DS:ViBase
	PUSH	BX
	MOV	DX,2*80
	MOV	AX,600h
	CALL	T71_00
	POP	BX
	ADD	BH,DS:ViSize.Y
	DEC	BH
	CALL	V31_00
	MOV	AL,DS:ViSize.Y
	CBW
	DEC	AX
	POP	DS
	MOV	CX,AX
	CALL	V20_00
V06_03:	CALL	NextLin
	JC	V06_05
	LOOP	V06_03
	MOV	BP,SI
	CALL	NextLin
	XCHG	BP,SI
	JNC	V06_04
	MOV	BP,ES:ViLoaded
V06_04:	PUSHF
	CALL	V32_00
	MOV	BX,ES:ViBase
	ADD	BH,AL
	MOV	DL,ES:ViSize.X
	MOV	DH,1
	CALL	Window
	POPF
	JNC	V06_06
V06_05:	CALL	V40_00
	JMP	V00_21
V06_06:	JMP	V00_30

V07_00:	CALL	V20_00			; PgUp
	MOV	AX,SI
	OR	AX,ES:ViOffset.Lo
	OR	AX,ES:ViOffset.Hi
	JE	V06_06
	MOV	CL,ES:ViSize.Y
	MOV	CH,0
	DEC	CX
	DEC	CX
V07_01:	CALL	PrevLin
	JC	V07_02
	LOOP	V07_01
	OR	SI,SI
	JNE	V07_04
V07_02:	MOV	AX,ES:ViSpace
	SHR	AX,1
	CMP	AX,ES:ViFirst
	JBE	V07_04
	MOV	AX,ES:ViOffset.Lo
	OR	AX,ES:ViOffset.Hi
	JE	V07_04
	CALL	V51_00
	CALL	V20_00
	MOV	CL,ES:Lines
	SUB	CL,4
	MOV	CH,0
V07_03:	CALL	PrevLin
	JC	V07_04
	LOOP	V07_03
V07_04:	MOV	ES:ViFirst,SI
	MOV	ES:ViPoint,BX
	CALL	V25_00
	JMP	V00_22

V08_00:	CMP	ES:Vi100pc,0		; PgDn
	JNZ	V06_06
	CALL	V20_00
	MOV	DI,OFFSET TmpBuf-2
	MOV	CL,ES:ViSize.Y
	MOV	CH,0
	DEC	CX
	DEC	CX
V08_01:	CALL	NextLin
	JC	V08_06
	INC	DI
	INC	DI
	MOV	ES:[DI],SI
	LOOP	V08_01
	MOV	CL,ES:ViSize.Y
	DEC	CX
V08_02:	CALL	NextLin
	JC	V08_03
	LOOP	V08_02
	JMP	V08_05
V08_03:	MOV	AX,ES:ViOffset.Lo
	MOV	BX,ES:ViOffset.Hi
	ADD	AX,ES:ViLoaded
	ADC	BX,0
	CMP	BX,ES:ViLen.Hi
	JB	V08_05
	JA	V08_04
	CMP	AX,ES:ViLen.Lo
	JB	V08_05
V08_04:	ADD	CX,CX
	SUB	DI,CX
	JC	V09_01
	CMP	DI,OFFSET TmpBuf
	JB	V09_01
V08_05:	MOV	SI,ES:[DI]
	MOV	ES:ViFirst,SI
	JMP	V00_20
V08_06:	CALL	V50_00
	MOV	ES:ViFirst,0
	MOV	BX,ES:ViSpace
	DEC	BX
	DEC	BX
	MOV	ES:ViPoint,BX
	JMP	V00_20

V09_00:	MOV	AX,ES:ViOffset.Lo	; Home
	OR	AX,ES:ViOffset.Hi
	JNZ	V09_02
	MOV	AX,ES:ViFirst
	OR	AX,ES:ViCol
	JE	V09_01
	MOV	ES:ViFirst,0
	MOV	ES:ViCol,0
	JMP	V00_20
V09_01:	JMP	V00_30
V09_02:	MOV	ES:ViOffset.Lo,0
	MOV	ES:ViOffset.Hi,0
	MOV	ES:ViFirst,0
	MOV	ES:ViCol,0
	MOV	CX,ES:ViSpace
	CMP	ES:ViLen.Hi,0
	JNE	V09_03
	MOV	AX,ES:ViLen.Lo
	CMP	CX,AX
	JBE	V09_03
	MOV	CX,AX
V09_03:	MOV	ES:ViLoaded,CX
	MOV	BX,ES:ViHandle
	PUSH	CX
	XOR	DX,DX
	XOR	CX,CX
	MOV	AX,4200h
	CALL	Intr21
	POP	CX
	XOR	DX,DX
	CALL	V55_00
	JMP	V00_15

V10_00:	CMP	ES:Vi100pc,0		; End
	JNZ	V09_01
	MOV	AX,ES:ViLen.Lo
	MOV	BX,ES:ViLen.Hi
	SUB	AX,ES:ViOffset.Lo
	SBB	BX,ES:ViOffset.Hi
	OR	BX,BX
	JNE	V10_01
	CMP	AX,ES:ViLoaded
	JBE	V10_03
V10_01:	MOV	AX,ES:ViLen.Lo
	MOV	BX,ES:ViLen.Hi
	MOV	DX,AX
	AND	DX,0Fh
	MOV	CX,ES:ViSpace
	SHR	CX,1
	SUB	CX,10h
	ADD	CX,DX
	SUB	AX,CX
	SBB	BX,0
	MOV	ES:ViOffset.Lo,AX
	MOV	ES:ViOffset.Hi,BX
	MOV	ES:ViLoaded,CX
	MOV	ES:ViFirst,0
	MOV	BX,ES:ViHandle
	MOV	SI,ES:ViOffset.Lo
	MOV	DI,ES:ViOffset.Hi
	MOV	DX,SI
	MOV	CX,DI
	MOV	AX,4200h
	CALL	Intr21
	CMP	AX,SI
	JNE	V10_02
	CMP	DX,DI
	JNE	V10_02
	XOR	DX,DX
	MOV	CX,ES:ViLoaded
	CALL	V55_00
V10_02:	XOR	SI,SI
	MOV	DI,ES:ViSpace
	CALL	V60_00
V10_03:	MOV	BX,ES:ViEndTab
	MOV	SI,DS:[BX]
	MOV	CL,ES:ViSize.Y
	MOV	CH,0
	DEC	CX
	CMP	ES:ViTxHex,0
	JE	V10_04
	MOV	SI,ES:ViLoaded
	DEC	SI
V10_04:	CALL	PrevLin
	JC	V10_05
	LOOP	V10_04
V10_05:	MOV	ES:ViFirst,SI
	MOV	ES:ViPoint,BX
	JMP	V00_20

V11_00:	MOV	BX,OFFSET ViTxHex	; F4 - ASCII / Hex
	CALL	InvVar
	JMP	V00_20

V12_00:	CMP	ES:ViTxHex,0		; F2 - Wrap / Unwrap
	JNZ	V12_01
	MOV	BX,OFFSET ViWrap
	CALL	InvVar
	JMP	V00_20
V12_01:	JMP	V00_30

V13_00:	MOV	BX,ES:ViHandle		; Esc
	MOV	AH,3Eh
	CALL	Intr21
	MOV	CL,0
V13_01:	CALL	Quit
	MOV	AX,ES:ViKeyBar
	MOV	ES:Clock,AL
	MOV	ES:KeyBar,AH
	MOV	DS,ES:WCB1seg
	CMP	DS:Visible,0
	JZ	V13_02
	CALL	Panel
	CALL	PutPath
	CALL	PutFls
V13_02:	OR	DS:ChgFlg,CL
	MOV	DS,ES:WCB2seg
	CMP	DS:Visible,0
	JZ	V13_03
	CALL	Panel
	CALL	PutPath
	CALL	PutFls
V13_03:	OR	DS:ChgFlg,CL
	CALL	PutPrm
	MOV	AL,0
	CALL	PutMnu
	MOV	AL,0FFh
	CALL	PutKey
	XOR	BX,BX
	MOV	DH,ES:Lines
	MOV	DL,80
	CALL	Window
	MOV	ES:WinMous,0
	JMP	Init50

V14_00:	PUSH	DS			; F7 - search
	PUSH	ES
	POP	DS
	MOV	DX,OFFSET ViFl_S
	MOV	BP,OFFSET Srch_B
	CALL	Dialog
	POP	DS
	JE	V15_00
V14_01:	JMP	V00_30

V15_00:	CMP	ES:SrchStr,0		; Continue search
	JE	V14_01
	MOV	DX,OFFSET ViFl_S
	CALL	V80_00
	PUSH	SI
	PUSH	DX
	PUSH	BX
	MOV	SI,ES:ViFirst
	CMP	ES:ViSearch.Hi,0FFFFh
	JE	V15_01
	MOV	BX,ES:ViPoint
	CALL	NextLin
	JNC	V15_01
	MOV	SI,ES:ViLoaded
V15_01:	MOV	AX,ES:ViOffset.Lo
	MOV	BX,ES:ViOffset.Hi
	ADD	AX,SI
	ADC	BX,0
	MOV	ES:ViSearch.Lo,AX
	MOV	ES:ViSearch.Hi,BX
	MOV	CX,ES:ViLoaded
	SUB	CX,SI
	JB	V15_04
	MOV	DX,SI
	CALL	V82_00
	JNC	V15_02
	JMP	V15_06
V15_02:	MOV	DI,SI
	SUB	DI,DX
	MOV	AH,0
	JMP	V15_10
V15_03:	JMP	V15_22
V15_04:	CALL	TstBrk
	MOV	AL,2
	JC	V15_03
	MOV	AX,ES:ViSearch.Lo
	MOV	BX,ES:ViSearch.Hi
	MOV	CX,ES:ViLen.Lo
	MOV	DX,ES:ViLen.Hi
	SUB	CX,AX
	SBB	DX,BX
	JB	V15_07
	PUSH	DX
	OR	DX,CX
	POP	DX
	JE	V15_07
	OR	DX,DX
	MOV	DX,LenMenu
	JNE	V15_05
	CMP	DX,CX
	JBE	V15_05
	MOV	DX,CX
V15_05:	PUSH	DX
	MOV	BX,ES:ViHandle
	MOV	DX,ES:ViSearch.Lo
	MOV	CX,ES:ViSearch.Hi
	MOV	AX,4200h
	CALL	Intr21
	POP	CX
	CMP	AX,ES:ViSearch.Lo
	JNE	V15_07
	CMP	DX,ES:ViSearch.Hi
	JNE	V15_07
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	DX,OFFSET MenuBuf
	MOV	AH,3Fh
	CALL	Intr21
	POP	DS
	MOV	CX,AX
	MOV	AL,2
	JC	V15_03
	MOV	SI,DX
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	V82_00
	POP	DS
	LEA	DI,[SI-MenuBuf]
	MOV	AH,1
	JNC	V15_10
V15_06:	SUB	CX,WORD PTR ES:HelpBuf[90]
	JB	V15_07
	INC	CX
	ADD	ES:ViSearch.Lo,CX
	ADC	ES:ViSearch.Hi,0
	JMP	V15_04
V15_07:	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	DX,OFFSET ViFl_S
	MOV	BP,OFFSET SrEr_B
	CALL	Dialog
	POP	DS
	MOV	AL,3
	JMP	V15_22
V15_10:	MOV	ES:ViSrPan,AL
	MOV	BX,WORD PTR ES:HelpBuf[90]
	CMP	AL,0
	JNZ	V15_11
	MOV	BX,ES:HexLen
V15_11:	MOV	ES:ViSrLen,BL
	SUB	DI,BX
	ADD	ES:ViSearch.Lo,DI
	ADC	ES:ViSearch.Hi,0
	OR	AH,AH
	JZ	V15_15
	MOV	AX,ES:ViSearch.Lo
	MOV	BX,ES:ViSearch.Hi
	SUB	AX,2000
	SBB	BX,0
	JAE	V15_12
	XOR	AX,AX
	XOR	BX,BX
V15_12:	AND	AX,WORD PTR 0FFF0h
	MOV	ES:ViOffset.Lo,AX
	MOV	ES:ViOffset.Hi,BX
	MOV	CX,ES:ViLen.Lo
	MOV	DX,ES:ViLen.Hi
	SUB	CX,AX
	SBB	DX,BX
	MOV	AX,ES:ViSpace
	OR	DX,DX
	JNE	V15_13
	CMP	AX,CX
	JBE	V15_13
	MOV	AX,CX
V15_13:	MOV	ES:ViLoaded,AX
	MOV	BX,ES:ViHandle
	MOV	SI,ES:ViOffset.Lo
	MOV	DI,ES:ViOffset.Hi
	MOV	DX,SI
	MOV	CX,DI
	MOV	AX,4200h
	CALL	Intr21
	CMP	AX,SI
	JNE	V15_14
	CMP	DX,DI
	JNE	V15_14
	XOR	DX,DX
	MOV	CX,ES:ViLoaded
	CALL	V55_00
V15_14:	XOR	SI,SI
	MOV	DI,ES:ViSpace
	CALL	V60_00
V15_15:	MOV	CX,ES:ViSearch.Lo
	SUB	CX,ES:ViOffset.Lo
	MOV	ES:ViFirst,CX
	CMP	ES:ViTxHex,0
	JNZ	V15_21
	CALL	V20_00
	SUB	CX,SI
	JB	V15_21
	XOR	BX,BX
	JCXZ	V15_18
V15_16:	LODSB
	CMP	AL,9
	JNE	V15_17
	OR	BL,7
V15_17:	INC	BX
	LOOP	V15_16
V15_18:	MOV	AX,ES:ViCol
	CMP	BX,AX
	JA	V15_19
	OR	BX,BX
	JE	V15_20
	DEC	BX
	JMP	V15_20
V15_19:	ADD	BL,ES:ViSrLen
	ADC	BH,0
	ADD	AL,ES:ViSize.X
	ADC	AH,0
	SUB	BX,AX
	JB	V15_21
	ADD	BX,AX
	SUB	BL,ES:ViSize.X
	SBB	BH,0
	INC	BX
V15_20:	MOV	ES:ViCol,BX
V15_21:	MOV	AL,0
V15_22:	POP	BX
	POP	DX
	POP	SI
	CALL	ResWin
	CALL	Window
	CMP	AL,0
	JZ	V15_24
	MOV	ES:ViSearch.Lo,0FFF0h
	MOV	ES:ViSearch.Hi,0FFFFh
	JMP	V00_30
V15_24:	JMP	V00_20

NextLin	PROC	NEAR			; Find next line
	PUSH	AX
	PUSH	DX
	PUSH	BX
	PUSH	SI
	MOV	BX,SP
	MOV	DX,DS
	CallC	NextLin_c
	POP	SI
	POP	BX
	POP	DX
	NEG	AX			; CY if there is no such line
	POP	AX
	RET
NextLin	ENDP

PrevLin	PROC	NEAR			; Find previous line
	PUSH	AX
	PUSH	DX
	PUSH	BX
	PUSH	SI
	MOV	BX,SP
	MOV	DX,DS
	CallC	PrevLin_c
	POP	SI
	POP	BX
	POP	DX
	NEG	AX			; CY if there is no such line
	POP	AX
	RET
PrevLin	ENDP

V20_00	PROC	NEAR			; Find first line of screen
	PUSH	AX
	MOV	AX,DS
	CallC	V20_00_c
	MOV	BX,ES:ViPoint
	POP	AX
	RET
V20_00	ENDP

V25_00	PROC	NEAR			; Output status line
	PUSH	AX
	CallC	V25_00_c
	POP	AX
	RET
V25_00	ENDP

V30_00	PROC	NEAR			; Output screen
	PUSH	AX
	MOV	AX,DS
	CallC	V30_00_c
	NEG	AX			; CY if the text ends on the screen
	POP	AX
	RET
V30_00	ENDP
	FromC	T70_00

V31_00	PROC	NEAR			; Clear line
	CALL	FAR PTR V31_00_c
	RET
V31_00	ENDP

V32_00	PROC	NEAR			; Output line #AL
	PUSH	AX
	PUSH	BX
	PUSH	CX
	MOV	BX,DS
	MOV	CX,BP
	CallC	V32_00_c
	POP	CX
	POP	BX
	POP	AX
	RET
V32_00	ENDP

V40_00	PROC	NEAR			; Test 100%
	PUSH	AX
	CallC	V40_00_c
	NEG	AX			; CY if the file goes on
	POP	AX
	RET
V40_00	ENDP

V50_00	PROC	NEAR			; Read next part
	PUSH	AX
	MOV	AX,DS
	CallC	V50_00_c
	POP	AX
	RET
V50_00	ENDP

V51_00	PROC	NEAR			; Read previous part
	PUSH	AX
	MOV	AX,DS
	CallC	V51_00_c
	POP	AX
	RET
V51_00	ENDP

V55_00	PROC	NEAR			; Read block
	MOV	AX,DS
	CallC	V55_00_c
	RET
V55_00	ENDP

V60_00	PROC	NEAR			; Format text
	PUSH	AX
	MOV	AX,DS
	CallC	V60_00_c
	POP	AX
	RET
V60_00	ENDP

V80_00	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	BX,528h
	MOV	BP,OFFSET SrFor_P
	CALL	DialBox
	CALL	Window
	CALL	V81_00
	POP	DS
	RET
V80_00	ENDP

V81_00	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR V81_00_c
	POP	AX
	RET
V81_00	ENDP

V82_00	PROC	NEAR			; Search
	PUSH	BX
	PUSH	DX
	PUSH	AX
	MOV	BX,DS
	CallC	V82_00_c
	MOV	SI,DX			; Behind what was found
	POP	DX
	MOV	BL,AH
	MOV	AH,DH
	NEG	BL			; CY if nothing was found
	POP	DX
	POP	BX
	RET
V82_00	ENDP
