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

TreeErr	EQU	BYTE PTR TreHead[0]
TreeLev	EQU	BYTE PTR TreHead[1]
DirRecu	EQU	BYTE PTR DirName[12]

	.CONST
Tree_F	DB	'\TREEINFO.NCD',0
TreBeg	DB	'PNCI',0
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
ReadTre	PROC	NEAR
	PUSH	DS
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	CLD
	PUSH	ES
	POP	DS

	MOV	BP,OFFSET TreeBuf+MinTree*SIZE DIRENT+SIZE DIRTREE ; ??? Expand

	MOV	AL,DS:DosPth
	SUB	AL,'A'-1
	CALL	Fantom
	JC	RdTr01
	PUSH	DS			; Reading TREEINFO
	PUSH	CS
	POP	DS
	MOV	DX,OFFSET Tree_F
	MOV	AL,40h
	CALL	OpenFil
	POP	DS
	JNC	RdTr02
	CMP	AX,WORD PTR 53h
	JE	$+5			; JNE RdTr10
	JMP	NEAR PTR RdTr10
RdTr01:	MOV	DI,OFFSET TreeBuf.TreList
	PUSH	DI
	MOV	CX,SIZE DIRENT
	MOV	AL,0
	REP	STOSB
	POP	DI
	MOV	DS:[DI].DirName,'\'
	MOV	DS:[DI].DirShift,1
	MOV	DS:TreeBuf.TreeLen,1
	MOV	DS:TreeBuf.TreeErr,0
	JMP	RdTr40
RdTr02:	MOV	BX,AX
	MOV	DX,OFFSET TreeBuf
	MOV	CX,BP
	SUB	CX,DX
	MOV	AH,3Fh
	CALL	Intr21
	PUSHF
	PUSH	AX
	MOV	AH,3Eh
	CALL	Intr21
	POP	AX
	POP	SI
	JC	RdTr01
	PUSH	SI
	POPF
	JC	RdTr01
	CMP	AX,CX			; TreeBuf overflow ???
	JAE	RdTr10
	PUSH	DS
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET TreBeg
	MOV	DI,OFFSET TreeBuf.TreHead
	MOV	CX,5
	PUSH	DI
	REPE	CMPSB
	POP	SI
	POP	DS
	JNE	RdTr10
	MOV	BX,DS:[DI]
	MOV	CL,7
	PUSH	AX
	CALL	ChkSum
	CMP	AX,DS:[DI+2]
	POP	AX
	JNE	RdTr10
	MOV	CL,4
	SHL	BX,CL
	SUB	AX,WORD PTR 11
	CMP	AX,BX
	JNE	RdTr10
	MOV	CX,BX
	MOV	SI,OFFSET TreeBuf.TreList
	CALL	ChkSum
	CMP	AX,DS:[SI+BX]
	JNE	RdTr10
	JMP	RdTr40
RdTr10:	MOV	DI,OFFSET TreeBuf	; Scanning tree
	MOV	CX,BP
	SUB	CX,DI
	MOV	AL,0
	REP	STOSB
	MOV	DS:ZoomOne,AL
	PUSH	CS
	POP	DS
	MOV	DI,OFFSET MenuBuf+40
	MOV	SI,OFFSET Mask_S
	MOV	CX,5
	REP	MOVSB
	PUSH	ES
	POP	DS
	MOV	DS:TreeBuf.TreList.DirName,'\'
	MOV	DS:TreeBuf.TreList.DirShift,1
	MOV	DI,OFFSET TreeBuf.TreList+SIZE DIRENT
RdTr11:	INC	DS:TreeBuf.TreeLen
	MOV	CX,DS:TreeBuf.TreeLen
	CMP	CX,5
	JB	RdTr13
	JA	RdTr12
	PUSH	BP
	MOV	AL,DS:[DosPth]
	CBW
	MOV	WORD PTR DS:[MenuBuf],AX
	MOV	BX,728h
	MOV	BP,OFFSET Tree_P
	CALL	OncZoom
	POP	BP
RdTr12:	PUSH	DI
	PUSH	BX
	XOR	BX,BX
	MOV	SI,OFFSET MenuBuf
	MOV	DI,SI
	MOV	AL,1
	CALL	Number
	POP	BX
	PUSH	BP
	PUSH	BX
	ADD	BH,4
	MOV	BL,28h
	MOV	BP,OFFSET Tree_P
	CALL	CntLine
	POP	BX
	POP	BP
	PUSH	DX
	PUSH	BX
	ADD	BH,4
	MOV	DH,1
	CALL	Window
	POP	BX
	POP	DX
	POP	DI
RdTr13:	PUSH	DX
	PUSH	BX
	CMP	DS:TreeBuf.TreeLen,1
	JE	RdTr15
RdTr14:	MOV	AH,4Fh
	JMP	RdTr16
RdTr15:	CALL	SetDTA
	MOV	DX,OFFSET MenuBuf+40
	MOV	CX,37h
	MOV	AH,4Eh
RdTr16:	CALL	TstBrk
	MOV	AL,2
	JC	RdTr18
	CALL	Intr21
	JC	RdTr17
	TEST	DS:DTA.FilAttr,10h
	JZ	RdTr14
	CMP	DS:HidDirs,0
	JNZ	RdTr29
	TEST	DS:DTA.FilAttr,2
	JNZ	RdTr14
RdTr29:	MOV	SI,OFFSET DTA.FilName
	CMP	BYTE PTR DS:[SI],'.'
	JE	RdTr14
	LEA	AX,[BP-(OFFSET TreeBuf+SIZE DirTree)-SIZE DirEnt]
	MOV	CL,4
	SHR	AX,CL
	CMP	AX,DS:TreeBuf.TreeLen
	JA	RdTr20
	CALL	Beep
	MOV	AL,0
	JMP	RdTr18
RdTr17:	CMP	AX,WORD PTR 53h
	JNE	RdTr22
	MOV	AL,1
RdTr18:	MOV	DS:TreeBuf.TreeErr,AL
	MOV	SI,OFFSET TreeBuf.TreList.DirRecu
RdTr19:	MOV	BYTE PTR DS:[SI],0
	ADD	SI,16
	CMP	SI,BP
	JB	RdTr19
	JMP	RdTr26
RdTr20:	LEA	CX,[BP]-SIZE DIRENT
	SUB	CX,DI
	JBE	RdTr80
	PUSH	SI
	PUSH	DI
	LEA	DI,[BP]-1
	LEA	SI,[DI]-SIZE DIRENT
	STD
	REP	MOVSB
	CLD
	POP	DI
	POP	SI
RdTr80:	MOV	CX,12
	PUSH	DI
	PUSH	CX
	REP	MOVSB
	POP	CX
	POP	SI
	CALL	FilZero
	MOV	AL,0FFh
	STOSB
	MOV	AL,DS:TreeBuf.TreeLev
	INC	AX
	STOSB
	INC	DI
	INC	DI
	CMP	AL,DS:[DI-2*SIZE DIRENT].DirLevel
	MOV	AL,1
	JNE	RdTr21
	MOV	DS:[DI-2*SIZE DIRENT].DirCont,1
	MOV	AL,0
RdTr21:	MOV	DS:[DI-SIZE DIRENT].DirShift,AL
	MOV	DS:[DI-SIZE DIRENT].DirCont,0
	POP	BX
	POP	DX
	JMP	RdTr11
RdTr22:	MOV	AL,0
	CMP	AL,DS:[DI-SIZE DIRENT].DirRecu
	JNE	RdTr23
	CMP	AL,DS:[DI].DirRecu
	JNE	RdTr24
	JMP	RdTr26
RdTr23:	SUB	DI,16
	CMP	AL,DS:[DI-SIZE DIRENT].DirRecu
	JNE	RdTr23
RdTr24:	MOV	DS:[DI].DirRecu,AL
	MOV	CL,DS:[DI].DirLevel
	MOV	DS:TreeBuf.TreeLev,CL
	MOV	CH,0
	PUSH	DI
	MOV	DI,OFFSET MenuBuf+40
RdTr25:	PUSH	CX
	MOV	CX,20
	MOV	AL,'\'
	REPNE	SCASB
	POP	CX
	LOOP	RdTr25
	POP	SI
	PUSH	SI
	MOV	CX,13
	CALL	MvzLine
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET Mask_S
	MOVSB
	MOV	AX,DI
	MOV	CX,4
	REP	MOVSB
	PUSH	ES
	POP	DS
	POP	DI
	ADD	DI,16
	CMP	AX,OFFSET MenuBuf+40+LenPath
	JA	RdTr22
	JMP	RdTr15
RdTr26:	POP	BX
	POP	DX
	CMP	DS:TreeBuf.TreeLen,5
	JB	RdTr30
	MOV	SI,OFFSET ErrBuf
	CALL	ResWin
	CALL	Window
RdTr30:	CMP	DS:TreeBuf.TreeErr,0	; Delete TREEINFO
	JNE	RdTr31
	CMP	ES:TreeBuf.TreeLen,5
	JAE	RdTr32
	PUSH	DS
	PUSH	CS
	POP	DS
	MOV	DX,OFFSET Tree_F
	MOV	AH,41h
	CALL	Intr21
	POP	DS
RdTr31:	JMP	RdTr40
RdTr32:	CALL	TstBrk			; Save TREEINFO
	JC	RdTr31
	MOV	BX,728h
	MOV	BP,OFFSET TrSav_P
	CALL	OncZoom
	PUSH	SI
	PUSH	DX
	PUSH	BX
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET TreBeg
	MOV	DI,OFFSET TreeBuf.TreHead
	MOV	CX,5
	PUSH	DI
	REP	MOVSB
	PUSH	ES
	POP	DS
	POP	SI
	MOV	BX,DS:[DI]
	INC	DI
	INC	DI
	MOV	CL,7
	CALL	ChkSum
	STOSW
	MOV	CL,4
	SHL	BX,CL
	MOV	CX,BX
	MOV	SI,DI
	CALL	ChkSum
	MOV	DS:[SI+BX],AX
	MOV	BX,CX
	ADD	BX,11
	PUSH	CS
	POP	DS
	MOV	DX,OFFSET Tree_F
	MOV	AX,4300h
	CALL	Intr21
	JNC	RdTr34
	CMP	AX,WORD PTR 2
	JNE	RdTr37
	XOR	CX,CX
RdTr34:	OR	CL,20h
	MOV	ES:BlocEr,1
	MOV	AH,3Ch
	CALL	Intr21
	JNC	RdTr36
	CMP	AX,WORD PTR 5
	JNE	RdTr37
RdTr35:	CALL	Beep
	JMP	RdTr37
RdTr36:	PUSH	ES
	POP	DS
	MOV	CX,BX
	MOV	BX,AX
	MOV	DX,OFFSET TreeBuf
	MOV	AH,40h
	CALL	Intr21
	PUSHF
	PUSH	AX
	MOV	AH,3Eh
	CALL	Intr21
	POP	AX
	POP	BX
	JC	RdTr37
	PUSH	BX
	POPF
	JC	RdTr37
	CMP	AX,CX
	JNE	RdTr35
RdTr37:	MOV	ES:BlocEr,0
	POP	BX
	POP	DX
	POP	SI
	CALL	ResWin
	CALL	Window
RdTr40:	CALL	TreeCur			; Set cursor
	MOV	AL,ES:TreeBuf.TreeErr
	DEC	AX
	CMP	AL,1
					; ??? Compress
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	DS
	RET
ReadTre	ENDP

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

	.CONST
ChTr0_S	DB	' Choose Directory ',0
ChTr1_S	DB	'Reading directory tree...',0
	DB	'Speed search: ^r',0FFh,12,0

	.DATA
ChTr_P	DB	Conf_C, CnfRe_C, CnfBo_C, CnfUn_C
ChTr_T	DW	4800h, ChTr35
	DW	5000h, ChTr37
	DW	4900h, ChTr40
	DW	5100h, ChTr42
	DW	4700h, ChTr30
	DW	4F00h, ChTr32
	DW	000Dh, ChTr45
	DW	001Bh, ChTr46
	DW	4400h, ChTr46
	DW	3C00h, ChTr55
	DW	0012h, ChTr55
	DW	0

	.CODE
ChTree	PROC	NEAR
	PUSH	DS
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	CALL	HidCur
	MOV	BL,AL
	MOV	BH,2
	MOV	DL,50
	MOV	DH,ES:Lines
	SUB	DH,3
	MOV	SI,OFFSET WinBuf
	CALL	SavWin
	MOV	BP,OFFSET ChTr_P
	MOV	AH,CS:[BP]
	CALL	Color
	CALL	MinBox
	PUSH	SI
	MOV	DS,ES:WCB1seg
	CMP	DS:WinTyp,1
	JNE	ChTr01
	MOV	DS:ChgFlg,1
ChTr01:	MOV	DS,ES:WCB2seg
	CMP	DS:WinTyp,1
	JNE	ChTr02
	MOV	DS:ChgFlg,1
ChTr02:	PUSH	ES
	POP	DS
	MOV	SI,OFFSET CrnPath
	MOV	DI,OFFSET MenuBuf+250
	MOV	CX,LenPath
	REP	MOVSB
ChTr59:	PUSH	BX
	PUSH	CS
	POP	DS
	ADD	BX,118h
	MOV	SI,OFFSET ChTr0_S
	CALL	CntLine
	POP	BX
	PUSH	DX
	PUSH	BX
	ADD	BH,DH
	SUB	BH,6
	CALL	BasAdr
	MOV	CX,3
	SUB	DL,10
	MOV	DH,0
	MOV	BX,'Ç¶'
	MOV	AL,'Ä'
	CALL	BoxLine
	MOV	SI,OFFSET ChTr1_S
	ADD	DI,2*5
	MOV	AL,38
	PUSH	DI
	CALL	NgLine
	POP	DI
	ADD	DI,2*80
	CALL	NgLine
	POP	BX
	POP	DX
	CALL	Window
	PUSH	ES
	POP	DS
	MOV	DS:CrnPath,0
	CALL	ReadTre
	MOV	AL,DH
	SUB	AL,8
	CALL	TrCr20
	MOV	SI,DS:TreeBuf.TreeLen
	MOV	AL,DH
	SUB	AL,8
	CBW
	SUB	SI,AX
	JAE	ChTr03
	XOR	SI,SI
ChTr03:	PUSH	DX
	CALL	NumAd1
	POP	DX
	CMP	SI,DS:TreFrst
	JAE	ChTr04
	MOV	DS:TreFrst,SI
ChTr04:	MOV	DS:MousTim.Hi,0FFFFh
	MOV	DS:MousEnt,0
	MOV	BP,DS:TreAdr
	MOV	DI,DS:TreFrst
	ADD	BP,2*16
	ADD	DI,2*16
	XCHG	BP,DS:TreAdr
	XCHG	DI,DS:TreFrst
	JMP	ChTr11

ChTr10:	PUSH	DX			; Convert adresses
	MOV	SI,BP
	CALL	NumAd1
	MOV	BP,SI
	MOV	SI,DI
	CALL	NumAd1
	MOV	DI,SI
	POP	DX
ChTr11:	CMP	BP,DS:TreAdr		; Clear search line
	JE	ChTr12
	XOR	AX,AX
	MOV	WORD PTR DS:[MenuBuf+228],AX
	MOV	DS:[MenuBuf+230],AL
ChTr12:	CMP	BP,DI			; Test parameters
	JAE	ChTr13
	MOV	DI,BP
ChTr13:	MOV	AL,DH
	SUB	AL,9
	MOV	AH,16
	MUL	AH
	MOV	SI,BP
	SUB	SI,DI
	CMP	SI,AX
	JBE	ChTr14
	MOV	DI,BP
	SUB	DI,AX
ChTr14:	MOV	CX,DI
	XCHG	BP,DS:TreAdr
	XCHG	DI,DS:TreFrst
	SUB	CX,DI
	JNE	ChTr15
	CMP	BP,DS:TreAdr
	JNE	ChTr15
	JMP	ChTr20
ChTr15:	PUSH	BP			; Put lines
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	MOV	BP,OFFSET ChTr_P
	CALL	BasAdr
	ADD	DI,2*165
	MOV	CL,DH
	SUB	CL,8
	MOV	CH,0
	MOV	AH,CS:[BP]
	CALL	Color
	MOV	AL,' '
ChTr16:	PUSH	DI
	PUSH	CX
	MOV	CX,38
	REP	STOSW
	POP	CX
	POP	DI
	ADD	DI,2*80
	LOOP	ChTr16
	ADD	BX,104h
	SUB	DX,40Ah
	MOV	DS,ES:WCBcrn
	CALL	PutTree
	PUSH	ES
	POP	DS
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	BP
	JCXZ	ChTr18			; Output window
	PUSH	DX
	PUSH	BX
	ADD	BX,205h
	SUB	DX,80Ch
	MOV	AL,DH
	DEC	AX
	MOV	AH,6
	CMP	CX,16
	JE	ChTr17
	MOV	AX,700h
	CMP	CX,-16
	JE	ChTr17
	CALL	Window
	POP	BX
	POP	DX
	JMP	ChTr19
ChTr17:	NEG	CX
	ADD	BP,CX
	MOV	CX,BX
	ADD	DX,CX
	SUB	DX,101h
	PUSH	AX
	MOV	AH,CS:ChTr_P
	CALL	Color
	MOV	BH,AH
	POP	AX
	PUSH	AX
	MOV	AL,1
	CALL	Int10m
	POP	AX
	POP	BX
	POP	DX
	CBW
	MOV	CL,4
	SHL	AX,CL
	CALL	ChTr50
ChTr18:	MOV	AX,BP
	SUB	AX,DI
	CALL	ChTr50
	MOV	AX,DS:TreAdr
	SUB	AX,DS:TreFrst
	CALL	ChTr50
ChTr19:	PUSH	DX			; Output mini status
	PUSH	BX
	ADD	BH,DH
	SUB	BH,5
	ADD	BL,5
	MOV	DX,126h
	CALL	Window
	POP	BX
	POP	DX
ChTr20:	PUSH	DX			; Output search line
	PUSH	BX
	ADD	BH,DH
	SUB	BH,4
	ADD	BL,19
	CMP	DS:WinMous,0
	JNZ	ChTr21
	MOV	CX,12
	MOV	SI,OFFSET MenuBuf+230
	MOV	AH,CS:ChTr_P+1
	CALL	InSt10
ChTr21:	MOV	AX,BX
	ADD	AX,WORD PTR DS:[MenuBuf+228]
	CALL	Cursor
	MOV	AX,DS:TreAdr		; Input
	CALL	TreNum
	MOV	BP,AX
	MOV	AX,DS:TreFrst
	CALL	TreNum
	MOV	DI,AX
	POP	BX
	POP	DX
	CALL	GetMous
	MOV	CX,DS:TreeBuf.TreeLen
	JNZ	ChTr22
	MOV	DS:MousEnt,0
	MOV	DS:WinMous,0
	MOV	AL,14
	CALL	Input
	CMP	AH,0FCh
	JE	ChTr22
	MOV	SI,OFFSET ChTr_T
	CALL	Case

	PUSH	DX			; Letters
	PUSH	BX
	MOV	BP,CX
	MOV	CL,4
	SHL	BP,CL
	MOV	BX,OFFSET TreeBuf.TreList
	ADD	BP,BX
	MOV	DX,16
	MOV	SI,DS:TreAdr
	CALL	Search
	POP	BX
	POP	DX
	JZ	ChTr20
	MOV	BP,SI
	MOV	DI,DS:TreFrst
	JMP	ChTr12
ChTr22:	JMP	ChTr60

ChTr30:	XOR	BP,BP			; Home
	JMP	ChTr10

ChTr32:	MOV	BP,DS:TreeBuf.TreeLen	; End

ChTr35:	OR	BP,BP			; Up
	JE	ChTr38
	DEC	BP
	JMP	ChTr10

ChTr37:	INC	BP			; Down
	CMP	BP,DS:TreeBuf.TreeLen
	JAE	ChTr38
	JMP	ChTr10
ChTr38:	JMP	ChTr20

ChTr40:	MOV	AL,DH			; PgUp
	SUB	AL,8
	CBW
	MOV	SI,BP
	CALL	PgUp
	MOV	BP,SI
	JMP	ChTr10

ChTr42:	MOV	AL,DH			; PgDn
	SUB	AL,8
	CBW
	MOV	SI,BP
	MOV	BP,DS:TreeBuf.TreeLen
	CALL	PgDn
	MOV	BP,SI
	JMP	ChTr10

ChTr55:	PUSH	DX			; Rescan
	PUSH	BX
	ADD	BX,205h
	SUB	DX,80Ch
	MOV	CL,DH
	MOV	CH,0
	MOV	BP,OFFSET ChTr_P
	MOV	AH,CS:[BP]
	CALL	Color
	MOV	AL,' '
ChTr56:	PUSH	CX
	CALL	BasAdr
	MOV	CL,DL
	REP	STOSW
	INC	BH
	POP	CX
	LOOP	ChTr56
	PUSH	AX
	CALL	DelTr0
	POP	AX
	POP	BX
	POP	DX
	JMP	ChTr59

ChTr60:	XOR	SI,SI			; Mouse
ChTr61:	CALL	GetMous
	JZ	ChTr65
	MOV	BP,CX
	CALL	ClrKbd
	MOV	AH,'x'
	TEST	AL,3
	JZ	ChTr62
	MOV	AH,0
ChTr62:	MOV	CX,SI
	MOV	SI,AX
	CMP	AH,CH
	JE	ChTr63
	MOV	AL,AH
	CALL	TypMous
ChTr63:	TEST	SI,3
	JZ	ChTr61
	PUSH	BX
	PUSH	DX
	ADD	BX,204h
	SUB	DX,80Ah
	MOV	CX,BP
	CALL	InWind
	POP	DX
	POP	BX
	JNC	ChTr67
	CMP	DS:WinMous,0
	JZ	ChTr61
	MOV	BP,DI
	MOV	AL,BH
	ADD	AL,2
	CMP	CH,AL
	JAE	ChTr64
	JMP	ChTr35
ChTr64:	MOV	AL,BH
	ADD	AL,DH
	SUB	AL,6
	CMP	CH,AL
	JB	ChTr61
	MOV	AL,DH
	SUB	AL,9
	CBW
	ADD	BP,AX
	JMP	ChTr37
ChTr65:	MOV	AL,0
	CALL	TypMous
	MOV	AX,SI
	CMP	AL,2
	JE	ChTr66
	CMP	AL,1
	JB	ChTr66
	JNE	ChTr46
	CMP	DS:WinMous,0
	JNZ	ChTr66
	PUSH	DX
	PUSH	CX
	MOV	CX,BP
	SUB	DX,102h
	CALL	InWind
	POP	CX
	POP	DX
	JC	ChTr46
ChTr66:	JMP	ChTr20
ChTr67:	MOV	DS:WinMous,1
	SUB	CX,BX
	MOV	AL,CH
	SUB	AL,2
	CBW
	MOV	SI,AX
	PUSH	DX
	PUSH	BX
	PUSH	AX
	ADD	BX,204h
	ADD	BH,AL
	SUB	DL,10
	MOV	DH,1
	CALL	DoublMo
	POP	AX
	POP	BX
	POP	DX
	JNC	ChTr69
	ADD	AX,DI
	MOV	BP,DS:TreeBuf.TreeLen
	DEC	BP
	CMP	BP,AX
	JBE	ChTr68
	MOV	BP,AX
ChTr68:	JMP	ChTr10
ChTr69:	JE	ChTr66
	MOV	AL,0
	CALL	TypMous
	MOV	AX,1C0Dh

ChTr45:	MOV	SI,OFFSET CrnPath	; Enter
	MOV	DI,OFFSET TmpBuf
	MOV	CX,LenPath
	REP	MOVSB
	MOV	BYTE PTR DS:[DI],0

ChTr46:	CALL	HidCur			; Esc
	MOV	SI,OFFSET MenuBuf+250
	MOV	DI,OFFSET CrnPath
	MOV	CX,LenPath
	REP	MOVSB
	POP	SI
	CALL	ResWin
	CALL	Window
	CMP	AL,0Dh
	JE	ChTr47
	STC
ChTr47:	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	DS
	RET
ChTree	ENDP

ChTr50	PROC	NEAR
	PUSH	DX
	PUSH	BX
	MOV	CL,4
	SHR	AX,CL
	ADD	BH,AL
	ADD	BX,205h
	MOV	DX,126h
	CALL	Window
	POP	BX
	POP	DX
	RET
ChTr50	ENDP

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