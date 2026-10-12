;-------------------------------------------------------
;		Version :	07.06.2000
;-------------------------------------------------------

AdrNum	PROC	NEAR
	PUSH	AX			; Room for the four numbers
	PUSH	AX
	PUSH	AX
	PUSH	AX
	MOV	BX,SP
	MOV	AX,DS
	CallC	AdrNum_c
	POP	SI
	POP	DI
	POP	DX
	POP	AX
	MOV	BX,AX
	RET
AdrNum	ENDP

NumAdr	PROC	NEAR
	MOV	AX,DS
	CALL	FAR PTR NumAdr_c
	MOV	AX,SI			; What the multiplication left
	XOR	DX,DX
	RET
NumAdr	ENDP

ChgAdr	PROC	NEAR
	PUSH	DX
	PUSH	AX
	MOV	AX,DS
	CallC	ChgAdr_c
	MOV	SI,AX
	MOV	DI,DX
	POP	AX
	POP	DX
	RET
ChgAdr	ENDP

F00_50	PROC	NEAR
	CALL	ChgAdr
	CALL	CorPar
	MOV	CX,DI
	MOV	AX,SIZE FilEnt
	MOV	BX,DS:First
	MOV	DX,DS:CurAdr
	CMP	DS:WinTyp,0
	JE	F00_51
	MOV	AX,16
	MOV	BX,ES:TreFrst
	MOV	DX,ES:TreAdr
	CMP	SI,DX
	JE	F00_51
	MOV	ES:FlagCD,1
F00_51:	SUB	CX,BX
	JE	F00_52
	MOV	BL,7
	CMP	CX,AX
	JE	F00_53
	MOV	BL,6
	NEG	AX
	CMP	CX,AX
	JE	F00_53
	CALL	PutFls
	CALL	BegBX
	CALL	SizDX
	JMP	F00_58
F00_52:	CMP	SI,DX
	JNE	F00_53
	JMP	F00_59
F00_53:	CALL	ChgAdr
	MOV	AL,0
	CALL	SetCur
	CALL	ChgAdr
	CALL	PutFls
	JCXZ	F00_57
	MOV	AL,BL
	PUSH	AX
	CALL	BegBX
	LEA	CX,[BX+201h]
	CALL	SizDX
	SUB	DH,4
	CMP	DS:WinTyp,0
	JE	F00_54
	DEC	CH
	DEC	DH
	JMP	F00_55
F00_54:	CMP	ES:MiniSta,0
	JZ	F00_55
	SUB	DH,2
F00_55:	MOV	DL,38-1
	ADD	DX,CX
	MOV	AH,Back_C
	CALL	Color
	MOV	BH,AH
	MOV	AH,AL
	MOV	AL,1
	CALL	Int10m
	POP	AX
	MOV	BX,CX
	CMP	AL,7
	JE	F00_56
	MOV	BH,DH
F00_56:	MOV	DX,126h
	CALL	Window
F00_57:	MOV	AL,1
	CALL	SetCur
	MOV	AL,DS:WinTyp
	OR	AL,ES:MiniSta
	JZ	F00_59
	CALL	BegBX
	CALL	SizDX
	ADD	BH,DH
	SUB	BH,2
	INC	BL
	MOV	DX,126h
F00_58:	CALL	Window
F00_59:	RET
F00_50	ENDP

F00_60:	TEST	BP,2			; Scroll & select
	JZ	F00_63
	CMP	DS:WinTyp,0
	JNE	F00_70
	PUSH	SI
	CALL	NumAdr
	MOV	BX,SI
	POP	SI
	CMP	BX,DS:EndBuf
	JAE	F00_70
	CMP	DS:[BX].NameF[0],'.'
	JE	F00_70
	TEST	ES:WinMous,2
	JNZ	F00_61
	OR	ES:WinMous,2
	TEST	DS:[BX].AttrF,40h
	JNZ	F00_61
	OR	ES:WinMous,4
F00_61:	MOV	AL,DS:[BX].AttrF
	AND	AL,0BFh
	TEST	ES:WinMous,4
	JZ	F00_62
	OR	AL,40h
F00_62:	CMP	AL,DS:[BX].AttrF
	JE	F00_70
	MOV	DS:[BX].AttrF,AL
	CALL	F08_10
	JMP	F00_70
F00_63:	AND	ES:WinMous,1

F00_70:	CALL	NumAdr
	XCHG	SI,DI
	CALL	NumAdr
	XCHG	SI,DI
	CALL	F00_50
F00_80:	JMP	Func07

F00_00:	CMP	DS:Visible,0		; Up
	JNZ	F00_01
	JMP	F25_00
F00_01:	CMP	DS:WinTyp,1
	JA	F00_80
	CALL	AdrNum
	XOR	BP,BP
F00_05:	OR	SI,SI
	JE	F00_06
	DEC	SI
F00_06:	JMP	F00_60

F01_00:	CMP	DS:Visible,0		; Down
	JNZ	F01_01
	JMP	F26_00
F01_01:	CMP	DS:WinTyp,1
	JA	F00_80
	CALL	AdrNum
	XOR	BP,BP
F01_05:	CMP	SI,DX
	JAE	F01_06
	INC	SI
F01_06:	JMP	F00_60

F02_00:	MOV	AL,0			; Left
	CMP	AL,DS:Visible
	JZ	F02_10
	CMP	AL,DS:WinTyp
	JNE	F02_10
	CMP	AL,DS:BrfFul
	JNE	F02_10
	CALL	AdrNum
	MOV	AX,SI
	SUB	AX,DI
	CMP	AX,BX
	JB	F02_02
	SUB	SI,BX
F02_01:	JMP	F00_70
F02_02:	SUB	SI,BX
	JAE	F02_03
	XOR	SI,SI
F02_03:	SUB	DI,BX
	JAE	F02_01
	JMP	F06_01
F02_10:	JMP	F73_00

F03_00:	MOV	AL,0			; Right
	CMP	AL,DS:Visible
	JZ	F02_10
	CMP	AL,DS:WinTyp
	JNE	F02_10
	CMP	AL,DS:BrfFul
	JNZ	F02_10
	CALL	AdrNum
	MOV	AX,SI
	SUB	AX,DI
	CALL	F03_30
	MOV	BP,BX
	ADD	BP,BP
	CMP	AX,BP
	JB	F02_01
	XCHG	SI,DI
	CALL	F03_30
	XCHG	SI,DI
	ADD	BP,BX
	MOV	AX,DX
	SUB	AX,DI
	INC	AX
	CMP	AX,BP
	JAE	F02_01
	OR	DI,DI
	JE	F02_01
	JMP	F07_01

F03_30	PROC	NEAR
	ADD	SI,BX
	CMP	SI,DX
	JBE	F03_31
	MOV	SI,DX
F03_31:	RET
F03_30	ENDP

F04_00:	CMP	DS:Visible,0		; PgUp
	JZ	F05_10
	CMP	DS:WinTyp,1
	JA	F05_10
	CALL	F04_30
	OR	DI,DI
	JNE	F04_01
	JMP	F06_01
F04_01:	SUB	DI,BX
	JAE	F04_02
	XOR	DI,DI
F04_02:	ADD	BX,DI
	CMP	BX,SI
	JAE	F04_03
	MOV	SI,BX
F04_03:	JMP	F00_70

F04_30	PROC	NEAR
	CALL	AdrNum
	MOV	AL,DS:WinTyp
	OR	AL,DS:BrfFul
	MOV	AX,BX
	JNZ	F04_31
	ADD	BX,AX
	ADD	BX,AX
F04_31:	DEC	BX
	RET
F04_30	ENDP

F05_00:	CMP	DS:Visible,0		; PgDn
	JZ	F05_10
	CMP	DS:WinTyp,1
	JA	F05_10
	CALL	F04_30
	MOV	CX,DI
	XCHG	SI,DI
	CALL	F03_30
	XCHG	SI,DI
	MOV	AX,DX
	SUB	AX,BX
	JAE	F05_01
	XOR	AX,AX
F05_01:	CMP	DI,AX
	JBE	F05_02
	MOV	DI,AX
F05_02:	CMP	DI,CX
	JBE	F05_04
	CMP	DI,SI
	JBE	F05_03
	MOV	SI,DI
F05_03:	JMP	F00_70
F05_04:	JMP	F07_01
F05_10:	JMP	Func07

F06_00:	CMP	DS:Visible,0		; Home
	JZ	F06_10
	CMP	DS:WinTyp,1
	JA	F06_10
	CALL	AdrNum
F06_01:	XOR	SI,SI
	XOR	DI,DI
	JMP	F00_70
F06_10:	JMP	F73_00

F07_00:	CMP	DS:Visible,0		; End
	JZ	F06_10
	CMP	DS:WinTyp,1
	JA	F06_10
	CALL	AdrNum
F07_01:	MOV	SI,DX
	MOV	DI,DX
	JMP	F00_70

F08_00:	CALL	FilePan			; Ins
	JNC	F05_10
	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F05_10
	PUSH	SI
	CMP	ES:InsDwn,0
	JZ	F08_01
	ADD	SI,SIZE FilEnt
	CMP	SI,DS:EndBuf
	JAE	F08_01
	PUSH	DS
	XOR	AX,AX
	MOV	DS,AX
	AND	BYTE PTR DS:[418h],7Fh
	POP	DS
F08_01:	POP	SI
	CMP	DS:[SI].NameF[0],'.'
	JE	F08_02
	XOR	DS:[SI].AttrF,40h
F08_02:	CALL	F08_10
	CMP	ES:InsDwn,0
	JZ	F05_10
	JMP	F01_01

F08_10	PROC	NEAR
	MOV	AL,1
	CALL	SetCur
	CMP	ES:MiniSta,0
	JZ	F08_11
	CALL	PutFls
	CALL	BegBX
	CALL	SizDX
	ADD	BH,DH
	SUB	BH,2
	INC	BX
	MOV	DX,126h
	CALL	Window
F08_11:	RET
F08_10	ENDP

	.CONST
Sel_S	DB	'Select',0
Unsl_S	DB	'Unselect',0
Inv_S	DB	'Invert',0
IncSp_S	DB	' ^I ',0
Sel1_S	DB	'^I the files', 0
ErSel_S	DB	'Could not find a match',0
Kov_S	DB	'"^V"'
Empty_S	DB	0

	.DATA
Sel_P	DB	Box_C, BoxCu_C, 0, 0
	DB	3, 0
	DW	WinBuf
	DW	IncSp_S
	DW	Sel1_S
	DW	Empty_S
Sel_B	BOXHD	< Sel_P, 528h, 0, 1, 2, 25, 0 >
	BOXMN	< 1 ,0,0,0,0, 305h, SelMas, 20Ch >
	DB	0
ErSel_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	WinBuf
	DW	SelMas
	DB	4, 50
	DW	IncSp_S
	DW	ErSel_S
	DW	Kov_S
	DW	Ok_S
ErSel_B	BOXHD	< ErSel_P, 528h, 0, 1, 0FFh, 0, 0 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpBuf, OkMn_D>
	DB	0

	.CODE
F76_00:	CMP	ES:DosBuf,0		; Grey *
	JNE	F76_02
F76_01:	MOV	DX,OFFSET Inv_S
	JMP	F10_01
F76_02:	JMP	F73_00

F09_00:	MOV	DX,OFFSET Sel_S		; Grey +
	JMP	F10_01

F10_00:	MOV	DX,OFFSET Unsl_S	; Grey -
F10_01:	CALL	FilePan
	JNC	F76_02
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	BP,OFFSET Sel_B
	CALL	Dialog
	POP	DS
	JNE	F10_08
	MOV	SI,OFFSET SelMas
	CMP	BYTE PTR ES:[SI],0
	JE	F10_08
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	DI,OFFSET TmpBuf
	CALL	Convert
	POP	DS
	MOV	CH,0
	MOV	SI,OFFSET DskBuf
F10_03:	CMP	SI,DS:EndBuf
	JAE	F10_07
	CMP	DS:[SI].NameF,'.'
	JE	F10_06
	CALL	TstMas
	JNC	F10_06
	CMP	DX,OFFSET Unsl_S
	JNE	F10_04
	MOV	CH,1
	AND	DS:[SI].AttrF,0BFh
	JMP	F10_06
F10_04:	TEST	DS:[SI].AttrF,10h
	JNZ	F10_06
	MOV	CH,1
	CMP	DX,OFFSET Sel_S
	JNE	F10_05
	OR	DS:[SI].AttrF,40h
	JMP	F10_06
F10_05:	XOR	DS:[SI].AttrF,40h
F10_06:	ADD	SI,SIZE FilEnt
	JMP	F10_03
F10_07:	OR	CH,CH
	JNZ	F10_09
	PUSH	ES
	POP	DS
	MOV	BP,OFFSET ErSel_B
	CALL	Dialog
F10_08:	JMP	Func04
F10_09:	JMP	Func01

F78_00:	CALL	FilePan			; ^M - restore marks
	JNC	F10_08
	MOV	SI,OFFSET DskBuf
F78_01:	CMP	SI,DS:EndBuf
	JAE	F10_09
	CMP	DS:[SI].NameF,'.'
	JE	F78_02
	TEST	DS:[SI].AttrF,80h
	JZ	F78_02
	OR	DS:[SI].AttrF,40h
F78_02:	ADD	SI,SIZE FilEnt
	JMP	F78_01

F11_00:	CMP	DS:Visible,0		; ^O
	PUSH	DS
	MOV	DS,ES:WCBsec
	JZ	F11_01
	MOV	AL,DS:Visible		; Panels Off
	MOV	ES:Ctrl_O,AL
	MOV	DS:Visible,0
	POP	DS
	MOV	DS:Visible,0
	CALL	BegBX
	CALL	SizDX
	MOV	BL,0
	MOV	DL,80
	CALL	ResUser
	JMP	Func03
F11_01:	MOV	AL,ES:Ctrl_O		; Panels On
	MOV	DS:Visible,AL
	MOV	AH,DS:ChgFlg
	POP	DS
	MOV	DS:Visible,1
	CMP	DS:ChgFlg,0
	JNZ	F11_02
	CMP	AL,0
	JZ	F11_03
	CMP	AH,0
	JZ	F11_03
F11_02:	CALL	PutMnu3
F11_03:	JMP	Init50

F12_00:	CMP	DS:Visible,0		; ^P
	PUSH	DS
	MOV	DS,ES:WCBsec
	MOV	CL,0
	JNZ	F14_01
	POP	DS
	JMP	Func05

F13_00:	PUSH	DS			; ^F1
	MOV	DS,ES:WCB1seg
	MOV	CL,1
	JMP	F14_01

F14_00:	PUSH	DS			; ^F2
	MOV	DS,ES:WCB2seg
	MOV	CL,5
F14_01:	JMP	M06_00

F15_00:	CMP	DS:Visible,0		; ^U
	JNZ	F15_01
	JMP	Func04
F15_01:	MOV	AX,ES:WCB1seg
	XCHG	AX,ES:WCB2seg
	MOV	ES:WCB1seg,AX
	MOV	BX,OFFSET Active
	CALL	InvVar
	CALL	BegBX
	CALL	SizDX
	MOV	BL,0
	MOV	DL,80
	CALL	ResUser
	JMP	Init50

F16_00:	PUSH	ES			; ^B
	POP	DS
	MOV	BH,DS:Lines
	DEC	BH
	CMP	DS:KeyBar,0
	JZ	F16_02
	XOR	AX,AX
	MOV	DS:KeyBar,AL
	MOV	DS:BarPrev,0FFh
	CALL	ClsLine
	MOV	DX,150h
	CALL	Window
F16_01:	JMP	Func04
F16_02:	MOV	DS:KeyBar,1
	CMP	BH,DS:DosLin
	JNE	F16_01
	CALL	BegBX
	CALL	SizDX
	MOV	BL,0
	MOV	DL,80
	CALL	ResUser
	CALL	Window
	CALL	Cursor1
	CALL	SavMous
	MOV	AL,0
	CALL	CurMous
	JMP	Init40

F17_00:	PUSH	DS			; ^R
F17_01:	MOV	AL,DS:WinTyp
	CMP	AL,0
	JE	F17_04
	CMP	AL,2
	JA	F17_03
	JE	F17_02
	MOV	SI,OFFSET DosPth
	CALL	DelTr0
	JC	F17_03
F17_02:	MOV	CH,5
	SUB	CH,AL
	JMP	M03_00
F17_03:	POP	DS			; ???
	JMP	Func04
F17_04:	MOV	SI,DS:EndBuf
	MOV	DS:CurAdr,SI
	MOV	DS:[SI],AL
	MOV	SI,OFFSET DskBuf
F17_05:	CMP	SI,DS:EndBuf
	JAE	F17_06
	AND	DS:[SI].AttrF,0BFh
	ADD	SI,SIZE FilEnt
	JMP	F17_05
F17_06:	MOV	DS:ChgFlg,1
	MOV	CL,DS:SortTyp
	MOV	CH,DS:BrfFul
	JMP	M01_01

F79_00:	MOV	AL,1			; ^Z - tree
	JMP	F18_01

F18_00:	MOV	AL,2			; ^L - info
F18_01:	CMP	DS:Visible,0
	JE	F18_03
	PUSH	DS
	CMP	AL,DS:WinTyp
	JE	F18_02
	CALL	InvWCB
	CMP	DS:Visible,0
	JZ	F17_02
	CMP	AL,DS:WinTyp
	JNE	F17_02
F18_02:	MOV	AL,DS:Ctrl_L
	CMP	AL,0
	JE	F17_06
	JMP	F17_02
F18_03: JMP	Func04

F20_00:	PUSH	DS			; Tab
	CALL	InvWCB
	MOV	AX,DS
	CMP	DS:Visible,0
	POP	DS
	JNZ	F20_01
	JMP	Func05
F20_01:	XCHG	AX,ES:WCBcrn
	CALL	PutPath
	CALL	PutFls
	XCHG	AX,ES:WCBcrn
	CALL	F20_02
	CALL	PutPath
	JMP	Func01

F20_02	PROC	NEAR
	XOR	BX,BX
	MOV	DH,ES:Lines
	MOV	DL,80
	CMP	DS:WinTyp,0
	MOV	DS,ES:WCBsec
	JNE	F20_03
	CMP	DS:WinTyp,0
	JNE	F20_03
	CMP	ES:PathEr,0
	JNZ	F20_04
	PUSH	ES
	MOV	ES,ES:WCBcrn
	MOV	SI,OFFSET WinPath
	MOV	DI,SI
	CALL	StrLen
	REPE	CMPSB
	POP	ES
	JNE	F20_04
	JMP	F20_08
F20_03:	CMP	ES:PathEr,0
	JZ	F20_08
	CALL	HidCur
	CALL	Window
	JMP	F20_07
F20_04:	CALL	HidCur
	CALL	Window
	MOV	AL,DS:WinPath
	SUB	AL,'A'-1
	CALL	Fantom
	JC	F20_05
	MOV	DX,OFFSET WinPath
	MOV	AH,3Bh
	CALL	Intr21
	JNC	F20_06
	CMP	AX,WORD PTR 3
	JE	F20_06
F20_05:	MOV	SI,OFFSET WinPath
	MOV	DI,OFFSET DosPth
	MOV	CX,3
	REP	MOVSB
	MOV	AL,0
	STOSB
	MOV	ES:PathEr,1
	MOV	DS:ChgFlg,1
	MOV	DL,DS:WinPath
	SUB	DL,'A'
	MOV	AH,0Eh
	INT	21h
	JMP	F20_08
F20_06:	MOV	DL,DS:WinPath
	SUB	DL,'A'
	MOV	AH,0Eh
	INT	21h
F20_07:	CALL	SetPth
F20_08:	CALL	PutPrm
	MOV	AX,ES:WCBcrn
	XCHG	AX,ES:WCBsec
	MOV	ES:WCBcrn,AX
	MOV	DS,AX
	MOV	BX,OFFSET Active
	CALL	InvVar
	RET
F20_02	ENDP

F21_00:	CMP	ES:DosBuf,0		; Enter
	JNE	F21_01
	CMP	DS:Visible,0
	JNZ	F21_10
F21_01:	JMP	F21_25
F21_10:	CMP	DS:WinTyp,1		; Tree
	JNE	F21_20
	PUSH	DS
	PUSH	ES
	POP	DS
	CMP	DS:TreeBuf.TreeLen,0
	JE	F21_12
	CALL	HidCur
	MOV	AL,DS:DosPth[0]
	SUB	AL,'A'-1
	CALL	Fantom
	JC	F21_12
	MOV	DX,OFFSET CrnPath+2
	CMP	DS:CrnPath[3],0
	JE	F21_11
	CALL	SetDTA
	MOV	CX,37h
	MOV	AH,4Eh
	CALL	Intr21
	JC	F21_12
	TEST	DS:DTA.FilAttr,10h
	JZ	F21_12
F21_11:	MOV	AH,3Bh
	CALL	Intr21
	JC	F21_12
	POP	DS
	JMP	F24_02
F21_12:	POP	DS
F21_13:	JMP	Func04
F21_20:	CMP	DS:WinTyp,0		; Files
	JNE	F21_13
	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F21_13
	TEST	DS:[SI].AttrF,10h
	JZ	F21_21
	JMP	F23_02
F21_21:	CALL	TstExe
	JC	F21_22
	MOV	SI,OFFSET ExtBuf	; Test extension
	MOV	DI,OFFSET MenuBuf
	MOV	CX,LenMenu-1
	REP	MOVSB
	MOV	AL,1Ah
	STOSB
	MOV	SI,DS:CurAdr
	CALL	FindExt
	JC	F21_13
	DEC	SI
	MOV	BYTE PTR ES:[SI],' '
	XOR	BP,BP
	JMP	Run_00
F21_22:	MOV	BX,DS:CurAdr		; Run file from panel
	XOR	DX,DX
	MOV	SI,OFFSET DosBuf
	MOV	ES:[SI],DL
	MOV	CX,LenCmd-1
	MOV	AH,0
F21_23:	MOV	AL,DS:[BX]
	CMP	AL,0
	JE	F21_24
	INC	BX
	CALL	EdLin
	JC	F21_23
F21_24:	MOV	ES:DosCol,DL
	CALL	DosCom
	CALL	Cursor1
F21_25:	MOV	DI,OFFSET DosBuf	; Run command from DOS line
	PUSH	DI
	MOV	CX,256
	MOV	AL,0
	REPNE	SCASB
	POP	SI
	MOV	CX,DI
	SUB	CX,SI
	STOSB
	CMP	CX,1
	JE	F21_30
	MOV	DI,OFFSET HstBuf+1
	PUSH	CX
	PUSH	DS
	PUSH	ES
	POP	DS
	REPE	CMPSB
	POP	DS
	POP	BX
	JE	F21_30
	MOV	DI,OFFSET HstBuf+LenHist-1
	MOV	SI,DI
	SUB	SI,BX
	MOV	CX,LenHist
	SUB	CX,BX
	PUSH	DS
	PUSH	ES
	POP	DS
	STD
	REP	MOVSB
	CLD
	MOV	SI,OFFSET DosBuf
	MOV	DI,OFFSET HstBuf+1
	MOV	CX,BX
	REP	MOVSB
	POP	DS
	MOV	AL,0
F21_26:	CMP	AL,ES:[DI]
	JE	F21_30
	MOV	SI,DI
	MOV	CX,256
	REPNE	SCASB
	CMP	DI,OFFSET HstBuf+LenHist
	JB	F21_26
	MOV	ES:[SI],AL
F21_30:	MOV	AL,ES:LoadFl
	AND	AL,7Fh
	MOV	ES:LoadRes,AL
	MOV	AH,2
	INT	16h
	TEST	AL,3
	JZ	F21_31
	MOV	BX,OFFSET LoadFl
	MOV	AH,ES:[BX]
	AND	BYTE PTR ES:[BX],7Fh
	CALL	InvVar
	AND	AH,80h
	OR	ES:[BX],AH
F21_31:	CALL	RestVct
	PUSH	ES
	POP	DS
	CALL	SavMous
	PUSH	SS
	POP	ES
	XOR	SI,SI
	MOV	DI,OFFSET ComnIni
	MOV	CX,CmnSave
	REP	MOVSB
	PUSH	DS
	MOV	DS,DS:WCB1seg
	MOV	DI,OFFSET WCB1sav
	CALL	F21_40
	POP	DS
	PUSH	DS
	MOV	DS,DS:WCB2seg
	MOV	DI,OFFSET WCB2sav
	CALL	F21_40
	POP	ES
	CALL	Quit
	INC	DH
	TEST	DS:LoadFl,80h
	JNZ	F21_32
	CALL	PutPrm
F21_32:	CALL	Window
	CALL	Cursor1
	MOV	AH,49h
	INT	21h
	MOV	AL,0
	CALL	CurMous
	PUSH	SS
	PUSH	SS
	POP	DS
	POP	ES
	MOV	BX,OFFSET ComnIni
	CMP	DS:[BX].DosBuf,0
	JNE	F21_33
	MOV	AL,DS:[BX].LoadRes
	MOV	DS:[BX].LoadFl,AL
	JMP	Init11
F21_33:	MOV	AX,0E0Dh
	INT	10h
	TEST	DS:[BX].LoadFl,80h
	JNZ	F21_34
	MOV	AX,0E0Ah
	INT	10h
F21_34:	MOV	AH,19h
	INT	21h
	MOV	AH,0
	CMP	AL,DS:[BX].DosChg[0]
	JE	F21_35
	ADD	AL,'A'
	MOV	AH,AL
F21_35:	MOV	AL,2
	MOV	WORD PTR DS:[BX].DosChg[0],AX
	MOV	WORD PTR DS:[BX].DosChg[2],0D3Ah
	MOV	AX,CS
	ADD	AX,WORD PTR ((Init00-Zero) SHR 4)
	MOV	ES,AX
	MOV	AX,OFFSET Exec
	PUSH	DS
	PUSH	AX
	RETF

F21_40	PROC	NEAR
	MOV	AX,DS:EndBuf
	MOV	CX,OFFSET DskBuf+MinFls*SIZE FilEnt
	CMP	AX,CX
	JBE	F21_42
	MOV	AX,CX
	MOV	DS:EndBuf,AX
	CMP	AX,DS:CurAdr
	JA	F21_42
	PUSH	ES
	PUSH	DI
	PUSH	DS
	POP	ES
	MOV	CX,SIZE FilEnt
	MOV	DI,AX
	SUB	DI,CX
	MOV	SI,DS:First
	CMP	SI,DI
	JB	F21_41
	SUB	DI,CX
	MOV	DS:First,DI
	AND	DS:[SI].AttrF,3Fh
	PUSH	CX
	REP	MOVSB
	POP	CX
F21_41:	MOV	SI,DS:CurAdr
	MOV	DS:CurAdr,DI
	AND	DS:[SI].AttrF,3Fh
	REP	MOVSB
	POP	DI
	POP	ES
F21_42:	XOR	SI,SI
	MOV	CX,WCBsave
	REP	MOVSB
	MOV	SI,OFFSET DskBuf
	SUB	AX,SI
	XOR	DX,DX
	MOV	CX,SIZE FilEnt
	DIV	CX
	MOV	CX,AX
	JCXZ	F21_44
F21_43:	PUSH	SI
	PUSH	CX
	MOV	CX,12
	REP	MOVSB
	INC	SI
	MOVSB
	POP	CX
	POP	SI
	ADD	SI,SIZE FilEnt
	LOOP	F21_43
F21_44:	RET
F21_40	ENDP

F22_00:	MOV	SI,OFFSET DskBuf	; ^PgUp
	CMP	DS:[SI].NameF,'.'
	JE	F23_01
F22_01:	JMP	Func05

F23_00:	MOV	SI,DS:CurAdr		; ^PgDn
F23_01:	CALL	FilePan
	JNC	F22_01
	CMP	SI,DS:EndBuf
	JAE	F22_01
	TEST	DS:[SI].AttrF,10h
	JZ	F22_01
F23_02:	CALL	HidCur			; Change Directory
	MOV	AL,DS:WinPath
	SUB	AL,'A'-1
	CALL	Fantom
	JC	F23_05
	MOV	DX,SI
	MOV	AH,3Bh
	CALL	Intr21
	JC	F23_05
	CMP	DS:[SI].NameF,'.'
	JNE	F24_02
	MOV	DI,OFFSET DskBuf
	LEA	BX,[DI+SIZE FilEnt]
	MOV	DS:CurAdr,DI
	MOV	DS:First,BX
	MOV	DS:EndBuf,BX
	MOV	DS:[BX].NameF,0
	MOV	SI,OFFSET WinPath
	CALL	CutPath
	PUSH	ES
	PUSH	DS
	POP	ES
	PUSH	SI
	MOV	CX,12
	REP	MOVSB
	MOV	AX,1000h
	STOSW
	POP	DI
	CMP	DI,OFFSET WinPath[3]
	JBE	F23_04
	DEC	DI
F23_04:	STOSB
	POP	ES
	JMP	F24_02
F23_05:	JMP	Func04

	.CONST
CDir_S	DB	'\',0

	.CODE
F24_00:	CALL	HidCur			; ^\
	MOV	AL,ES:DosPth[0]
	SUB	AL,'A'-1
	CALL	Fantom
	JC	F23_05
	MOV	DX,OFFSET CDir_S
	PUSH	DS
	PUSH	CS
F24_01:	POP	DS
	MOV	AH,3Bh
	CALL	Intr21
	POP	DS
	JC	F23_05
	MOV	DS:EndBuf,OFFSET DskBuf
F24_02:	CALL	SetPth
	MOV	AH,0
	MOV	AL,DS:WinTyp
	CMP	AL,1
	JE	F24_03
	MOV	DS:ChgFlg,1
	JMP	F24_04
F24_03:	CMP	DS:ChgFlg,0
	JNE	F24_04
	MOV	AH,1
F24_04:	CALL	InvWCB
	OR	AL,DS:WinTyp
	JZ	F24_07
	CMP	DS:WinTyp,1
	JE	F24_05
	MOV	DS:ChgFlg,1
	JMP	F24_06
F24_05:	CMP	DS:ChgFlg,0
	JNE	F24_06
	MOV	AH,1
F24_06:	CMP	AH,0
	JE	F24_07
	PUSH	ES
	POP	DS
	CALL	TrCr10
F24_07:	JMP	Init50

F25_00:	MOV	DI,ES:HstAdr		; ^E
	MOV	CX,256
	MOV	AL,0
	REPNE	SCASB
	CMP	AL,ES:[DI]
	JNE	F26_02
	MOV	DI,ES:HstAdr
	JMP	F26_02

F26_00:	MOV	DI,ES:HstAdr		; ^X
	DEC	DI
	CMP	DI,OFFSET HstBuf
	JB	F26_01
	JE	F26_02
	DEC	DI
	MOV	CX,256
	MOV	AL,0
	STD
	REPNE	SCASB
	CLD
	INC	DI
F26_01:	INC	DI
F26_02:	MOV	BX,DI
	JMP	F27_06

F27_00:	MOV	DL,ES:DosCol		; ^Enter
	MOV	DH,0
	MOV	SI,DX
	ADD	SI,OFFSET DosBuf
	CMP	ES:DosBuf,0
	JE	F27_05
	MOV	DI,ES:HstAdr
	CMP	DL,0
	JE	F27_03
	MOV	AL,ES:[SI-1]
	CALL	CmpSym
	JE	F27_05
	CMP	AL,'@'
	JE	F27_05
	JMP	F27_03
F27_01:	CMP	BYTE PTR ES:[DI],0
	JE	F27_04
	MOV	BX,DI
	MOV	SI,OFFSET DosBuf
F27_02:	LODS	ES:Byt
	CMP	AL,0
	JE	F27_06
	CALL	UpCase
	MOV	AH,AL
	MOV	AL,ES:[DI]
	INC	DI
	CALL	UpCase
	CMP	AL,AH
	JE	F27_02
F27_03:	MOV	CX,256
	MOV	AL,0
	REPNE	SCASB
	JMP	F27_01
F27_04:	CMP	DL,0
	JNE	F27_11
F27_05:	CALL	FilePan
	JNC	F27_11
	MOV	SI,DS:CurAdr
	CMP	SI,DS:EndBuf
	JAE	F27_11
	MOV	DI,OFFSET TmpBuf
	MOV	BX,DI
	MOV	CX,13
	REP	MOVSB
	JMP	F27_07
F27_06:	MOV	ES:HstAdr,BX
	XOR	DX,DX
	MOV	WORD PTR ES:DosBuf,DX
F27_07:	MOV	DI,BX
	MOV	SI,OFFSET DosBuf
	MOV	CX,LenCmd-1
	MOV	AH,0
F27_08:	MOV	AL,ES:[BX]
	CMP	AL,0
	JE	F27_09
	INC	BX
	CALL	EdLin
	JC	F27_08
	JMP	F27_10
F27_09:	CMP	DI,OFFSET TmpBuf
	JNE	F27_10
	MOV	AL,' '
	CALL	EdLin
F27_10:	MOV	ES:DosCol,DL
	CALL	DosCom
F27_11:	JMP	Func05

; Putting the selected files into the command line is C: F80_00_c of
; port/cmdline.c, which takes the panel and answers 1 if it has put any
; there, and the panel is to be drawn anew.
F80_00:	MOV	AX,DS			; ^I - insert files
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR F80_00_c
	POP	DS
	OR	AX,AX
	JNE	F80_10
	JMP	Func05
F80_10:	JMP	Func01

	.CONST
Name_S	DB	PrgName,0
Quit1_S	DB	' The ^I ',0
Quit2_S	DB	'Do you want to quit the ^I?',0
YesNo_S	DB	'Yes',0FFh,3,'No',0

	.DATA
YesNo_D	DB	5, 0
	DB	4, 6
	DB	0
Quit_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	3, 0
	DW	OFFSET WinBuf
	DW	OFFSET Quit1_S
	DW	OFFSET Quit2_S
	DW	OFFSET YesNo_S
Quit_B	BOXHD	< Quit_P, 528h, 0, 1, 2, 26, 0 >
	BOXMN	< 4, 0,0,0,0, 323h, TmpBuf, YesNo_D>
	DB	0

	.CODE
F30_00:	CMP	ES:QuitAck,0		; F10 - quit
	JZ	F30_01
	MOV	DX,OFFSET Name_S
	MOV	BP,OFFSET Quit_B
	CALL	Dialog
	JE	F30_01
	JMP	Func04
F30_01:	JMP	Exit

; Texts and a box that the C code of the user menu reads here, and the name
; of the list file.
	PUBLIC	Menu_F, User_S, UsrEr_B, List_F
	.CONST
Menu_F	DB	'VC.MNU',0
User_S	DB	' User Menu ',0
User1_S	DB	'Could not find the menu file',0

	.DATA
UsrEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	OFFSET ErrBuf
	DW	OFFSET TmpBuf
	DB	7, 64
	DW	OFFSET User_S
	DW	OFFSET User1_S
	DW	OFFSET Var_S
	DW	OFFSET Ok_S
UsrEr_B	BOXHD	< UsrEr_P, 528h, 0, 1, 0FFh, 0, 2 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpBuf, OkMn_D>
	DB	0

	.CODE
; The user menu is C: F32_00_c of port/usermenu.c, which takes 1 for the
; main menu. It answers 1 if a line was chosen that has a command to run,
; and Run_00_c has put that into DosBuf then.
F42_00:	MOV	AX,1			; Shift-F2 - main user menu
	JMP	SHORT F32_0A
F32_00:	XOR	AX,AX			; F2 - user menu
F32_0A:	PUSH	ES
	POP	DS
	CALL	FAR PTR F32_00_c
	JMP	Run_0A

; The box that asks for the drive of a panel is C: F62_01_c of
; port/drive.c, which takes the panel and the item of the menu bar, and
; answers 0 if no drive was chosen.
F61_00:	PUSH	DS			; Alt F1
	MOV	AX,ES:WCB1seg
	MOV	DX,1
	JMP	SHORT F62_01

F62_00:	PUSH	DS			; Alt F2
	MOV	AX,ES:WCB2seg
	MOV	DX,5
F62_01:	PUSH	ES
	POP	DS
	PUSH	AX
	PUSH	DX
	CALL	FAR PTR F62_01_c
	POP	CX			; The item, for M06_10
	POP	DX
	OR	AX,AX
	JE	F62_46
	MOV	DS,DX			; The panel
	JMP	M06_10
F62_46:	POP	DS
	JMP	Func04

; The history in its box is C: F68_00_c of port/cmdline.c, which answers 1
; if the command that it has put into the command line is to be run.
F68_00:	PUSH	ES			; Alt F8 - History
	POP	DS
	CALL	FAR PTR F68_00_c
	OR	AX,AX
	JNE	F68_29
	JMP	Func04
F68_29:	JMP	F21_25

; Putting the path of a panel into the command line is C: F82_01_c, which
; takes the panel and the key.
F81_00:	MOV	DX,ES:WCB1seg		; ^[ - insert left path
	JMP	SHORT F82_01

F82_00:	MOV	DX,ES:WCB2seg		; ^] - insert right path
F82_01:	PUSH	ES
	POP	DS
	XCHG	AX,DX
	CALL	FAR PTR F82_01_c
	JMP	Func04

; Esc is C: F74_00_c, which answers 1 if it is to switch the panels off
; or on, and has emptied the command line otherwise.
F74_00:	PUSH	DS			; Esc
	PUSH	ES
	POP	DS
	CALL	FAR PTR F74_00_c
	POP	DS
	OR	AX,AX
	JE	F73_01
	JMP	F11_00

; A key for the command line is C: F73_00_c.
F73_00:	PUSH	DS			; Input command line
	PUSH	ES
	POP	DS
	CALL	FAR PTR F73_00_c
	POP	DS
F73_01:	JMP	Func05

F75_00:	CMP	DS:WinTyp,1		; Auto CD / Quick view
	JNE	F75_01
	JMP	F21_10
F75_01:	JMP	Func04

; The scan codes of Alt with a digit or a letter, which begin the speed
; search: the main loop looks for a key here. The C code of the search,
; in port/cmdline.c, has them too.
	.DATA
AltSr_T	DB	78h, 79h, 7Ah, 7Bh, 7Ch, 7Dh, 7Eh, 7Fh, 80h, 81h, 82h, 83h
	DB	10h, 11h, 12h, 13h, 14h, 15h, 16h, 17h, 18h, 19h
	DB	1Eh, 1Fh, 20h, 21h, 22h, 23h, 24h, 25h, 26h
	DB	2Ch, 2Dh, 2Eh, 2Fh, 30h, 31h, 32h

	.CODE
; The speed search is C: F77_00_c, which takes the key and the panel, and
; answers with the key that ended the search, to be acted on, and in DX
; with where its box was. BX, DX, SI and CX are then as the routine left
; them; DI and BP are not.
F77_00:	PUSH	DS			; Speed search
	MOV	DX,DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR F77_00_c
	POP	DS
	MOV	BX,DX
	MOV	DX,318h
	MOV	SI,OFFSET WinBuf
	MOV	CL,ES:KeyBar
	MOV	CH,0
	JMP	Func12

; The piece of the speed search that is still assembly, for the C code to
; call: Search has the key AX act on what has been typed and looks for the
; next name that begins with it, among the files of the panel CX or in
; the tree, and F00_50 puts the cursor there. Both take the panel in DS.
	PUBLIC	F77_10_a
F77_10_a PROC	FAR
	PUSH	BP
	PUSH	DX
	PUSH	BX
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	DS,CX
	PUSH	DS
	MOV	SI,DS:CurAdr
	MOV	DI,DS:First
	MOV	BP,DS:EndBuf
	MOV	BX,OFFSET DskBuf
	MOV	DX,SIZE FilEnt
	CMP	DS:WinTyp,0
	JE	F77_11
	PUSH	ES
	POP	DS
	MOV	SI,DS:TreAdr
	MOV	DI,DS:TreFrst
	MOV	BP,DS:TreeBuf.TreeLen
	MOV	CL,4
	SHL	BP,CL
	MOV	BX,OFFSET TreeBuf.TreList
	ADD	BP,BX
	MOV	DX,16
F77_11:	PUSH	DI
	CALL	Search
	POP	DI
	POP	DS
	JZ	F77_12
	CALL	F00_50
F77_12:	POP	DS
	POP	BX
	POP	DX
	POP	BP
	RET
F77_10_a ENDP

	.CONST
List_F	DB	'vc.lst',0

	.CODE
; The commands of a line of a menu or of an extension file are made by C:
; Run_00_c of port/usermenu.c, which answers 1 if there is one to run.
;    SI - where they begin, in MenuBuf
;    BP - not 0 for the file that is viewed or edited
Run_00:	PUSH	ES
	POP	DS
	MOV	AX,SI
	MOV	DX,BP
	CALL	FAR PTR Run_00_c
Run_0A:	OR	AX,AX
	JNE	Run_0B
	JMP	Func04
Run_0B:	JMP	F21_30

; Entries for the C code of this file's commands to routines of files
; that are being translated elsewhere (see FromC in VC.ASM, and
; port/usermenu.c and port/cmdline.c for what they take and return).
	FromC	DosCom

; FndItem, F34_30 and F34_40 take a place in MenuBuf in SI and hand one
; back, here in DX, and AX is not 0 if they set the carry flag. F34_40
; takes in CX what it has in BP.
	PUBLIC	FndItem_a, F34_30_a, F34_40_a
FndItem_a PROC	FAR
	PUSH	DS
	POP	ES
	CALL	FndItem
	MOV	DX,SI
	SBB	AX,AX
	RET
FndItem_a ENDP

F34_30_a PROC	FAR
	PUSH	DS
	POP	ES
	CALL	F34_30
	MOV	DX,SI
	SBB	AX,AX
	RET
F34_30_a ENDP

F34_40_a PROC	FAR
	PUSH	BP
	PUSH	DS
	POP	ES
	MOV	BP,CX
	CALL	F34_40
	MOV	DX,SI
	SBB	AX,AX
	POP	BP
	RET
F34_40_a ENDP

; Input0, and DX is not 0 if it set the carry flag.
	PUBLIC	Input0_a
Input0_a PROC	FAR
	PUSH	DS
	POP	ES
	CALL	Input0
	SBB	DX,DX
	RET
Input0_a ENDP

; ClsLine, with in AX what it is to have in DS.
	PUBLIC	ClsLine_a
ClsLine_a PROC	FAR
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	DS,AX
	CALL	ClsLine
	POP	DS
	RET
ClsLine_a ENDP
