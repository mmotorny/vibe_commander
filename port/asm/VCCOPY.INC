;-------------------------------------------------------
;		Version :	27.08.1999
;-------------------------------------------------------

; Of this file, and named by C code or by another file.
	PUBLIC	Prot_B

CpVar1	EQU	BYTE PTR TmpBuf+120	; Variable 1
CpVar2	EQU	BYTE PTR TmpBuf+140	; Variable 2

CpInpt	EQU	BYTE PTR TmpBuf+0	; Input buffer
CpMenu	EQU	BYTE PTR TmpBuf+230	; Menu variable

CpSrc	EQU	BYTE PTR TmpBuf+0	; Source filename
CpDest	EQU	BYTE PTR TmpBuf+130	; Destination filename

FncChg	EQU	BYTE PTR TmpBuf+91	; There was writing
SplitFl	EQU	BYTE PTR TmpBuf+94	; Split flag
CpCase	EQU	BYTE PTR TmpBuf+87	; Case: pathname, filename or none
CpAll	EQU	BYTE PTR TmpBuf+90	; Copy / rename all
CopyMov	EQU	BYTE PTR TmpBuf+100	; Copy / Move / Rename
CpBgNam	EQU	WORD PTR TmpBuf+102	; Start address of source filename
CpDev1	EQU	BYTE PTR TmpBuf+86	; Source is a device
CpDev2	EQU	BYTE PTR TmpBuf+104	; Destination is a device
CpTree1	EQU	BYTE PTR TmpBuf+88	; Source tree was changed
CpTree2	EQU	BYTE PTR TmpBuf+89	; Destination tree was changed
CpHndl1	EQU	WORD PTR TmpBuf+116	; Source handle
CpHndl2	EQU	WORD PTR TmpBuf+118	; Destination handle
CpRead	EQU	DWORD PTR TmpBuf+106	; Was read
CpWrite	EQU	DWORD PTR TmpBuf+110	; Was written
CpSize	EQU	DWORD PTR TmpBuf+96	; File size
CpAttr	EQU	WORD PTR TmpBuf+120	; File attribute
CpDate	EQU	WORD PTR TmpBuf+122	; File date
CpTime	EQU	WORD PTR TmpBuf+124	; File time
CpCont	EQU	WORD PTR TmpBuf+128	; Start segment to continue copying
CpCntBg	EQU	WORD PTR TmpBuf+126	; Base of the per cent counter
CpCntSz	EQU	BYTE PTR TmpBuf+95	; Length of the per cent counter

	.CONST
Copy0_S	DB	' Copy ',0
Move0_S	DB	' Move ',0
Ren0_S	DB	' Rename ',0
Ren1_S	DB	'Rename "^V" to',0
RnMv_S	DB	'Rename or move '
CopTo_S	DB	'^V to',0
CpTo1_S	DB	'to',0FFh,LenStr-2,0
CoMnu_S	DB	'[^I]',0FFh,3,'[ F10-Tree ]',0FFh,3,'[ Cancel ]',0
CopFi_S	DB	'Copying the file',0
MovFi_S	DB	'Moving the file',0
RenFi_S	DB	'Renaming or moving the file',0
CopDi_S	DB	'Copying the directory',0
MovDi_S	DB	'Renaming or moving the directory',0
Diagr_S	DB	0FFh, 24, 0
OvRn1_S	DB	'The following file exists',0
OvRn2_S	DB	'Do you wish to write over the old file?',0
OvrAp_S	DB	'Overwrite',0FFh,3,'All',0FFh,3,'aPpend',0FFh,3,'Skip',0
Prot1_S	DB	'is a read-only file.',0
Prot2_S	DB	'Do you still wish to overwrite it?',0
Cont1_S	DB	"There isn't enough room to copy",0
Cont2_S	DB	"Abort",0FFh,3,"Continue",0
OpnEr_S	DB	"Can't open the file",0
Itslf_S	DB	"You can't copy a file to itself",0
RenEr_S	DB	"Can't rename the file",0

	.DATA
Copy1_P	DB	Box_C, BoxCu_C, Box_C, 0
	DB	5, 2
	DW	OFFSET MenuBuf+500
	DW	CpVar1
	DB	0, 70
	DW	CpVar2
	DB	0, 70
	DW	OFFSET Var_S
	DW	OFFSET Var2_S
	DW	OFFSET Empty_S
	DW	0
	DW	OFFSET CoMnu_S
Copy1_B	DW	OFFSET Copy1_P, 528h, OFFSET F35_61
	DB	8, 2, 0, 8
	DB	1
	DB	1, 1, 0, 0
	DW	305h, CpInpt, 200h+LenStr
	DB	4
	DB	0, 0, 1, 1
	DW	516h, CpMenu, OFFSET Copy_D
	DB	0
Copy2_P	DB	Box_C, BoxCu_C, Box_C, 0
	DB	7, 2
	DW	OFFSET MenuBuf+500
	DW	CpVar1
	DB	0, 70
	DW	CpVar2
	DB	0, 70
	DW	OFFSET Var_S
	DW	OFFSET Var2_S
	DW	OFFSET Empty_S
	DW	OFFSET CpTo1_S
	DW	OFFSET Empty_S
	DW	0
	DW	OFFSET CoMnu_S
Copy2_B	DW	OFFSET Copy2_P, 528h, OFFSET F35_60
	DB	8, 2, 0, 8
	DB	1
	DB	2, 1, 0, 0
	DW	305h, ViewFil, 300h+LenStr
	DB	1
	DB	0, 2, 1, 1
	DW	505h, CpInpt, 200h+LenStr
	DB	4
	DB	1, 0, 2, 2
	DW	716h, CpMenu, OFFSET Copy_D
	DB	0
Renam_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 1
	DW	OFFSET WinBuf
	DW	CpDest
	DB	0, 30
	DW	OFFSET Ren0_S
	DW	OFFSET Ren1_S
	DW	OFFSET Empty_S
Renam_B	DW	OFFSET Renam_P, 528h, 0
	DB	1, 2, 0, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, CpDest, 20Ch
	DB	0
CopFi_P	DB	Box_C, 0, 0, 0
	DB	6, 2
	DW	OFFSET WinBuf
	DW	CpSrc
	DB	7, 64
	DW	CpDest
	DB	4, 64
	DW	OFFSET Copy0_S
	DW	OFFSET CopFi_S
	DW	OFFSET Var_S
	DW	OFFSET CopTo_S+3
	DW	OFFSET Var2_S
	DW	OFFSET Diagr_S
MovFi_P	DB	Box_C, 0, 0, 0
	DB	6, 2
	DW	OFFSET WinBuf
	DW	CpSrc
	DB	7, 64
	DW	CpDest
	DB	4, 64
	DW	OFFSET Move0_S
	DW	OFFSET MovFi_S
	DW	OFFSET Var_S
	DW	OFFSET CopTo_S+3
	DW	OFFSET Var2_S
	DW	OFFSET Diagr_S
RenFi_P	DB	Box_C, 0, 0, 0
	DB	4, 2
	DW	OFFSET WinBuf
	DW	CpSrc
	DB	7, 64-3
	DW	CpDest
	DB	7, 64
	DW	OFFSET Ren0_S
	DW	OFFSET RenFi_S
	DW	OFFSET CopTo_S
	DW	OFFSET Var2_S
CopDi_P	DB	Box_C, 0, 0, 0
	DB	5, 2
	DW	OFFSET WinBuf
	DW	CpSrc
	DB	6, 64
	DW	CpDest
	DB	6, 64
	DW	OFFSET Inc_S
	DW	OFFSET CopDi_S
	DW	OFFSET Var_S
	DW	OFFSET CopTo_S+3
	DW	OFFSET Var2_S
MovDi_P	DB	Box_C, 0, 0, 0
	DB	5, 2
	DW	OFFSET WinBuf
	DW	CpSrc
	DB	6, 64
	DW	CpDest
	DB	6, 64
	DW	OFFSET Inc_S
	DW	OFFSET MovDi_S
	DW	OFFSET Var_S
	DW	OFFSET CopTo_S+3
	DW	OFFSET Var2_S
OvrCo_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 1
	DW	OFFSET ErrBuf
	DW	CpDest
	DB	4, 64
	DW	OFFSET Inc_S
	DW	OFFSET OvRn1_S
	DW	OFFSET Var_S
	DW	OFFSET OvRn2_S
	DW	OFFSET OvrAp_S
OvrCo_B	DW	OFFSET OvrCo_P, 928h, 0
	DB	1, 0FFh, 0, 5
	DB	4
	DB	0, 0, 0, 0
	DW	517h, CpMenu, OFFSET OvrAp_D
	DB	0
Cont_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 2
	DW	OFFSET ErrBuf
	DW	CpSrc
	DB	7, 64
	DW	CpDest
	DB	4, 64
	DW	OFFSET Inc_S
	DW	OFFSET Cont1_S
	DW	OFFSET CopTo_S
	DW	OFFSET Var2_S
	DW	OFFSET Cont2_S
Cont_B	DW	OFFSET Cont_P, 928h, 0
	DB	1, 0FFh, 0, 5
	DB	4
	DB	0, 0, 0, 0
	DW	51Fh, CpMenu, OFFSET Cont_D
	DB	0
OpnEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	CpDest
	DB	4, 64
	DW	OFFSET Inc_S
	DW	OFFSET OpnEr_S
	DW	OFFSET Var_S
	DW	OFFSET OkCan_S
OpnEr_B	DW	OFFSET OpnEr_P, 928h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	422h, CpMenu, OFFSET OkCan_D
	DB	0
Itslf_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	CpSrc
	DB	7, 64
	DW	OFFSET Inc_S
	DW	OFFSET Itslf_S
	DW	OFFSET Var_S
	DW	OFFSET OkCan_S
Itslf_B	DW	OFFSET Itslf_P, 928h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	422h, CpMenu, OFFSET OkCan_D
	DB	0
RenEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	CpSrc
	DB	7, 64
	DW	OFFSET Ren0_S
	DW	OFFSET RenEr_S
	DW	OFFSET Var_S
	DW	OFFSET OkCan_S
RenEr_B	DW	OFFSET RenEr_P, 928h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	422h, CpMenu, OFFSET OkCan_D
	DB	0
Prot_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 1
	DW	OFFSET WinBuf+2000
	DW	CpDest
	DB	4, 64
	DW	OFFSET Inc_S
	DW	OFFSET Var_S
	DW	OFFSET Prot1_S
	DW	OFFSET Prot2_S
	DW	OFFSET YesNo_S
Prot_B	DW	OFFSET Prot_P, 928h, 0
	DB	1, 0FFh, 0, 5
	DB	4
	DB	0, 0, 0, 0
	DW	523h, OFFSET TmpErr, OFFSET YesNo_D
	DB	0
Copy_D	DB	 8, 0
	DB	12,11
	DB	10,26
	DB	 0
OvrAp_D	DB	11, 0
	DB	 5,12
	DB	 8,18
	DB	 6,27
	DB	 0
OvrSk_D	DB	11, 0
	DB	 5,12
	DB	 6,18
	DB	 0
Cont_D	DB	 7, 0
	DB	10, 8
	DB	 0

; Texts and boxes that C code hands to OncZoom and Dialog. MkErr_B is of
; VCF1.INC, which has made the others of its own public.
	PUBLIC	Copy0_S, Move0_S, Ren0_S, RenFi_P, CopFi_P, MovFi_P, CopDi_P
	PUBLIC	MovDi_P, OvrCo_B, Cont_B, OpnEr_B, Itslf_B, RenEr_B
; What the commands in C copy texts from and hand to Dialog.
	PUBLIC	RnMv_S, CopTo_S, CopFi_S, Copy1_B, Copy2_B, Renam_B

	.CODE
; What Copy and Rename ask before anything is copied is C: F35_01_c and
; F45_01_c of port/copy.c, which get the panel in AX, the command in CL,
; and BX and DX as the key has left them: Shift-F6 goes by them without
; having set them. They return in AX where to go on, and in DX the title
; of the boxes for FncMas.
F36_00:	MOV	CL,1			; Rename / Move
	JMP	F35_01
F35_00:	MOV	CL,0			; Copy
F35_01:	MOV	AX,DS
	CallC	F35_01_c
	JMP	F35_90

F46_00:	MOV	CL,1			; Rename / Move...
	JMP	F45_01
F45_00:	MOV	CL,0			; Copy...
F45_01:	MOV	AX,DS
	CallC	F45_01_c
F35_90:	MOV	SI,AX
	ADD	SI,SI
	MOV	BX,DX
	JMP	CS:F35_T[SI]
F35_T	DW	Func04, Init50, FncSel, FncMas, FncExt

F55_00:	MOV	CL,0			; ^F5 - copy file
	JMP	F56_01

F56_00:	MOV	CL,1			; ^F6 - rename or move file
F56_01:	MOV	ES:OneFile,1
	JMP	F35_01

; What the dialogs of the commands have Dialog call when they are answered:
; F35_61_c of port/copy.c, which shows the tree for F10 and its button.
;			   AX - the answer
;			BH,BL - where the box begins
;	Return :	 CY=1 - go on with the dialog
F35_60	PROC	NEAR			; Tree
	ADD	BH,2
F35_61:	ADD	BX,305h
	PUSH	AX
	CallC	F35_61_c
	NEG	AX			; CY for 1
	POP	AX
	RET
F35_60	ENDP

; The entries by which port/copy.c calls what is still assembly are next to
; their routines.
