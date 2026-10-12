;-------------------------------------------------------
;		Version :	27.08.1999
;-------------------------------------------------------

; Of this file, and named by C code or by another file.
	PUBLIC	F35_60, F35_61

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