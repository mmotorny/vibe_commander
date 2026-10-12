;-------------------------------------------------------
;		Version :	14.06.2000
;-------------------------------------------------------

; Of this file, and named by C code or by another file.
	PUBLIC	ViewE_F, EditE_F

; Boxes, dialogs and texts that C code hands to DialBox and Dialog.
	PUBLIC	ViFl_S, Col_S, RdFil_P, FndEr_B, Srch_B, SrFor_P, SrEr_B
	PUBLIC	ViFil_B, Viewr_P, Editr_P, RunEr_B, RunEr_S, Run1_S, Run2_S

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
; The commands that view or edit a file are C: port/view.c, with F34_30
; and F34_40, which read a line of an extension file. F34_10 is where the
; mouse comes in with a click on the dirinfo of an Info panel.
F43_00:	CmdC	F43_00_c		; View...
F44_00:	CmdC	F44_00_c		; Edit...
F63_00:	CmdC	F63_00_c		; Internal viewer
F64_00:	CmdC	F64_00_c		; Internal editor
F33_00:	CmdC	F33_00_c		; View
F34_00:	CmdC	F34_00_c		; Edit
F34_10:	CmdC	F34_10_c		; Edit 'dirinfo'

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