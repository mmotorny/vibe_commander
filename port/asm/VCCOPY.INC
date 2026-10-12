;-------------------------------------------------------
;		Version :	27.08.1999
;-------------------------------------------------------

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
	PUBLIC	MovDi_P, OvrCo_B, Cont_B, OpnEr_B, Itslf_B, RenEr_B, MkErr_B

	.CODE
F36_00:	MOV	CL,1			; Rename / Move
	JMP	F35_01
F36_01:	CALL	F35_40
	MOV	BP,OFFSET MkErr_B
	CALL	Dialog
F36_02:	JMP	Func04
F36_03:	PUSH	ES
	POP	DS
	MOV	SI,DS:TreAdr
	MOV	DI,OFFSET CpDest
	MOV	CX,13
	PUSH	DI
	REP	MOVSB
	POP	SI
	MOV	BP,OFFSET Renam_B
	CALL	Dialog
	CMP	AL,0
	JNE	F36_02
	MOV	BX,SI
	CALL	CutPath
	CMP	SI,BX
	JNE	F36_01
	MOV	SI,OFFSET CrnPath
	MOV	DI,OFFSET CpInpt
	MOV	CX,LenPath
	PUSH	DI
	REP	MOVSB
	POP	SI
	PUSH	SI
	CALL	CutPath
	MOV	DI,SI
	MOV	SI,BX
	MOV	CX,13
	REP	MOVSB
	POP	SI
	MOV	DI,SI
	MOV	CX,LenStr+1
	PUSH	CX
	REPNE	SCASB
	POP	CX
	JNE	F36_01
	MOV	DI,OFFSET ToFile
	REP	MOVSB
	MOV	DS:CopyMov,3
F36_04:	MOV	SI,OFFSET CrnPath
	MOV	DI,OFFSET CpSrc
	PUSH	DI
	MOV	CX,LenPath
	REP	MOVSB
	POP	SI
	CALL	CutPath
	MOV	DS:CpBgNam,SI
	CALL	F35_70
	MOV	DS:ZoomOne,AL
	MOV	AL,10h
	CALL	C00_00
	CMP	AL,0
	JZ	F36_06
	PUSH	ES
	POP	DS
	CMP	DS:CopyMov,0
	JE	F36_06
	MOV	DI,OFFSET CrnPath
	MOV	SI,DI
	MOV	CX,LenPath+13
	MOV	AL,0
	REPNE	SCASB
	DEC	DI
	MOV	CX,DI
	SUB	CX,SI
	MOV	DI,OFFSET DosPth
	REPE	CMPSB
	JNE	F36_05
	PUSH	DI
	MOV	SI,OFFSET ToFile
	MOV	DI,OFFSET CpInpt
	MOV	CX,LenStr+1
	CALL	MvzLine
	POP	SI
	MOV	CX,LenPath
	REP	MOVSB
	MOV	DX,OFFSET CpInpt+2
	MOV	AH,3Bh
	CALL	Intr21
F36_05:	MOV	SI,OFFSET ToFile
	MOV	DI,OFFSET CrnPath
	MOV	CX,LenStr+1
	REP	MOVSB
F36_06:	JMP	FncExt

F35_00:	MOV	CL,0			; Copy
F35_01:	MOV	ES:CopyMov,CL
	MOV	AL,20
	ADD	AL,CL
	MOV	ES:HlpPage,AL
	CMP	DS:Visible,0
	JZ	F35_03
	CMP	DS:WinTyp,1
	JNE	F35_02
	MOV	AX,ES:TreAdr
	CMP	AX,OFFSET TreeBuf.TreList
	JBE	F35_11
	OR	CL,CL
	JE	F35_04
	JMP	F36_03
F35_02:	JA	F35_03
	XOR	BX,BX
	XOR	DX,DX
	CMP	ES:OneFile,BL
	JNZ	F35_12
	CALL	TotSel
	MOV	AX,BX
	OR	AX,DX
	JNE	F35_04
F35_12:	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F35_11
	CMP	DS:[SI].NameF,'.'
	JNE	F35_04
F35_11:	JMP	Func04
F35_03:	JMP	F45_01
F35_04:	CALL	F35_50
	CMP	DS:WinTyp,1
	JE	F35_05
	MOV	SI,DS:CurAdr
	MOV	CX,13
	MOV	AX,BX
	OR	AX,DX
	JE	F35_06
	CALL	FilDir
	DEC	DI
	JMP	F35_07
F35_05:	PUSH	DI
	PUSH	ES
	POP	DS
	MOV	SI,OFFSET CrnPath
	MOV	DI,OFFSET TmpErr
	PUSH	DI
	MOV	CX,LenPath+13
	REP	MOVSB
	POP	SI
	MOV	AX,LenStr-10
	CALL	ShrtLin
	POP	DI
	MOV	CX,LenStr
F35_06:	MOV	AL,'"'
	STOSB
	CALL	MvzLine
	MOV	AL,'"'
	STOSB
F35_07:	PUSH	CS
	POP	DS
	MOV	SI,OFFSET CopTo_S+2
	MOV	CX,3
	REP	MOVSB
	MOV	CX,OFFSET CpVar2+LenStr
	SUB	CX,DI
	JBE	F35_08
	MOV	AL,' '
	REP	STOSB
F35_08:	MOV	AL,0
	STOSB
	PUSH	ES
	POP	DS
	CALL	F35_40
	MOV	BP,OFFSET Copy1_B
	CALL	Dialog
	CMP	AL,0
	JNE	F35_09
	MOV	SI,OFFSET CpInpt
	MOV	DI,OFFSET ToFile
	MOV	CX,LenStr+1
	REP	MOVSB
	CMP	DS:ToFile,0
	JE	F35_09
	PUSH	DS
	MOV	DS,DS:WCBcrn
	CMP	DS:WinTyp,1
	POP	DS
	JE	F35_10
	CALL	F35_70
	MOV	DS:CpBgNam,OFFSET CpSrc
	JMP	FncSel
F35_09:	JMP	Init50
F35_10:	JMP	F36_04

F46_00:	MOV	CL,1			; Rename / Move...
	JMP	F45_01
F45_00:	MOV	CL,0			; Copy...
F45_01:	MOV	ES:CopyMov,CL
	MOV	AL,20
	ADD	AL,CL
	MOV	ES:HlpPage,AL
	CALL	F35_50
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET CopFi_S+12
	MOV	CX,4
	REP	MOVSB
	PUSH	ES
	POP	DS
	CALL	F35_40
	MOV	BP,OFFSET Copy2_B
	CALL	Dialog
	CMP	AL,0
	JNE	F45_02
	MOV	SI,OFFSET CpInpt
	MOV	DI,OFFSET ToFile
	MOV	CX,LenStr+1
	PUSH	CX
	REP	MOVSB
	POP	CX
	MOV	SI,OFFSET ViewFil
	MOV	DI,OFFSET CpSrc
	REP	MOVSB
	MOV	AL,0
	CMP	AL,DS:ViewFil
	JE	F45_02
	CMP	AL,DS:ToFile
	JE	F45_02
	MOV	SI,OFFSET CpSrc
	CALL	CutPath
	MOV	DS:CpBgNam,SI
	CALL	F35_70
	CALL	F35_40
	MOV	BX,DX
	JMP	FncMas
F45_02:	JMP	Init50

F55_00:	MOV	CL,0			; ^F5 - copy file
	JMP	F56_01

F56_00:	MOV	CL,1			; ^F6 - rename or move file
F56_01:	MOV	ES:OneFile,1
	JMP	F35_01

;	Return :	CS:DX - the title of the boxes
F35_40	PROC	NEAR
	CallC	F35_40_c
	RET
F35_40	ENDP

F35_50	PROC	NEAR			; Set default target path
	CMP	ES:OneFile,0
	JZ	F35_51
	CMP	ES:CopyMov,0
	JNZ	F35_54
F35_51:	PUSH	DS
	CALL	InvWCB
	CMP	DS:Visible,0
	JZ	F35_53
	CMP	DS:WinTyp,1
	JAE	F35_52
	MOV	SI,OFFSET WinPath
	JMP	F35_57
F35_52:	JA	F35_53
	MOV	SI,OFFSET CrnPath
	JMP	F35_56
F35_53:	CMP	ES:CopyMov,0
	JE	F35_55
	MOV	AX,BX
	OR	AX,DX
	JNE	F35_55
	POP	DS
F35_54:	PUSH	DS
	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F35_55
	CMP	DS:[SI].NameF,'.'
	JNE	F35_57
F35_55:	MOV	SI,OFFSET ToFile
F35_56:	PUSH	ES
	POP	DS
F35_57:	MOV	DI,OFFSET CpInpt
	MOV	CX,LenStr
	REP	MOVSB
	MOV	AL,0
	STOSB
	MOV	SI,OFFSET Copy0_S
	MOV	BP,OFFSET Copy0_S+1
	MOV	AX,5
	CMP	ES:CopyMov,0
	JE	F35_58
	MOV	SI,OFFSET Ren0_S
	MOV	BP,OFFSET RnMv_S
	MOV	AL,15
F35_58:	PUSH	CS
	POP	DS
	MOV	DI,OFFSET CpVar1
	MOV	CX,CpVar2-CpVar1
	REP	MOVSB
	PUSH	AX
	PUSH	DI
	MOV	CX,LenStr
	MOV	AL,' '
	REP	STOSB
	MOV	AL,0
	STOSB
	POP	DI
	POP	CX
	MOV	SI,BP
	REP	MOVSB
	POP	DS
        RET
F35_50  ENDP

F35_60	PROC	NEAR			; Tree
	ADD	BH,2
F35_61:	ADD	BX,305h
	CMP	AL,1
	JE	F35_62
	CLC
	RET
F35_62:	MOV	SI,OFFSET DosPth
	MOV	DI,OFFSET MenuBuf+350
	MOV	CX,LenPath
	PUSH	SI
	PUSH	DI
	PUSH	CX
	REP	MOVSB
	OR	AH,AH
	JZ	F35_63
	MOV	SI,OFFSET DosBuf+LenCmd+10+3
	MOV	DI,OFFSET TmpBuf
	PUSH	DI
	MOV	CX,LenStr+1
	REP	MOVSB
	POP	SI
	CALL	NormStr
	MOV	AX,DS:[SI]
	CMP	AH,':'
	JNE	F35_63
	CALL	UpCase
	SUB	AL,'A'
	MOV	DL,AL
	MOV	AH,0Eh
	INT	21h
	CALL	SetPth
	JC	F35_64
F35_63:	MOV	AL,28
	CALL	ChTree
F35_64:	POP	CX
	POP	SI
	POP	DI
	PUSHF
	MOV	DL,DS:[SI]
	REP	MOVSB
	SUB	DL,'A'
	MOV	AH,0Eh
	INT	21h
	POPF
	JC	F35_66
	MOV	SI,OFFSET TmpBuf
	MOV	DI,OFFSET DosBuf+LenCmd+10+3
	MOV	CX,LenPath+13
	PUSH	CX
	PUSH	DI
	CALL	MvzLine
	POP	SI
	MOV	AX,'\'
	CMP	AL,DS:[DI-1]
	JE	F35_65
	STOSW
F35_65:	POP	CX
	MOV	DI,SI
	MOV	AL,0
	MOV	DS:LenStr[SI],AL
	REPNE	SCASB
	DEC	DI
	SUB	DI,SI
	MOV	WORD PTR DS:[DosBuf+LenCmd+10],DI
	MOV	BYTE PTR DS:[DosBuf+LenCmd+10+2],1
	MOV	CX,LenStr
	MOV	BP,OFFSET Copy1_B
	MOV	BP,CS:[BP]
	MOV	AH,CS:[BP+1]
	CALL	InSt10
	STC
F35_66:	RET
F35_60	ENDP

F35_70	PROC	NEAR			; Init variables
	MOV	AL,0
	MOV	DS:CpCase,AL
	MOV	DS:CpDev2,AL
	MOV	DS:CpTree1,AL
	MOV	DS:CpTree2,AL
	MOV	DS:CpAll,AL
	MOV	DS:FncChg,AL
	MOV	DS:FncAddr,OFFSET C00_00
	RET
F35_70	ENDP

;	Copy, rename or move a file or a directory
;			AL - its attributes
;	Return :	AL - 1 if it was done
;			AH - 1 to stop
C00_00	PROC	NEAR
	PUSH	ES
	POP	DS
	CALL	FAR PTR C00_00_c
	RET
C00_00	ENDP

; The entries by which port/copy.c calls what is still assembly are next to
; their routines, or with the stubs of the batch that made them: OncZoom_a and
; TstBrk_a in VCSUB3.INC.
