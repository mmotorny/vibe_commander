;-------------------------------------------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;-------------- Read directory with save ---------------
;			    DS - WCBseg
;			AL = 0 - Without save
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

;---------------------- Save setup ---------------------
; Saving the setup is C: SaveSet and MakeIni of port/setup.c, which only
; C code calls. The name of the file is in port/system_t.c.

;-------------------- Fantom Diskette ------------------
;			AL - Disk ID
; C: Fantom_c of port/system.c, which returns 1 if the user would not
; change the disk. It has Dialog show the box of port/system_t.c.

