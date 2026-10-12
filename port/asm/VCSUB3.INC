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

; C code asks whether a key is waiting that stops what is being done,
; adds up bytes of the main data, and puts a box on the screen that
; grows only the first time: OncZoom for the box at CS:AX, which puts
; where the box is, BX, DX and SI, into the three words at SS:DI.
	FromC	TstBrk, <SBB AX,AX>	; CY as a number
	FromC	ChkSum
	PUBLIC	OncZoom_a
OncZoom_a PROC	FAR
	PUSH	BP
	PUSH	DS
	POP	ES
	MOV	BP,AX
	CALL	OncZoom
	MOV	BP,DI
	MOV	[BP],BX
	MOV	[BP+2],DX
	MOV	[BP+4],SI
	POP	BP
	RET
OncZoom_a ENDP

;	Put the cursor of the tree on the current directory
;			   ES - main data
TreeCur	PROC	NEAR
	CallC	TreeCur_c
	RET
TreeCur	ENDP

;	The same, without regard to where it was
;			DS=ES - main data
TrCr10	PROC	NEAR
	CALL	FAR PTR TrCr10_c
	RET
TrCr10	ENDP

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
;---------------------- Output files -------------------
;			   DS - WCBseg
PutFls	PROC	NEAR
	PUSH	AX
	MOV	AX,DS
	CMP	DS:WinTyp,0
	JNE	PtFs01
	CallC	PutFls_c
	POP	AX
	RET
PtFs01:	CMP	DS:WinTyp,1
	JNE	PtFs02
	CallC	PutTre_c		; The tree
	POP	AX
	RET
PtFs02:	CallC	PutInf_c		; The Info
	POP	AX
	RET
PutFls	ENDP

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

;----------------------- Set Path ----------------------
;	Return :	CY=1 - Error
SetPth	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	AX
	PUSH	ES
	POP	DS
	MOV	DS:PathEr,1
	MOV	AH,19h
	INT	21h
	MOV	DL,AL
	MOV	DI,OFFSET DosPth
	ADD	AL,'A'
	STOSB
	MOV	AX,'\:'
	STOSW
	MOV	SI,DI
	MOV	AL,0
	STOSB
	INC	DX
	MOV	AL,DL
	CALL	Fantom
	JC	SetPt1
	MOV	AH,47h
	CALL	Intr21
	JC	SetPt1
	MOV	DS:PathEr,0
SetPt1:	POP	AX
	POP	DX
	POP	DI
	POP	SI
	POP	DS
	RET
SetPth	ENDP

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
Fantom	PROC	NEAR
	PUSH	DS
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	PUSH	ES
	POP	DS
	MOV	BL,AL
	MOV	AX,440Eh
	INT	21h
	JC	Fantm1
	CMP	AL,0
	JE	Fantm1
	CMP	AL,BL
	JE	Fantm1
	MOV	BH,AL
	MOV	AX,440Fh
	INT	21h
	JC	Fantm1
	CMP	AL,BH
	JE	Fantm1
	XCHG	BL,BH
	MOV	AX,440Fh
	INT	21h
	JC	Fantm1
	XCHG	BL,BH
	MOV	AL,BL
	CBW
	ADD	AL,'A'-1
	MOV	WORD PTR DS:[TmpErr+1],AX
	MOV	CL,DS:HlpPage
	MOV	BP,OFFSET Fant_B
	CALL	Dialog
	MOV	DS:HlpPage,CL
	STC
	JNE	Fantm2
	MOV	AX,440Fh
	INT	21h
Fantm1:	CLC
Fantm2:	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	DS
	RET
Fantom	ENDP
	FromC	Fantom, <SBB AX,AX>	; CY as a number

ChgDisk	PROC	NEAR
	PUSH	BX
	PUSH	AX
	MOV	BX,DX
	MOV	AX,ES:[BX]
	CMP	AH,':'
	JE	ChDsk1
	MOV	AL,ES:[DosPth]
ChDsk1:	CALL	UpCase
	SUB	AL,'A'-1
	CALL	Fantom
	POP	AX
	POP	BX
	RET
ChgDisk	ENDP
	FromC	ChgDisk, <SBB AX,AX>	; CY as a number

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

; C code waits for a key, writes a text of the main data into a field,
; and spoils the tree file of a disk.
	FromC	Input
	FromC	InSt10
	FromC	DelTr0

; C code has a key looked at for the speed search: Search, which takes in
; CX what it has in BP, and hands back SI.
	PUBLIC	Search_a
Search_a PROC	FAR
	PUSH	BP
	PUSH	DS
	POP	ES
	MOV	BP,CX
	CALL	Search
	MOV	AX,SI
	POP	BP
	RET
Search_a ENDP

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

; For DirSize in C: where DiSi10 is, to put into FncAddr, and a call of
; what FncAddr names, with the attributes of a file in AL.
	PUBLIC	DiSi10_o, FncCall_a
DiSi10_o PROC	FAR
	MOV	AX,OFFSET DiSi10
	RET
DiSi10_o ENDP

FncCall_a PROC	FAR
	PUSH	BP
	PUSH	DS
	POP	ES
	CALL	ES:FncAddr
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

OpenFil	PROC	NEAR
	PUSH	BX
	MOV	AH,3Dh
	MOV	BX,AX
	CALL	Intr21
	JNC	OpnFl1
	CMP	AX,WORD PTR 20h
	STC
	JNE	OpnFl1
	MOV	AX,BX
	AND	AL,0Fh
	CALL	Intr21
OpnFl1:	POP	BX
	RET
OpenFil	ENDP

; C code opens a file: AX as OpenFil has it, DX:BX - the name. DX is not 0
; afterwards if the carry flag was set.
	PUBLIC	OpenFil_a
OpenFil_a PROC	FAR
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	DS,DX
	MOV	DX,BX
	CALL	OpenFil
	POP	DS
	SBB	DX,DX
	RET
OpenFil_a ENDP
