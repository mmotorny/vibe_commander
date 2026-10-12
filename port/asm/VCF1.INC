;-------------------------------------------------------
;		Version :	16.10.1999
;-------------------------------------------------------

; What deletes, sets attributes and goes through a directory is C, in
; port/dirfunc.c. These are the boxes and the texts that it hands to
; OncZoom and Dialog.
	PUBLIC	DelFl_P, DelDr_P, DelCo_B, RdOnl_B, NoEmp_B, DelEr_B, CntDi_B
	PUBLIC	StAtr_P, AtrEr_B, NoDat_B, PthEr_B, Del_S, DelFs_S, Empty_S
	PUBLIC	NoFnd_S, NoAtr_S
; Those that its commands hand to Dialog, a text that F48_00 copies, and
; the stubs whose offsets they put into FncAddr.
	PUBLIC	MkDi1_B, MkDi2_B, CrDir_S, MkDir_S, DelDi_B, Subdi_S, DelFi_B
	PUBLIC	DelSe_B, DlMas_B, DlWrn_B, DlWr4_S, StAt1_B, StAt2_B, Attr8_S
	PUBLIC	Label_B, DelVo_B, ErVol_B, NoFnc_B, F38_40, M30_50

	.CONST
MkDir_S	DB	" Make directory ",0
MkDi1_S	DB	"^I",0FFh,LenStr-20,0
CrDir_S	DB	"Create the directory",0
MkErr_S	DB	"Can't create the directory",0

	.DATA
MkDi1_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	MkDir_S
	DW	MkDi1_S
	DW	Empty_S
MkDi1_B	DW	MkDi1_P, 528h, 0
	DB	1, 2, 22, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, TmpBuf, 200h+LenStr
	DB	0
MkDi2_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	MkDir_S
	DW	Inc_S
	DW	Empty_S
MkDi2_B	DW	MkDi2_P, 528h, 0
	DB	1, 2, 22, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, TmpBuf, 20Ch
	DB	0
MkErr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	TmpBuf+130
	DB	6, 64
	DW	Inc_S
	DW	MkErr_S
	DW	Var_S
	DW	Ok_S
MkErr_B	DW	MkErr_P, 928h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf+200, OkMn_D
	DB	0

	.CODE
; The commands of this file are C, in port/dirfunc.c. Each is jumped to,
; from the table of the keys or from a menu, with the panel in DS, and
; jumps on by what its function returns.
F37_00:	MOV	AX,DS			; F7 - make directory
	CallC	F37_00_c
	JMP	F1Next

F47_00:	CallC	F47_00_c		; Shift F7 - make directory
	JMP	F1Next

; Where a command that is C goes on: AX says where (TO_ in port/vc.h).
F1Next:	DEC	AX
	JNS	F1Nx1
	JMP	Func04
F1Nx1:	JNZ	F1Nx2
	JMP	FncSel
F1Nx2:	DEC	AX
	JNZ	F1Nx3
	JMP	FncExt
F1Nx3:	JMP	FncEx1

	.CONST
Del_S	DB	" Delete ",0
DlMas_S	DB	"Delete the file:",0FFh,LenStr-16,0
DelFl_S	DB	"Deleting the file^I",0
DelFs_S	DB	"s",0
DelDr_S	DB	"Deleting the subdirectory",0
DelSl_S	DB	"You have selected",0
Del1_S	DB	"Do you wish to delete^I",0
Subdi_S	DB	" the directory",0
RdOn_S	DB	"The following file is marked read-only.",0
NoEmp_S	DB	"The following subdirectory is not empty.",0
StDel_S	DB	"Do you still wish to delete it?",0
CnDl_S	DB	"Can't delete the file",0
CnSb_S	DB	"Can't delete the subdirectory",0
DlWr1_S	DB	"WARNING!",0
DlWr2_S	DB	"Do you want to delete all the files",0
DlWr3_S	DB	"^V?",0
DlWr4_S	DB	"in the current directory",0
DlSel_S	DB	"You are ^uDELETING^u",0
From_S	DB	"^V from",0
Var2_S	DB	"^W",0
DelCa_S	DB	" Delete",0FFh,3,"Cancel ",0
DelAl_S	DB	" Delete",0FFh,3,"All",0FFh,3,"Cancel ",0
DelSk_S	DB	"Delete",0FFh,3,"Skip",0FFh,3,"Cancel",0

	.DATA
DelCa_D	DB	8, 0
	DB	8, 9
	DB	0
DelAl_D	DB	8, 0
	DB	5, 9
	DB	8,15
	DB	0
DelSk_D	DB	8, 0
	DB	6, 9
	DB	8,16
	DB	0
DelFl_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf
	DB	7, 64
	DW	Del_S
	DW	DelFl_S
	DW	Var_S
DelDr_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf
	DB	6, 64
	DW	Inc_S
	DW	DelDr_S
	DW	Var_S
DlMas_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	Del_S
	DW	DlMas_S
	DW	Empty_S
DlMas_B	DW	DlMas_P, 528h, 0
	DB	1, 2, 23, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, ViewFil, LenStr
	DB	0
DelSe_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 1
	DW	WinBuf
	DW	TmpBuf
	DB	0, 64
	DW	Del_S
	DW	DelSl_S
	DW	Var_S
	DW	DelCa_S
DelSe_B	DW	DelSe_P, 528h, F38_10
	DB	1, 2, 23, 1
	DB	4
	DB	0, 0, 0, 0
	DW	41Fh, TmpBuf+200, DelCa_D
	DB	0
DelCo_P	DB	Err_C, ErInv_C, ErInv_C, ErBr_C
	DB	5, 2
	DW	ErrBuf
	DW	TmpBuf
	DB	0, 64
	DW	TmpBuf+100
	DB	6, 64
	DW	Del_S
	DW	DlSel_S
	DW	From_S
	DW	Var2_S
	DW	DelAl_S
DelCo_B	DW	DelCo_P, 928h, 0
	DB	1, 0FFh, 0, 1
	DB	4
	DB	0, 0, 0, 0
	DW	51Ch, TmpBuf+100, DelAl_D
	DB	0
DelFi_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 1
	DW	WinBuf
	DW	TmpBuf
	DB	7, 64
	DW	Del_S
	DW	Del1_S
	DW	Var_S
	DW	DelCa_S
DelFi_B	DW	DelFi_P, 528h, 0
	DB	1, 2, 23, 1
	DB	4
	DB	0, 0, 0, 0
	DW	41Fh, TmpBuf+100, DelCa_D
	DB	0
DelDi_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 1
	DW	WinBuf
	DW	TmpBuf
	DB	6, 62
	DW	Del_S
	DW	Del1_S
	DW	Kov_S
	DW	DelCa_S
DelDi_B	DW	DelDi_P, 528h, 0
	DB	1, 2, 23, 1
	DB	4
	DB	0, 0, 0, 0
	DW	41Fh, TmpBuf+100, DelCa_D
	DB	0
RdOnl_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 1
	DW	ErrBuf
	DW	TmpBuf
	DB	7, 64
	DW	Inc_S
	DW	RdOn_S
	DW	Var_S
	DW	StDel_S
	DW	DelSk_S
RdOnl_B	DW	RdOnl_P, 928h, 0
	DB	1, 0FFh, 0, 5
	DB	4
	DB	0, 0, 0, 0
	DW	51Ch, TmpBuf+230, DelSk_D
	DB	0
NoEmp_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 1
	DW	ErrBuf
	DW	TmpBuf
	DB	6, 64
	DW	Del_S
	DW	NoEmp_S
	DW	Var_S
	DW	StDel_S
	DW	DelAl_S
NoEmp_B	DW	NoEmp_P, 928h, 0
	DB	1, 0FFh, 0, 1
	DB	4
	DB	0, 0, 0, 0
	DW	51Ch, TmpBuf+100, DelAl_D
	DB	0
DlWrn_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	5, 1
	DW	ErrBuf
	DW	TmpBuf+100
	DB	4, 64
	DW	Del_S
	DW	DlWr1_S
	DW	DlWr2_S
	DW	DlWr3_S
	DW	DelCa_S
DlWrn_B	DW	DlWrn_P, 528h, 0
	DB	1, 0FFh, 0, 1
	DB	4
	DB	0, 0, 0, 0
	DW	51Fh, TmpBuf+100, DelCa_D
	DB	0
DelEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	TmpBuf+130
	DB	7, 64
	DW	Inc_S
	DW	CnDl_S
	DW	Var_S
	DW	OkCan_S
DelEr_B	DW	DelEr_P, 928h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	422h, TmpBuf+230, OkCan_D
	DB	0
CntDi_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	TmpBuf
	DB	6, 64
	DW	Inc_S
	DW	CnSb_S
	DW	Var_S
	DW	OkCan_S
CntDi_B	DW	CntDi_P, 928h, 0
	DB	1, 0FFh, 0, 2
	DB	4
	DB	0, 0, 0, 0
	DW	422h, TmpBuf+100, OkCan_D
	DB	0

	.CODE
F38_00:	MOV	AX,DS			; F8 - delete
	CallC	F38_00_c
	JMP	F1Next

; Dialog calls this when the box for selected files is answered.
;			   AL - the answer
;	Return :	   AL - the answer to go by, CY=0
F38_10	PROC	NEAR
	CallC	F38_10_c
	CLC
	RET
F38_10	ENDP

;			   AL - attributes of what is at TmpBuf
;	Return :	   AX - 1 deleted, 0 not, 100h to stop
F38_40	PROC	NEAR			; Delete file or directory
	CallC	F38_40_c
	RET
F38_40	ENDP

; For F38_70 and F48_00 in C: DOS makes an FCB at DI of the name at SI,
; by the flags in AL, and says in AL whether the name is a mask. Not
; through Intr21, as in the original.
	PUBLIC	Fcb29_a
Fcb29_a	PROC	FAR
	PUSH	DS
	POP	ES
	MOV	AH,29h
	INT	21h
	RET
Fcb29_a	ENDP

F48_00:	CallC	F48_00_c		; Shift F8 - delete
	JMP	F1Next

F58_00:	MOV	AX,DS			; ^F8 - delete one file
	CallC	F58_00_c
	JMP	F1Next

	.CONST
Attr_S	DB	" Attributes ",0
Attr1_S	DB	"Change file attributes^I",0
Attr2_S	DB	"^uSet  Clear^u",0FFh,21,0
Attr3_S	DB	" [ ]  [ ] Read only",0FFh,4,"Date",0FFh,6,0
Attr4_S	DB	" [ ]  [ ] Archive",0FFh,16,0
Attr5_S	DB	" [ ]  [ ] Hidden",0FFh,7,"Time",0FFh,6,0
Attr6_S	DB	" [ ]  [ ] System",0FFh,17,0
Attr7_S	DB	"[ Set ]   [ Cancel ]",0
Attr8_S	DB	" for",0
StAtr_S	DB	"Setting attributes:",0
NoDt1_S	DB	"Can't set the attributes.",0
NoDt2_S	DB	"Illegal date or time.",0
NoFnd_S	DB	"Can't find the file",0
NoAtr_S	DB	"Can't set attributes",0

	.DATA
StAt1_P	DB	Box_C, BoxCu_C, Box_C, BoxBr_C
	DB	9, 1
	DW	WinBuf
	DW	TmpBuf+100
	DB	7, 62
	DW	Attr_S
	DW	Attr1_S
	DW	Kov_S
	DW	Attr3_S + 5
	DW	Attr4_S + 5
	DW	Attr5_S + 5
	DW	Attr6_S + 5
	DW	0
	DW	Attr7_S
StAt1_B	DW	StAt1_P, 428h, M30_60
	DB	1, 2, 24, 0
	DB	3			; Read only
	DB	6, 1, 0, 4
	DW	407h, TmpBuf+0, TmpBuf+0
	DB	3			; Archive
	DB	0, 2, 1, 4
	DW	507h, TmpBuf+1, TmpBuf+1
	DB	3			; Hidden
	DB	1, 3, 2, 5
	DW	607h, TmpBuf+2, TmpBuf+2
	DB	3			; System
	DB	2, 6, 3, 5
	DW	707h, TmpBuf+3, TmpBuf+3
	DB	1			; Date
	DB	3, 5, 1, 4
	DW	517h, TmpBuf+15, 00Ah
	DB	1			; Time
	DB	4, 6, 3, 5
	DW	717h, TmpBuf+30, 10Ah
	DB	4			; Set / Cancel
	DB	5, 0, 6, 6
	DW	91Eh, TmpBuf+8, SetCa_D
	DB	0
StAt2_P	DB	Box_C, BoxCu_C, Box_C, BoxBr_C
	DB	9, 0
	DW	WinBuf
	DW	Attr_S
	DW	Attr1_S
	DW	Attr2_S
	DW	Attr3_S
	DW	Attr4_S
	DW	Attr5_S
	DW	Attr6_S
	DW	0
	DW	Attr7_S
StAt2_B	DW	StAt2_P, 428h, M30_60
	DB	1, 2, 24, 0
	DB	3			; Read only
	DB	10, 1, 0, 4
	DW	407h, TmpBuf+0, TmpBuf+0
	DB	3			; Archive
	DB	0, 2, 1, 5
	DW	507h, TmpBuf+1, TmpBuf+1
	DB	3			; Hidden
	DB	1, 3, 2, 6
	DW	607h, TmpBuf+2, TmpBuf+2
	DB	3			; System
	DB	2, 10, 3, 7
	DW	707h, TmpBuf+3, TmpBuf+3
	DB	3			; Read only
	DB	3, 5, 0, 8
	DW	40Ch, TmpBuf+10, TmpBuf+10
	DB	3			; Archive
	DB	4, 6, 1, 8
	DW	50Ch, TmpBuf+11, TmpBuf+11
	DB	3			; System
	DB	5, 7, 2, 9
	DW	60Ch, TmpBuf+12, TmpBuf+12
	DB	3			; Hidden
	DB	6, 10, 3, 9
	DW	70Ch, TmpBuf+13, TmpBuf+13
	DB	1			; Date
	DB	7, 9, 5, 8
	DW	51Ch, TmpBuf+15, 00Ah
	DB	1			; Time
	DB	8, 10, 7, 9
	DW	71Ch, TmpBuf+30, 10Ah
	DB	4			; Set / Cancel
	DB	9, 0, 10, 10
	DW	91Eh, TmpBuf+8, SetCa_D
	DB	0
StAtr_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf
	DB	7, 64
	DW	Attr_S
	DW	StAtr_S
	DW	Var_S
NoDat_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 0
	DW	ErrBuf
	DW	Attr_S
	DW	NoDt1_S
	DW	NoDt2_S
	DW	Ok_S
NoDat_B	DW	NoDat_P, 0C28h, 0
	DB	1, 0FFh, 0, 0
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf+100, OkMn_D
	DB	0
AtrEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	TmpBuf
	DB	7, 64
	DW	Attr_S
	DW	Inc_S
	DW	Var_S
	DW	OkCan_S
AtrEr_B	DW	AtrEr_P, 928h, 0
	DB	1, 0FFh, 0, 2
	DB	4
	DB	0, 0, 0, 0
	DW	422h, TmpBuf+100, OkCan_D
	DB	0
SetCa_D	DB	 7,  0
	DB	10, 10
	DB	 0

AttrDat	EQU	WORD PTR TmpBuf[96]
AttrTim	EQU	WORD PTR TmpBuf[98]

	.CODE
M30_00:	MOV	AX,DS			; Attributes...
	CallC	M30_00_c
	JMP	F1Next

;			   AL - attributes of what is at TmpBuf
;	Return :	   AX - 1 set, 0 not, 100h to stop
M30_50	PROC	NEAR			; Set attributes
	CallC	M30_50_c
	RET
M30_50	ENDP

; Dialog calls this when the dialog of attributes is answered.
;			   AL - the answer
;	Return :	   AL - the answer to go by
;			 CY=1 - a date or a time was refused
M30_60	PROC	NEAR			; Test date & time
	PUSH	DX
	CallC	M30_60_c		; DX: not 0 if one was refused
	NEG	DX
	POP	DX
	RET
M30_60	ENDP

	.CONST
Lab0_S	DB	' Volume Label ',0
Lab1_S	DB	'Change volume label^V to',0
Lab2_S	DB	'Do you wish to delete volume label',0
Lab3_S	DB	'^V ?',0
ErVol_S	DB	'Could not set the volume label',0

	.DATA
Label_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 1
	DW	WinBuf
	DW	TmpBuf+20
	DB	0, 30
	DW	Lab0_S
	DW	Lab1_S
	DW	Empty_S
Label_B	DW	Label_P, 528h, 0
	DB	1, 3, 29, 0
	DB	1
	DB	0, 0, 0, 0
	DW	305h, TmpBuf+1, 20Bh
	DB	0
DelVo_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	Volume
	DB	0, 15
	DW	Lab0_S
	DW	Lab2_S
	DW	Lab3_S
	DW	DelCa_S
DelVo_B	DW	DelVo_P, 728h, 0
	DB	1, 0FFh, 0, 5
	DB	4
	DB	0, 0, 0, 0
	DW	41Fh, TmpBuf+100, DelCa_D
	DB	0
ErVol_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf
	DW	TmpBuf+1
	DB	6, 15
	DW	Lab0_S
	DW	ErVol_S
	DW	Var_S
	DW	Ok_S
ErVol_B	DW	ErVol_P, 728h, 0
	DB	1, 0FFh, 0, 2
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf+100, OkMn_D
	DB	0

	.CODE
F54_00:	CallC	F54_00_c		; Change label
	JMP	F1Next

	.CONST
PthEr_S	DB	'Path too long', 0

	.DATA
	ASSUME	CS:DGROUP
PthEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	TmpBuf
	DB	6, 64
	DW	Err_S
	DW	PthEr_S
	DW	Var_S
	DW	Ok_S
PthEr_B	DW	PthEr_P, 528h, 0
	DB	1, 0FFh, 0, 6
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf+200, OkMn_D
	DB	0

;	Return :	   AX - 1 all done, 0 not, 100h to stop
;			   DS - main data
DirFunc	PROC	NEAR			; Find files in directory
	PUSH	ES
	POP	DS
	CALL	FAR PTR DirFunc_c
	RET
DirFunc	ENDP

FncSel:	MOV	AL,0			; Selected files and directories
	MOV	ES:ZoomOne,AL
	MOV	ES:[TmpBuf+91],AL
	MOV	DS,ES:WCBcrn
	MOV	SI,DS:CurAdr
	CMP	ES:OneFile,0
	JNZ	FncSl5
	MOV	SI,OFFSET DskBuf
FncSl1:	CMP	SI,DS:EndBuf
	JAE	FncSl2
	AND	DS:[SI].AttrF,7Fh
	ADD	SI,SIZE FilEnt
	JMP	FncSl1
FncSl2:	CALL	TotSel
	MOV	SI,DS:CurAdr
	OR	BX,DX
	JE	FncSl5
	MOV	SI,OFFSET DskBuf
FncSl3:	CMP	SI,DS:EndBuf
	JB	FncSl4
	JMP	FncExt
FncSl4:	TEST	DS:[SI].AttrF,40h
	JNZ	FncSl5
	JMP	FncSl9
FncSl5:	PUSH	SI
	MOV	DI,OFFSET TmpBuf
	MOV	CX,13
	REP	MOVSB
	POP	SI
	MOV	AL,DS:[SI].AttrF
	CALL	ES:FncAddr
	OR	ES:[TmpBuf+91],AL
	MOV	DS,ES:WCBcrn
	CMP	ES:OneFile,0
	JNZ	FncExt
	PUSH	AX
	CMP	AL,0
	JZ	FncSl8
	TEST	DS:[SI].AttrF,40h
	JZ	FncSl8
	AND	DS:[SI].AttrF,3Fh
	OR	DS:[SI].AttrF,80h
	CMP	ES:UnSlct,0
	JZ	FncSl8
	CALL	PutFls
	CALL	LnsAX
	MOV	CX,AX
	MOV	AX,SI
	SUB	AX,DS:First
	MOV	BX,SIZE FilEnt
	XOR	DX,DX
	DIV	BX
	XOR	DX,DX
	DIV	CX
	XOR	CX,CX
	CMP	DS:BrfFul,CL
	JNE	FncSl6
	MOV	CL,2
FncSl6:	CMP	AX,CX
	JA	FncSl7
	CALL	BegBX
	ADD	BX,201h
	ADD	BH,DL
	MOV	DX,126h
	CALL	Window
FncSl7:	CMP	ES:MiniSta,0
	JZ	FncSl8
	CALL	BegBX
	CALL	SizDX
	ADD	BH,DH
	SUB	BH,2
	MOV	DX,128h
	CALL	Window
FncSl8:	POP	AX
	CALL	TstBrk
	JC	FncExt
	OR	AH,AH
	JNZ	FncExt
FncSl9:	ADD	SI,SIZE FilEnt
	JMP	FncSl3

FncExt:	CMP	BYTE PTR ES:[TmpBuf+91],0 ; Exit
	JE	FncEx2
FncEx1:	MOV	DS,ES:WCBcrn
	MOV	DS:ChgFlg,1
	CALL	InvWCB
	MOV	DS:ChgFlg,1
	JMP	Init50
FncEx2:	JMP	Func04

	.CONST
NoFnc_S	DB	"Can't find file^W",0

	.DATA
NoFnc_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 2
	DW	ErrBuf
	DW	TmpBuf
	DB	7, 64
	DW	TmpBuf+100
	DB	0, 50
	DW	Inc_S
	DW	NoFnc_S
	DW	Var_S
	DW	Ok_S
NoFnc_B	DW	NoFnc_P, 928h, 0
	DB	1, 0FFh, 0, 2
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpBuf+100, OkMn_D
	DB	0

	.CODE
; The files of a mask are gone through in C: FncMas_c of port/dirfunc.c.
;			   BX - a title for "Can't find file", or 0
FncMas:	MOV	AX,BX			; Masked files
	CallC	FncMas_c
	JMP	FncExt

;			ES:SI - a path on the drive
;	Return :	 CY=1 - error
;			   AX - what DOS answered, with CY or without:
;				^R in a tree goes on with it
DelTree	PROC	NEAR
	CMP	ES:ChgTree,0
	JZ	DelTr5
DelTr0:	PUSH	DX			; Whether the tree is read again
	MOV	AX,SI			; by itself or not
	CallC	DelTr0_c		; DX: not 0 for an error
	NEG	DX
	POP	DX
DelTr5:	RET
DelTree	ENDP
