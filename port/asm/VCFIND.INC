;-------------------------------------------------------
;		Version :	10.01.2000
;-------------------------------------------------------

LenFnf	EQU	LenStr+1

FndData	STRUC
	DB	TmpBuf + 100 DUP(?)
FndCur	DW	?			; Current line
FndFrst	DW	?			; First output line
FndNumb	DW	?			; Total number of lines
FndSize	DW	?			; Max. number of files
FndBg	DW	?			; Base of the window
FndSz	DW	?			; Size of the window
FndFls	DW	?			; Number of found files
FndMode	DB	?			; 0-auto scroll; 1-allow move cursor
FndDir	DB	?			; Not first file in subdirectory
FndMenu	DB	?			; Menu item
FndDrv	DB	?			; Drive letter
FndData	ENDS

	.CONST
Find0_S	DB	' Find File ',0
Find1_S	DB	'File name:',0
Find2_S	DB	'Containing:',0
Find3_S	DB	'Scanning:',0
Find5_S	DB	'^r[ Stop ]^r',0FFh,3,'[ Go to ]',0FFh,3,'[ Quit ]',0
Find6_S	DB	'[ New search ]',0FFh,3,'[ Go to ]',0FFh,3,'[ Quit ]',0
FnFl6_S	DB	' files',0
FnFl7_S	DB	' found.',0

	.DATA
Find_P	DB	Conf_C, CnfRe_C, CnfBo_C, CnfUn_C
FnFl_P	DB	Conf_C, CnfRe_C, CnfRe_C
Find_B	DW	Find_P, 0, 0
	DB	1, 0FFh, 32, 10h
	DB	1
	DB	1, 1, 0, 0
	DW	305h, FindStr, LenStr
	DB	1
	DB	0, 0, 1, 1
	DW	505h, SrchStr, 100h+LenStr
	DB	0
	PUBLIC	Find5_D			; C code hands it to the menu
Find5_D	DB	 8,  0
	DB	 9, 11
	DB	 8, 23
	DB	 0
Find6_D	DB	14,  0
	DB	 9, 17
	DB	 8, 29
	DB	 0

	.CODE
F67_00:	CALL	HidCur			; Alt F7 - file find
	CALL	PutMnu3
	MOV	BP,OFFSET Find_P
	MOV	BX,203h
	MOV	DH,ES:Lines
	SUB	DH,3
	MOV	DL,76
	MOV	SI,WinBuf
	CALL	SavWin
	MOV	AH,CS:[BP]
	CALL	Color
	CALL	MinBox
	PUSH	SI
	PUSH	CS
	POP	DS
	PUSH	BX
	INC	BH
	MOV	BL,40
	MOV	SI,OFFSET Find0_S
	CALL	CntLine
	POP	BX
F67_01:	PUSH	CS			; Clear window
	POP	DS
	PUSH	BX
	ADD	BH,2
	MOV	CL,DH
	SUB	CL,5
	MOV	CH,0
	MOV	SI,'ºº'
	MOV	AL,' '
	MOV	BP,OFFSET Find_P
	MOV	AH,CS:[BP]
	CALL	Color
F67_02:	CALL	StdLine
	LOOP	F67_02
	SUB	BH,5
	MOV	SI,'Ç¶'
	MOV	AL,'Ä'
	CALL	StdLine
	MOV	AL,25
	ADD	BL,5
	CALL	BasAdr
	MOV	SI,OFFSET Find1_S
	CALL	NgLine
	ADD	BH,2
	CALL	BasAdr
	MOV	SI,OFFSET Find2_S
	CALL	NgLine
	PUSH	ES
	POP	DS
	POP	BX
	CALL	Window
	PUSH	DX			; Input parameters
	PUSH	BX
	ADD	BH,DH
	MOV	DH,9
	SUB	BH,DH
	MOV	BP,OFFSET Find_B
	CALL	Dialog
	POP	BX
	POP	DX
	JE	F67_03
	JMP	F67_25
F67_03:	MOV	BP,OFFSET Find_P	; Clear window
	MOV	AH,CS:[BP]
	CALL	Color
	PUSH	BX
	ADD	BH,DH
	SUB	BH,8
	MOV	SI,'ºº'
	MOV	AL,' '
	CALL	StdLine
	CALL	StdLine
	INC	BH
	CALL	StdLine
	CALL	StdLine
	SUB	BH,3
	MOV	SI,'Ç¶'
	MOV	AL,'Ä'
	CALL	StdLine
	PUSH	CS
	POP	DS
	MOV	AL,25
	ADD	BL,5
	CALL	BasAdr
	MOV	SI,OFFSET Find3_S
	CALL	NgLine
	INC	BH
	MOV	BL,40
	MOV	SI,OFFSET Find5_S
	CALL	CntLine
	POP	BX
	CALL	Window
	PUSH	ES			; Find files
	POP	DS
	MOV	DS:FndBg,BX
	MOV	DS:FndSz,DX
	XOR	AX,AX
	MOV	DS:FndCur,AX
	MOV	DS:FndFrst,AX
	MOV	DS:FndNumb,AX
	MOV	DS:FndFls,AX
	MOV	DS:FndMode,AL
	MOV	DS:FndMenu,AL
	MOV	DS:WinMous,AL
	MOV	DS:MousEnt,AL
	MOV	DS:MousTim.Hi,0FFFFh
	MOV	DS:ViTxHex,AL
	MOV	AX,ES
	ADD	AX,DS:FreMem2
	SUB	AX,DS:ViewSeg
	MOV	DL,AH
	MOV	DH,0
	MOV	CL,4
	SHL	AX,CL
	SHR	DX,CL
	MOV	CX,LenFnf
	DIV	CX
	DEC	AX
	MOV	DS:FndSize,AX
	MOV	DS:FndDrv,'A'
F67_08:	MOV	DX,OFFSET TmpErr
	MOV	SI,OFFSET FindStr
	MOV	DI,DX
	LODSW
	STOSW
	CMP	AX,':*'
	JNE	F67_09
	MOV	AX,SI
	CALL	CutPath
	CMP	AX,SI
	MOV	SI,AX
	JNE	F67_09
	MOV	AL,'\'
	STOSB
F67_09:	MOV	CX,LenPath+13-2
	REP	MOVSB
	MOV	SI,DX
	CALL	NormStr
	CMP	BYTE PTR DS:[SI],0
	JE	F67_14
	CALL	CutPath
	MOV	WORD PTR DS:[SI],'A'
	PUSH	BX
	MOV	SI,DX
	MOV	BP,DS:[SI]
	CMP	BP,':*'
	JNE	F67_15
F67_10:	MOV	CL,DS:FndDrv
	MOV	DS:[SI],CL
	INC	DS:FndDrv
	SUB	CL,'A'
	MOV	CH,0
	XOR	BX,BX
	MOV	AX,150Bh
	INT	2Fh
	CMP	BX,0ADADh
	JNE	F67_11
	OR	AX,AX
	JNE	F67_13
F67_11:	MOV	BX,CX
	INC	BX
	MOV	AX,4409h
	INT	21h
	JC	F67_12
	TEST	DH,10h
	JNZ	F67_15
F67_12:	MOV	AX,4408h
	INT	21h
	JC	F67_13
	CMP	AL,1
	JE	F67_15
F67_13:	CMP	DS:FndDrv,'Z'
	JBE	F67_10
	POP	BX
	JMP	F67_18
F67_14:	MOV	DX,DS:FndSz
	JMP	F67_25
F67_15:	POP	BX
	MOV	DX,SI
	CALL	ChgDisk
	JC	F67_14
	MOV	DI,OFFSET TmpBuf
	LODSW
	CMP	AH,':'
	JE	F67_16
	DEC	SI
	DEC	SI
	MOV	AX,WORD PTR DS:DosPth
F67_16:	CALL	UpCase
	STOSW
	SUB	AL,'A'-1
	MOV	DL,AL
	MOV	CX,LenPath+13
	MOV	AL,'\'
	CMP	AL,DS:[SI]
	JE	F67_17
	STOSB
	XCHG	SI,DI
	MOV	AH,47h
	CALL	Intr21
	XCHG	SI,DI
	JC	F67_19
	MOV	AL,0
	REPNE	SCASB
	DEC	DI
	CALL	AddFile
F67_17:	REP	MOVSB
	MOV	AL,0
	STOSB
	MOV	SI,OFFSET TmpBuf
	CALL	CutPath
	MOV	BYTE PTR DS:[SI],0
	MOV	DS:FncAddr,OFFSET FindFil
	MOV	AL,10h
	CALL	FindFil
	OR	AH,AH
	JNZ	F67_18
	CMP	BP,':*'
	JNE	F67_18
	CMP	DS:FndDrv,'Z'
	JA	F67_18
	JMP	F67_08
F67_18:	TEST	DS:FndMode,2
	JZ	F67_19
	MOV	AL,DS:FndMenu
	CMP	AL,0
	JE	F67_19
	MOV	DX,DS:FndSz
	JMP	F67_23
F67_19:	MOV	DI,OFFSET TmpBuf	; Output NNN files found
	PUSH	DI
	MOV	CX,DS:FndFls
	XOR	BX,BX
	MOV	AL,1
	CALL	Number
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET FnFl6_S
	MOV	CX,15
	CALL	MvzLine
	CMP	ES:FndFls,1
	JNE	F67_20
	DEC	DI
F67_20:	MOV	SI,OFFSET FnFl7_S
	MOV	CX,15
	REP	MOVSB
	PUSH	ES
	POP	DS
	POP	SI
	MOV	BX,DS:FndBg
	MOV	DX,DS:FndSz
	PUSH	DX
	PUSH	BX
	ADD	BH,DH
	SUB	BH,5
	ADD	BL,5
	CALL	BasAdr
	SUB	DL,12
	MOV	DH,0
	MOV	CX,DX
	MOV	AH,DS:[DI+1]
	MOV	AL,0
	PUSH	DI
	REP	STOSW
	POP	DI
	MOV	BP,OFFSET Find_P
	MOV	AX,DX
	CALL	NgLine
	MOV	DH,1
	CALL	Window
	POP	BX
	POP	DX
	PUSH	BX
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET Find6_S
	ADD	BH,DH
	SUB	BH,4
	MOV	BL,40
	CALL	CntLine
	PUSH	ES
	POP	DS
	POP	BX
	MOV	DS:FndMenu,1
	MOV	BP,DS:FndCur
	MOV	DI,DS:FndFrst
	OR	BP,BP
	JE	F67_21
	MOV	AH,CS:Find_P[1]
	CALL	FndCurs
F67_21:	CALL	GetMous			; Dialog
	MOV	AH,0FCh
	JNZ	F67_22
	MOV	DS:WinMous,0
	MOV	DS:MousEnt,0
	PUSH	DX
	PUSH	BX
	ADD	BH,DH
	SUB	BH,4
	ADD	BL,18
	MOV	DX,125h
	MOV	BP,OFFSET FnFl_P
	MOV	SI,OFFSET Find6_D
	MOV	CL,DS:FndMenu
	MOV	AL,CS:Find_B.bKeyBar
	CALL	MinMnu
	MOV	DS:FndMenu,CL
	POP	BX
	POP	DX
F67_22:	MOV	CL,1
	CALL	FndMove
	OR	AH,AH
	JE	F67_21
F67_23:	CMP	AL,1
	JE	F67_30
	JA	F67_25
	JMP	F67_01
F67_25:	POP	SI
	CALL	ResWin
	CALL	Window
F67_26:	JMP	Func04
F67_30:	POP	SI			; Go to ...
	CALL	ResWin
	CALL	Window
	CMP	DS:FndNumb,0
	JE	F67_26
	MOV	BP,DS:FndCur
F67_31:	MOV	AX,BP
	CALL	FndAdr
	DEC	BP
	CMP	BYTE PTR DS:[SI],' '
	JE	F67_31
	MOV	DX,SI
	CALL	ChgDisk
	JC	F67_35
	MOV	AH,3Bh
	CALL	Intr21
	JC	F67_35
	MOV	DL,DS:[SI]
	SUB	DL,'A'
	MOV	AH,0Eh
	INT	21h
	PUSH	ES			; Set cursor
	PUSH	DS
	MOV	DS,ES:WCBcrn
	CMP	DS:WinTyp,0
	JE	F67_32
	CALL	InvWCB
F67_32:	PUSH	DS
	POP	ES
	POP	DS
	MOV	DI,OFFSET WinPath
	MOV	CX,LenPath+1
	REP	MOVSB
	PUSH	ES
	POP	DS
	POP	ES
	MOV	DI,OFFSET DskBuf
	LEA	BX,[DI+SIZE FilEnt]
	MOV	DS:CurAdr,DI
	MOV	DS:First,BX
	MOV	DS:EndBuf,BX
	MOV	DS:[DI].NameF,0
	MOV	DS:[BX].NameF,0
	PUSH	ES
	PUSH	DS
	MOV	AX,ES:FndCur
	CALL	FndAdr
	CMP	BYTE PTR DS:[SI],' '
	JNE	F67_33
	POP	ES
	PUSH	DI
	ADD	SI,3
	MOV	CX,12
	REP	MOVSB
	MOV	AL,0
	STOSB
	POP	SI
	CALL	NormStr
	PUSH	ES
F67_33:	POP	DS
	POP	ES
	MOV	AH,19h
	INT	21h
	ADD	AL,'A'
	CMP	AL,ES:DosPth
	JNE	F67_34
	JMP	F24_02
F67_34:	JMP	FncEx1
F67_35:	JMP	Func04

; The search and its list are C: port/find.c.
;			   AL - attributes of what is at TmpBuf
;	Return :	   AX - 0 to go on, 100h to stop
FindFil	PROC	NEAR			; Find files
	CallC	FindFil_c
	RET
FindFil	ENDP

;			   AX - key, or AH = 0FCh for the mouse
;			   CL - for FndMode
;			BH,BL - begin of the box
;			DH,DL - its size
;	Return :	   AX - 0, or AH = 1 and AL the answer
FndMove	PROC	NEAR			; Move the cursor
	CallC	FndMove_c
	RET
FndMove	ENDP

;			   AH - colour
;			   BP - line
;			   DI - first line shown
;			BH,BL - begin of the box
;			DH,DL - its size
FndCurs	PROC	NEAR			; Set/Reset cursor
	PUSH	SI
	MOV	SI,BP
	CallC	FndCurs_c
	POP	SI
	RET
FndCurs	ENDP

;			   AX - line
;	Return :	DS:SI - its address
FndAdr	PROC	NEAR
	PUSH	DX
	CallC	FndAdr_c
	MOV	DS,DX
	POP	DX
	RET
FndAdr	ENDP

; Entries for the C code of the search to routines of other files (see
; FromC in VC.ASM, and port/find.c for what they take and return).

; The BIOS for the keyboard: AX as INT 16h takes it and as it comes back,
; and DX is 1 if the zero flag was set, 0 if not.
	PUBLIC	Int16_a
Int16_a	PROC	FAR
	INT	16h
	MOV	DX,0
	JNZ	Int16A
	INC	DX
Int16A:	RET
Int16_a	ENDP
