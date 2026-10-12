;-------------------------------------------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;-------------- Read directory with save ---------------
;			    DS - WCBseg
;			AL = 0 - Without save
	.CONST
Mask_S	DB	'\*.*',0
Ext_F	DB	'VC.EXT',0

	.CODE
;			   ES - main data
;	Return :	CY=0 - what the panel shows has been read
ReDir	PROC	NEAR
	PUSH	AX
	MOV	DS:ChgFlg,1
	CMP	DS:WinTyp,0
	JNE	ReDr01
	PUSH	DX
	PUSH	BX
	SUB	SP,8			; struct OldList of port/read.c
	MOV	BX,SP
	MOV	DX,DS
	CallC	ReDir_c			; The files: read them,
	CallC	ReDr45_c		; filter and sort them,
	CallC	ReDr50_c		; and give them what the old had
	ADD	SP,8
	POP	BX
	POP	DX
	JMP	ReDr70
ReDr01:	CMP	DS:WinTyp,1
	JNE	ReDr02
	CALL	ReadTre			; The tree
	JC	ReDr70
	MOV	DS:ChgFlg,0
	JMP	ReDr70
ReDr02:	MOV	AX,DS
	CallC	RdInfo_c		; The Info
ReDr70:	CMP	DS:ChgFlg,1		; Exit
	CMC
	POP	AX
	RET
ReDir	ENDP

	.CONST
Tree_F	DB	'\TREEINFO.NCD',0
Scan1_S	DB	'Scanning the directory',0
Scan2_S	DB	'structure on drive ^V:',0
Scan3_S	DB	'0',0
Scan4_S	DB	'Saving the',0
Scan5_S	DB	'directory information...',0

	.DATA
Tree_P	DB	Box_C, 0, 0, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	OFFSET MenuBuf
	DB	0, 25
	DW	OFFSET Tree_S
	DW	OFFSET Scan1_S
	DW	OFFSET Scan2_S
	DW	OFFSET Scan3_S
TrSav_P	DB	Box_C, 0, 0, 0
	DB	3, 0
	DW	OFFSET ErrBuf
	DW	OFFSET Tree_S
	DW	OFFSET Scan4_S
	DW	OFFSET Scan5_S

	.CODE
; Read the tree of the current disk, from its file or from the disk
;			   ES - main data
;	Return :	CY=1 - the drive is not ready any more
ReadTre	PROC	NEAR
	PUSH	AX
	CLD
	CallC	ReadTre_c
	NEG	AX			; CY for 1
	POP	AX
	RET
ReadTre	ENDP

;	Put the cursor of the tree on the current directory
;			   ES - main data
TreeCur	PROC	NEAR
	CallC	TreeCur_c
	RET
TreeCur	ENDP

;	Begin the lines of the tree half a panel above the cursor
;			   AL - lines shown
;			   DS - main data
TrCr20	PROC	NEAR
	CALL	FAR PTR TrCr20_c
	RET
TrCr20	ENDP

	.CONST
DrIn_F	DB	'DIRINFO',0

	.CODE

;---------------------- Output tree --------------------
;			   DS - WCBseg
;			BH,BL - begin
;			DH,DL - size
;			CS:BP - address of color table
PutTree	PROC	NEAR
	PUSH	AX
	PUSH	DS
	PUSH	ES
	MOV	AX,DS
	PUSH	ES
	POP	DS
	PUSH	CS			; The colours are in this segment
	POP	ES
	XCHG	SI,BP
	CALL	FAR PTR PutTree_c
	XCHG	SI,BP
	POP	ES
	POP	DS
	POP	AX
	RET
PutTree	ENDP

;--------------------- Find near file ------------------
;			ES:DI - File
;			   BX - End
;			   DS - WCBseg
;	Return :	   SI - Address of file , CF=1
;				Not found ,       CF=0
NearFl	PROC	NEAR
	PUSH	AX
	PUSH	DX
	MOV	DX,DS
	CALL	FAR PTR NearFl_c
	POP	DX
	CMP	AX,1
	CMC			; CY if one was found
	JNC	NrFl01
	MOV	SI,AX
NrFl01:	POP	AX
	RET
NearFl	ENDP

;---------------------- Save setup ---------------------
	.CONST
Init_F	DB	'VC.INI',0

	.CODE
SaveSet	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	CALL	MakeIni
	MOV	BX,CX
	MOV	DX,OFFSET TmpBuf
	MOV	SI,OFFSET Init_F
	MOV	DI,DX
	CALL	MainFil
	CALL	ChgDisk
	JC	SavSt2
	MOV	AX,4300h
	CALL	Intr21
	JNC	SavSt1
	CMP	AX,WORD PTR 2
	JNE	SavSt3
	XOR	CX,CX
SavSt1:	OR	CL,20h
	MOV	AH,3Ch
	CALL	Intr21
	JC	SavSt4
	MOV	CX,BX
	MOV	BX,AX
	MOV	DX,OFFSET MenuBuf
	MOV	AH,40h
	CALL	Intr21
	PUSHF
	PUSH	AX
	MOV	AH,3Eh
	CALL	Intr21
	POP	BX
	POP	DX
	JC	SavSt4
	MOV	AX,BX
	PUSH	DX
	POPF
	JC	SavSt4
	CMP	AX,CX
	JE	SavSt4
SavSt2:	MOV	AL,0
SavSt3:	STC
SavSt4:	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
SaveSet	ENDP

MakeIni	PROC	NEAR
	PUSH	DI
	PUSH	AX
	MOV	DI,OFFSET MenuBuf
	PUSH	DI
	MOV	AL,'V'
	STOSB
	STOSB
	STOSB
	MOV	DS,ES:WCB1seg
	CALL	MkIni1
	MOV	DS,ES:WCB2seg
	CALL	MkIni1
	PUSH	ES
	POP	DS
	MOV	SI,0
	MOV	CX,CmnIni
	REP	MOVSB
	POP	SI
	MOV	CX,DI
	SUB	CX,SI
	CALL	ChkSum
	STOSW
	INC	CX
	INC	CX
	POP	AX
	POP	DI
	RET
MakeIni	ENDP

MkIni1	PROC	NEAR
	MOV	SI,OFFSET DskMas
	MOV	CX,13
	CALL	FilZero
	MOV	SI,OFFSET WinPath
	MOV	CX,LenPath
	CALL	FilZero
	XOR	SI,SI
	MOV	CX,WCBini
	REP	MOVSB
	RET
MkIni1	ENDP

;-------------------- Fantom Diskette ------------------
;			AL - Disk ID
	.CONST
Fant0_S	DB	' Change drive ',0
Fant1_S	DB	'Insert diskette for drive ^V:',0
OkCan_S	DB	' Ok',0FFh,3,'Cancel',0

	.DATA
Fant_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	3, 1
	DW	OFFSET ErrBuf1
	DW	OFFSET TmpErr+1
	DB	0, 25
	DW	OFFSET Fant0_S
	DW	OFFSET Fant1_S
	DW	OFFSET OkCan_S
Fant_B	DW	OFFSET Fant_P, 928h, 0
	DB	2, 0FFh, 0FFh, 5
	DB	4
	DB	0, 0, 0, 0
	DW	322h, OFFSET TmpErr, OFFSET OkCan_D
	DB	0
OkCan_D	DB	4, 0
	DB	8, 5
	DB	0

	.CODE
; C: Fantom_c of port/system.c, which returns 1 if the user would not
; change the disk. It has Dialog show the box here.
	PUBLIC	Fant_B

; The disk of the drive that the path at ES:DX names
;	Return :	CY=1 - the user would not change it
; C: ChgDisk_c of port/system.c, which returns 1 for that.
ChgDisk	PROC	NEAR
	PUSH	AX
	CallC	ChgDisk_c
	NEG	AX			; CY for 1
	POP	AX
	RET
ChgDisk	ENDP

; Have a directory chosen from the tree, in a box
;			   AL - the column the box begins in
;			   ES - main data
;	Return :	CY=0 - chosen, its path at TmpBuf
ChTree	PROC	NEAR
	PUSH	AX
	CallC	ChTree_c
	NEG	AX			; CY for 1
	POP	AX
	RET
ChTree	ENDP

;	Work out the sizes of the directories of a panel
;			   DS - WCBseg
;			   ES - main data
DirSize	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CallC	DirSize_c
	POP	AX
	RET
DirSize	ENDP

; For DirSize and DirFunc in C: where DiSi10 is, to put into FncAddr, and
; a call of what FncAddr names, with the attributes of a file in AL. Not
; every such routine leaves DS as it was.
	PUBLIC	DiSi10_o, FncCall_a
DiSi10_o PROC	FAR
	MOV	AX,OFFSET DiSi10
	RET
DiSi10_o ENDP

FncCall_a PROC	FAR
	PUSH	BP
	PUSH	DS
	PUSH	DS
	POP	ES
	CALL	ES:FncAddr
	POP	DS
	POP	BP
	RET
FncCall_a ENDP

DiSi10	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	TEST	AL,10h
	JZ	DiSi11
	CALL	DirFunc
	JMP	DiSi13
DiSi11:	MOV	AX,ES:DTA.FilSize.Lo
	MOV	BX,ES:DTA.FilSize.Hi
	ADD	AX,WORD PTR ES:[TmpBuf+90]
	ADC	BX,WORD PTR ES:[TmpBuf+92]
	JNC	DiSi12
	MOV	AX,0FFFFh
	MOV	BX,0FFFFh
DiSi12:	MOV	WORD PTR ES:[TmpBuf+90],AX
	MOV	WORD PTR ES:[TmpBuf+92],BX
	XOR	AX,AX
DiSi13:	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
DiSi10	ENDP

;	Get the label of the current drive
;			ES:SI - where to put it
;	Return :	   CY - 0 for a drive of a network
;			   AX - with CY, see GetVol in port/read.c
GetVol	PROC	NEAR
	PUSH	DX
	MOV	AX,SI
	CallC	GetVol_c
	NEG	DX
	POP	DX
	RET
GetVol	ENDP

