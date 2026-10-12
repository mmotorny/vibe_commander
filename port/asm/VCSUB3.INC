;-------------------------------------------------------
;		Version :	09.09.1999
;-------------------------------------------------------

; Of this file, and named by C code or by another file.
	PUBLIC	Mask_S, Ext_F, Tree_F, Tree_P, TrSav_P, DrIn_F

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

	.CONST
DrIn_F	DB	'DIRINFO',0

	.CODE

;---------------------- Save setup ---------------------
; Saving the setup is C: SaveSet and MakeIni of port/setup.c, which only
; C code calls. The name of the file stays here: the code that starts the
; program reads the file too.
	PUBLIC	Init_F
	.CONST
Init_F	DB	'VC.INI',0

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

; For DirSize and DirFunc in C: a call of what FncAddr names, with the
; attributes of a file in AL. Not every such routine leaves DS as it was.
	PUBLIC	FncCall_a
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

; What DirSize puts into FncAddr, for DirFunc to call with every file
;			   AL - its attributes
;	Return :	   AX - see DiSi10 in port/read.c
; C: DiSi10_c of port/read.c. Only FncCall_a calls this.
	PUBLIC	DiSi10
DiSi10	PROC	NEAR
	CallC	DiSi10_c
	RET
DiSi10	ENDP

