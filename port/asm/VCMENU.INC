;-------------------------------------------------------
;		Version :	10.01.2000
;-------------------------------------------------------

	.DATA
	ASSUME	CS:DGROUP
; The commands of the menus: for the menu bar, which is C, the stub below
; jumps by these. Menu1_T is for the Left and the Right menu, MenuS_T and
; MenuC_T are the first eight commands of the Files menu with Shift and
; with Ctrl held down.
Menu_T	DW	Menu1_T
	DW	Menu2_T
	DW	Menu3_T
	DW	Menu4_T
	DW	MenuS_T
	DW	MenuC_T
Menu1_T	DW	M01_00			; Brief
	DW	M01_00			; Full
	DW	M03_00			; Info
	DW	M03_00			; Tree
	DW	M06_00			; On/Off
	DW	0
	DW	M08_00			; Name
	DW	M08_00			; Extension
	DW	M08_00			; Time
	DW	M08_00			; Size
	DW	M08_00			; Unsorted
	DW	0
	DW	F17_01			; Re-read
	DW	M16_01			; Filter...
	DW	F62_01			; Drive...
Menu2_T	DW	M21_00			; Help
	DW	F32_00			; Menu
	DW	F33_00			; View
	DW	F34_00			; Edit
	DW	F35_00			; Copy
	DW	F36_00			; Rename or move
	DW	F37_00			; Make directory
	DW	F38_00			; Delete
	DW	0
	DW	M30_00			; File attributes
	DW	0
	DW	F09_00			; Gray +
	DW	F10_00			; Gray -
	DW	F76_01			; Gray *
	DW	F78_00			; ^M
	DW	0
	DW	F30_00			; Quit
Menu3_T	DW	M41_00			; Tree
	DW	F54_00			; Volume label
	DW	F65_00			; Memory info
	DW	M42_00			; Directory sizes
	DW	F67_00			; Find file
	DW	F68_00			; History
	DW	F69_00			; EGA lines
	DW	0
	DW	F15_00			; Swap panels
	DW	F11_00			; Panels on/off
	DW	M49_00			; Compare directories
	DW	0
	DW	M53_00			; Menu file edit
Menu4_T	DW	M60_00			; Configuration...
	DW	M61_00			; Advanced options...
	DW	M62_00			; Extension file edit...
	DW	M63_00			; Viewer...
	DW	M64_00			; Editor...
	DW	0
	DW	M66_00			; Auto menus
	DW	M67_00			; Path prompt
	DW	F16_00			; Key bar
	DW	M69_00			; Full screen
	DW	M70_00			; Auto directory sizes
	DW	M71_00			; Mini status
	DW	M72_00			; Clock
	DW	M73_00			; Memory location
	DW	0
	DW	M75_00			; Save setup
MenuS_T	DW	M21_00			; Help
	DW	F42_00			; Menu
	DW	F43_00			; View...
	DW	F44_00			; Edit...
	DW	F45_00			; Copy...
	DW	F46_00			; Rename or move...
	DW	F47_00			; Make directory...
	DW	F48_00			; Delete...
MenuC_T	DW	M21_00			; Help
	DW	F32_00			; Menu
	DW	F33_00			; View...
	DW	F34_00			; Edit...
	DW	F55_00			; Copy...
	DW	F56_00			; Rename or move...
	DW	F37_00			; Make directory...
	DW	F58_00			; Delete...

	.CODE
; The menu bar and its pull-down menus are C: F39_00_c of port/menu.c,
; which takes the keys and the mouse until an item is chosen or the bar is
; left. It gets in AX 1 for Shift-F10, which goes back to the last item,
; in DX the current panel and in SI the text of the bar; and it hands back
; in AX the menu and the item, as the commands have them in CL and CH, in
; DH the number of the table above that has the command, or 0 if no item
; was chosen, and in DL what the bar left there. A command of the Left or
; the Right menu gets that panel in DS and the current one on the stack.
; The commands get every register as the bar had it at this jump, but for
; BP, which none of them reads: Rename goes by BX and DX as they come.
F40_00:	MOV	AX,1			; Shift F10 - pull down menu
	JMP	SHORT F39_01
F39_00:	XOR	AX,AX			; F9 - pull down menu
F39_01:	MOV	DX,DS
	MOV	SI,OFFSET PtMnu_S
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR F39_00_c
	PUSH	DS
	POP	ES
	POP	DS
	MOV	CX,AX
	OR	DH,DH
	JNZ	F39_86
	JMP	Func04
F39_86:	MOV	AH,CH
	MOV	AL,DH
	MOV	DH,1
	MOV	DI,ES:WCB1seg
	CMP	CL,5
	JNE	F39_87
	MOV	DI,ES:WCB2seg
F39_87:	CMP	AL,1
	JNE	F39_90
	PUSH	DS
	MOV	DS,DI
F39_90:	MOV	BL,AL
	ADD	BL,BL
	MOV	BH,0
	MOV	SI,CS:Menu_T[BX-2]
	MOV	BL,AH
	DEC	BX
	ADD	BL,BL
	JMP	WORD PTR CS:[BX+SI]

M01_00:	DEC	CH			; Brief / Full
	MOV	CL,DS:SortTyp
M01_01:	CALL	HidCur
	CMP	DS:WinTyp,0
	JE	M01_03
	MOV	BP,DS
	CMP	BP,ES:WCBcrn
	JNE	M01_02
	CALL	M02_10
	JNC	M01_02
	POP	DS
	JMP	Init50
M01_02:	MOV	DS:WinTyp,0
	MOV	BYTE PTR DS:ChgFlg,1
M01_03:	MOV	AL,CL
	XCHG	DS:SortTyp,AL
	MOV	DS:BrfFul,CH
	CMP	AL,CL
	JE	M01_04
	CMP	BYTE PTR DS:ChgFlg,0
	JNZ	M01_04
	JMP	M16_11
M01_04:	JMP	M03_05

M02_10	PROC	NEAR
	PUSH	AX
	MOV	AL,DS:WinPath
	SUB	AL,'A'-1
	CALL	Fantom
	JC	M02_12
	MOV	DX,OFFSET WinPath
	MOV	AH,3Bh
	CALL	Intr21
	JNC	M02_11
	CMP	AX,WORD PTR 3
	STC
	JNE	M02_12
M02_11:	MOV	DL,DS:WinPath
	SUB	DL,'A'
	MOV	AH,0Eh
	INT	21h
	CALL	SetPth
	PUSHF
	CALL	PutPrm
	POPF
M02_12:	POP	AX
	RET
M02_10	ENDP

M03_00:	CALL	HidCur			; Info / Tree
	MOV	CL,5
	SUB	CL,CH
	JNE	M03_01
	MOV	CL,3
M03_01:	CMP	DS:Visible,0
	JZ	M03_02
	CMP	CL,DS:WinTyp
	JE	M03_04
M03_02:	MOV	DI,DS
	CMP	DI,ES:WCBcrn
	JNE	M03_03
	CALL	InvWCB
	CALL	FilePan
	JNC	M03_03
	CALL	M02_10
	JNC	M03_03
	POP	DS
	JMP	Func04
M03_03:	MOV	DS,DI
	MOV	AL,DS:WinTyp
	CMP	AL,CL
	JE	M03_04
	MOV	DS:Ctrl_L,AL
M03_04:	MOV	DS:WinTyp,CL
	MOV	DS:ChgFlg,1
	CMP	CL,1
	JNE	M03_05
	MOV	ES:CrnPath,0
M03_05:	MOV	AX,DS
	MOV	CL,1
	CMP	AX,ES:WCB1seg
	JE	M03_06
	MOV	CL,5
M03_06:	JMP	M06_10

M06_00:	MOV	AL,0			; Panel on/off
	CMP	AL,DS:Visible
	JZ	M06_10
	MOV	DS:Visible,AL		; Panel off
	CALL	BegBX
	CALL	SizDX
	CALL	ResUser
	MOV	ES:Ctrl_O,AL
	MOV	DI,DS
	MOV	SI,ES:WCBcrn
	CMP	SI,DI
	JNE	M06_01
	MOV	DS,ES:WCBsec
	CMP	AL,DS:Visible
	JZ	M06_01
	MOV	DS,SI
	MOV	AL,CL
	CALL	PutMnu
	CALL	F20_02
	CALL	PutPath
	CALL	PutFls
M06_01:	POP	DS
	JMP	Func02
M06_10:	CALL	HidCur			; Panels On
	MOV	DS:Visible,1
	MOV	DI,DS
	MOV	AX,ES:WCBcrn
	CMP	AX,DI
	JE	M06_11
	MOV	DS,AX
	CMP	DS:Visible,0
	MOV	DS,DI
	JNZ	M06_11
	POP	DS
	MOV	AL,CL
	CALL	PutMnu
	PUSH	CX
	CALL	F20_02
	POP	CX
	PUSH	DS
M06_11:	CMP	BYTE PTR DS:ChgFlg,0
	JZ	M06_12
	MOV	AL,CL
	CALL	PutMnu
	CALL	TstInfo
M06_12:	POP	DS
	JMP	Init50

M08_00:	MOV	CL,CH			; Sort
	SUB	CL,7
	MOV	CH,DS:BrfFul
	JMP	M01_01

	.CONST
Filt0_S	DB	' Filter ',0
Filt1_S	DB	'Select files to display',0
Filt2_S	DB	'Custom:',0FFh,14,0
Filt3_S	DB	'[ ] Hidden files',0FFh,5,0
Filt4_S	DB	'[ ] Executable files ',0
OkCn_S  DB	'[ Ok ]',0FFh,3,'[ Cancel ]',0

	.DATA
Filt_P	DB	Box_C, BoxCu_C, Box_C, 0
	DB	7, 0
	DW	OFFSET WinBuf
	DW	OFFSET Filt0_S
	DW	OFFSET Filt1_S
	DW	OFFSET Filt2_S
	DW	OFFSET Filt3_S
	DW	OFFSET Filt4_S
	DW	0
	DW	OFFSET OkCn_S
Filt_B	BOXHD	< Filt_P, 514h, 0, 1, 1, 14, 0 >
	BOXMN	< 1, 3,1,0,0, 30Eh, DskMas  , 20Ch   >	; Custom
	BOXMN	< 3, 0,2,1,1, 407h, TmpBuf  , Hidden >	; Hidden files
	BOXMN	< 3, 1,3,2,2, 507h, TmpBuf+1, MasTyp >	; Executable files
	BOXMN	< 4, 2,0,3,3, 70Ah, TmpBuf+2, OkCn_D >	; Ok / Cancel
	DB	0
OkCn_D	DB	 6, 0
	DB	10, 9
	DB	 0

	.CODE
M16_00:	CALL	FilePan			; Filter...
	JNC	M16_02
	PUSH	DS
M16_01:	MOV	BP,OFFSET Filt_B
	CALL	Dialog
	JE	M16_11
	POP	DS
M16_02:	JMP	Func04

M16_10:	CALL	FilePan			; Ctrl-H - hidden files
	JNC	M16_02
	PUSH	DS
	PUSH	ES
	PUSH	DS
	POP	ES
	MOV	BX,OFFSET Hidden
	CALL	InvVar
	POP	ES
M16_11:	CALL	M16_20
	JMP	M03_05

M16_20	PROC	NEAR
	MOV	SI,DS:CurAdr
	MOV	DI,OFFSET MenuBuf
	MOV	CX,13
	PUSH	CX
	REP	MOVSB
	POP	CX
	MOV	SI,DS:First
	REP	MOVSB
	MOV	BX,DS:EndBuf
	CALL	SetMask
	CALL	Sort
	MOV	CX,OFFSET DskBuf
	MOV	DX,CX
	CMP	BX,CX
	JBE	M16_13
	MOV	DI,OFFSET MenuBuf
	CALL	FnFile
	JNC	M16_12
	MOV	CX,SI
M16_12:	MOV	DI,OFFSET MenuBuf+13
	CALL	FnFile
	JNC	M16_13
	MOV	DX,SI
M16_13:	MOV	DS:CurAdr,CX
	MOV	DS:First,DX
	RET
M16_20	ENDP

M21_00:	CALL	Help			; Help
	JMP	Func04

M41_00:	CALL	PutMnu3			; Alt F10 - Tree
	MOV	ES:HlpPage,28
	MOV	AL,16
	CALL	ChTree
	JC	M41_01
	PUSH	DS
	PUSH	ES
	MOV	DX,OFFSET TmpBuf
	JMP	F24_01
M41_01:	JMP	Init50

	.CONST
DiSi1_S	DB	'Scanning sizes of subdirectories',0
DiSi2_S	DB	'on drive ^V:',0

	.DATA
DiSiz_P	DB	Box_C, 0, 0, 0
	DB	3, 1
	DW	OFFSET WinBuf
	DW	TmpBuf
	DB	0, 25
	DW	OFFSET Tree_S
	DW	OFFSET DiSi1_S
	DW	OFFSET DiSi2_S

	.CODE
M42_00:	CALL	FilePan			; Alt F6 - Directory sizes
	JNC	M42_01
	MOV	AL,DS:WinPath[0]
	MOV	AH,0
	PUSH	DS
	PUSH	ES
	POP	DS
	MOV	WORD PTR ES:TmpBuf[0],AX
	CALL	PutMnu3
	MOV	BX,528h
	MOV	BP,OFFSET DiSiz_P
	CALL	DialBox
	POP	DS
	CALL	Window
	PUSH	SI
	PUSH	DX
	PUSH	BX
	CALL	DirSize
	CALL	M16_20
	POP	BX
	POP	DX
	POP	SI
	CALL	ResWin
	CALL	Window
M42_01:	JMP	Init50

F69_00:	CMP	ES:TstEGA,0		; Alt F9 - EGA lines
	JNE	F69_01
	JMP	Func04
F69_01:	CALL	Quit
	MOV	BH,DS:DosLin
	CALL	ClsLine
	CALL	SavMous
	MOV	AL,0
	CALL	CurMous
	MOV	DS:BarPrev,0FFh
	MOV	DS:ScrPage,0
	PUSH	ES
	MOV	BH,0
	MOV	CL,16
	MOV	AX,1130h
	INT	10h
	PUSH	CX
	MOV	AL,DS:ScrMod
	MOV	AH,0
	INT	10h
	POP	AX
	CMP	AL,14
	JB	F69_03
	MOV	AX,1112h
	INT	10h
	MOV	AH,12h
	MOV	BL,20h
	INT	10h
F69_03:	MOV	DL,0
	MOV	BH,0
	MOV	AX,1130h
	INT	10h
	POP	ES
	MOV	AL,25
	OR	DL,DL
	JE	F69_04
	INC	DX
	MOV	AL,MaxLins
	CMP	AL,DL
	JBE	F69_04
	MOV	AL,DL
F69_04:	MOV	DL,AL
	XCHG	AL,DS:Lines
	CLD
	MOV	DI,OFFSET ScrBuf
	MOV	AH,2*80
	CMP	AL,DL
	JB	F69_06
	JE	F69_07
	SUB	AL,DL
	SUB	DS:DosLin,AL
	JA	F69_05
	MOV	DS:DosLin,1
F69_05:	MUL	AH
	MOV	SI,DI
	ADD	SI,AX
	MOV	AL,80
	MUL	DL
	MOV	CX,AX
	REP	MOVSW
	JMP	F69_07
F69_06:	SUB	DL,AL
	MUL	AH
	ADD	DI,AX
	MOV	AL,80
	MUL	DL
	MOV	CX,AX
	MOV	AH,DS:CrntCol
	MOV	AL,' '
	REP	STOSW
F69_07:	MOV	SI,OFFSET ScrBuf
	MOV	DI,OFFSET UserScr
	MOV	CX,(MaxLins-1)*80
	REP	MOVSW
	CALL	ScrOffs
	CMP	DS:Mouse,0
	JE	F69_09
	MOV	AL,DS:Lines
	CBW
	MOV	CL,3
	SHL	AX,CL
	DEC	AX
	CMP	DS:MouseY,AX
	JBE	F69_08
	MOV	DS:MouseY,AX
F69_08:	CALL	IniMous
F69_09:	JMP	M60_02

	.CONST
Cmp0_S	DB	' Compare ',0
Cmp1_S	DB	'The two directories appear',0
Cmp2_S	DB	'to be identical.',0

	.DATA
CmpDi_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 0
	DW	OFFSET WinBuf
	DW	OFFSET Cmp0_S
	DW	OFFSET Cmp1_S
	DW	OFFSET Cmp2_S
	DW	OFFSET Ok_S
CmpDi_B	BOXHD	< CmpDi_P, 728h, 0, 1, 3, 37, 0 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpBuf, OkMn_D >
	DB	0

	.CODE
M49_00:	CALL	FilePan			; Compare directories
	JNC	M49_01
	CALL	InvWCB
	CALL	FilePan
	JNC	M49_01
	MOV	CL,0
	CALL	M49_10
	CALL	InvWCB
	CALL	M49_10
	CMP	CL,0
	JNE	M49_02
	MOV	BP,OFFSET CmpDi_B
	CALL	Dialog
M49_01:	JMP	Func04
M49_02:	JMP	M60_02

M49_10	PROC	NEAR
	MOV	SI,OFFSET DskBuf
M49_11:	CMP	SI,DS:EndBuf
	JAE	M49_14
	TEST	BYTE PTR DS:[SI+13],10h
	JNZ	M49_13
	PUSH	DS
	PUSH	SI
	MOV	DI,OFFSET TmpBuf
	PUSH	DI
	PUSH	CX
	MOV	CX,13
	REP	MOVSB
	POP	CX
	POP	DI
	CALL	InvWCB
	CALL	FnFile
	MOV	AL,DS:[SI+13]
	MOV	BX,DS:[SI+14]
	MOV	DX,DS:[SI+16]
	POP	SI
	POP	DS
	JNC	M49_12
	TEST	AL,10h
	JNZ	M49_12
	CMP	DX,DS:[SI+16]
	JB	M49_12
	JA	M49_13
	CMP	BX,DS:[SI+14]
	JAE	M49_13
M49_12:	OR	BYTE PTR DS:[SI+13],40h
	MOV	CL,1
M49_13:	ADD	SI,SIZE FilEnt
	JMP	M49_11
M49_14:	RET
M49_10	ENDP

	.CONST
Cnf00_S	DB			' Configuration ',0
Cnf01_S	DB	' Ú ^bScreen colors^b ',0FFh,80h+6,'Ä¿  Ú ^bPanel options^b ',0FFh,80h+16,'Ä¿ ',0
Cnf02_S	DB	' ³  ( ) Black & White  ³^IIns moves down',0FFh,11,'³ ',0
Cnf03_S	DB	' ³  ( ) Color',0FFh,10,'³^IAuto change directory',0FFh,4,'³ ',0
Cnf04_S	DB	' ³  ( ) Laptop',0FFh,9,'³  À',0FFh,80h+31,'ÄÙ ',0
Cnf05_S	DB	' À',0FFh,80h+21,'ÄÙ  Ú ^bOther options^b ',0FFh,80h+16,'Ä¿ ',0
Cnf06_S	DB	' Ú ^bScreen blank delay^b Ä¿^IMenu bar always visible  ³ ',0
Cnf07_S	DB	' ³  ( ) 40 minutes',0FFh,5,'³^IAuto save setup',0FFh,10,'³ ',0
Cnf08_S	DB	' ³  ( ) 20 minutes',0FFh,5,'³^IConfirmation on quit',0FFh,5,'³ ',0
Cnf09_S	DB	' ³  ( )  5 minutes',0FFh,5,'³^IZooming boxes',0FFh,12,'³ ',0
Cnf10_S	DB	' ³  ( )  3 minutes',0FFh,5,'³  À',0FFh,80h+31,'ÄÙ ',0
Cnf11_S	DB	' ³  ( )  1 minute',0FFh,6,'³  Ú ^bMouse options^b ',0FFh,80h+16,'Ä¿ ',0
Cnf12_S	DB	' ³  ( ) Off',0FFh,12,'³^ILeft-handed mouse',0FFh,8,'³ ',0
Cnf13_S	DB	' À',0FFh,80h+21,'ÄÙ  À',0FFh,80h+31,'ÄÙ ',0
Cnf14_S	DB	'Press ^r Space ^r to change an option, ^r  ^r and ^r  ^r',0
Cnf15_S	DB			'to move between options',0
Chk1_S	DB	'  '
Chk2_S	DB	'³  [ ] ',0

	.DATA
Conf_P	DB	Conf_C, CnfRe_C, CnfBo_C, 0
	DB	18, 0
	DW	WinBuf
	DW	Cnf00_S
	DW	Cnf01_S
	DW	Cnf02_S
	DW	Cnf03_S
	DW	Cnf04_S
	DW	Cnf05_S
	DW	Cnf06_S
	DW	Cnf07_S
	DW	Cnf08_S
	DW	Cnf09_S
	DW	Cnf10_S
	DW	Cnf11_S
	DW	Cnf12_S
	DW	Cnf13_S
	DW	Cnf14_S
	DW	Cnf15_S
	DW	0
	DW	OkCn_S
Conf_B	BOXHD	< Conf_P, 228h, 0, 1, 4, 40, 0 >
	BOXMN	<2, 9,81h, 0, 2,  30Ah, TmpBuf   , 3 >	   ; Screen colors
	BOXMN	<2,0C0h,9, 1, 4,  80Ah, TmpBuf+2 , 6 >	   ; Screen blank delay
	BOXMN	<3, 9, 3,80h, 2,  323h, TmpBuf+4 , InsDwn> ; Ins moves down
	BOXMN	<3, 2, 4,80h, 3,  423h, TmpBuf+5 , AutoCD> ; Auto change dir
	BOXMN	<3, 3, 5,81h, 4,  723h, TmpBuf+6 , MenuVs> ; Menu bar visible
	BOXMN	<3, 4, 6,81h, 5,  823h, TmpBuf+7 , AutoSav>; Auto save setup
	BOXMN	<3, 5, 7,81h, 6,  923h, TmpBuf+8 , QuitAck>; Confirmation on quit
	BOXMN	<3, 6, 8,81h, 7, 0A23h, TmpBuf+9 , ZoomWin>; Zooming boxes
	BOXMN	<3, 7, 9,81h, 8, 0D23h, TmpBuf+10, LftMous>; Left-handed mouse
	BOXMN	<4, 8,80h, 9, 9, 121Eh, TmpBuf+11, OkCn_D> ; Ok / Cancel
	DB	0
Delay_T	DB	40, 20, 5, 3, 1, 0

	.CODE
M60_00:	PUSH	ES			; Configuration
	POP	DS
	MOV	DI,OFFSET TmpBuf
	MOV	AL,DS:ColTyp
	STOSB
	STOSB
	MOV	AH,DS:BlDelay
	MOV	SI,OFFSET Delay_T-1
	MOV	AL,0-1
M60_01:	INC	AL
	INC	SI
	CMP	AH,CS:[SI]
	JB	M60_01
	STOSB
	STOSB
	MOV	CL,DS:MenuVs
	MOV	DX,OFFSET Chk1_S
	MOV	BP,OFFSET Conf_B
	CALL	Dialog
	JNE	M61_01
	MOV	SI,OFFSET TmpBuf
	LODSW
	MOV	DS:ColTyp,AL
	LODSW
	CBW
	MOV	BX,AX
	MOV	AL,CS:Delay_T.[BX]
	MOV	DS:BlDelay,AL
	XOR	BX,BX
	MOV	DH,DS:Lines
	SUB	DH,2
	MOV	DL,80
	CALL	ResUser
	MOV	AL,0
	CALL	PutMnu
M60_02:	CALL	PutPrm
	CMP	DS:Visible,0
	JZ	M61_01
	JMP	Init50

	.CONST
Set00_S	DB		' Advanced Options '		 ,0
Set01_S	DB	'Ú ^bKey configuration^b ',0FFh,80h+16,'Ä¿',0
Set02_S	DB	'^IAlt alone selects menu',0FFh,7,'³',0
Set03_S	DB	'^IEscape toggles panels on/off ³',0
Set04_S	DB	'^IAlternative viewer',0FFh,11,'³',0
Set05_S	DB	'^IAlternative editor',0FFh,11,'³',0
Set07_S	DB	'Ú ^bOther options^b ',0FFh,80h+20,'Ä¿',0
Set08_S	DB	'^IQuick execute commands',0FFh,7,'³',0
Set09_S	DB	'^IClear keyboard buffer',0FFh,8,'³',0
Set10_S	DB	' À',0FFh,80h+35,'ÄÙ ',0
Set11_S	DB	'Press ^r Space ^r to change an option,' ,0
Set12_S	DB	'arrows to move between options',0

	.DATA
Sets_P	DB	Conf_C, CnfRe_C, CnfBo_C, 0
	DB	15, 0
	DW	WinBuf
	DW	Set00_S
	DW	Set01_S
	DW	Set02_S
	DW	Set03_S
	DW	Set04_S
	DW	Set05_S
	DW	Set10_S
	DW	Set07_S
	DW	Set08_S
	DW	Set09_S
	DW	Set10_S
	DW	Set11_S
	DW	Set12_S
	DW	0
	DW	OkCn_S
Sets_B	BOXHD	< Sets_P, 328h, 0, 1, 4, 41, 0 >
	BOXMN	<3, 6, 1, 0, 0, 30Ah, TmpBuf+0, AltBar > ; Alt selects menu
	BOXMN	<3, 0, 2, 1, 1, 40Ah, TmpBuf+1, EscBar > ; Esc toggles panels
	BOXMN	<3, 1, 3, 2, 2, 50Ah, TmpBuf+2, AltView> ; Alternative viewer
	BOXMN	<3, 2, 4, 3, 3, 60Ah, TmpBuf+3, AltEdit> ; Alternative editor
	BOXMN	<3, 3, 5, 4, 4, 90Ah, TmpBuf+4, ExecTyp> ; Quick execute cmds
	BOXMN	<3, 4, 6, 5, 5,0A0Ah, TmpBuf+5, ClrBuf > ; Clear buffer
	BOXMN	<4, 5, 0, 6, 6,0F1Eh, TmpBuf+6, OkCn_D > ; Ok / Cancel
	DB	0

	.CODE
M61_00:	PUSH	ES			; Advanced options
	POP	DS
	MOV	DX,OFFSET Chk2_S
	MOV	BP,OFFSET Sets_B
	CALL	Dialog
M61_01:	JMP	Func04

	.CONST
ExFl_S	DB	' Extension File ',0
MnLc1_S	DB	'Do you wish to edit the main,',0
MnLc2_S	DB	'or a local ^I?',0
UsMnu_S	DB	'user menu',0
ExtFl_S	DB	'extension file',0
MnLc3_S	DB	'Main',0FFh,3,'Local',0FFh,3,'Cancel',0

	.DATA
MnLoc_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 1
	DW	OFFSET WinBuf
	DW	TmpBuf
	DB	0, 50
	DW	OFFSET Var_S
	DW	OFFSET MnLc1_S
	DW	OFFSET MnLc2_S
	DW	OFFSET MnLc3_S
MnLoc_B	BOXHD	< MnLoc_P, 528h, 0, 1, 0FFh, 0, 0 >
	BOXMN	< 4, 0,0,0,0, 41Ch, TmpBuf, MnLoc_D>
	DB	0
MnLoc_D	DB	6, 0
	DB	7, 7
	DB	8,15
	DB	0

	.CODE
M53_00:	MOV	BX,OFFSET Menu_F	; Menu file edit
	MOV	SI,OFFSET User_S
	MOV	DX,OFFSET UsMnu_S
	MOV	AL,38
	JMP	M64_02

M62_00:	MOV	BX,OFFSET Ext_F		; Extension file edit
	MOV	AL,42
	JMP	M64_01

M63_00:	MOV	BX,OFFSET ViewE_F	; Viewer file
	MOV	AL,43
	JMP	M64_01

M64_00:	MOV	BX,OFFSET EditE_F	; Editor file
	MOV	AL,44
M64_01:	MOV	SI,OFFSET ExFl_S
	MOV	DX,OFFSET ExtFl_S
M64_02:	MOV	ES:HlpPage,AL
	PUSH	CX
	MOV	AL,CL
	CALL	PutMnu
	PUSH	CS
	POP	DS
	MOV	DI,OFFSET TmpBuf
	MOV	CX,80
	REP	MOVSB
	PUSH	ES
	POP	DS
	MOV	BP,OFFSET MnLoc_B
	CALL	Dialog
	CMP	AL,2
	JB	M64_03
	POP	CX
	JMP	Func04
M64_03:	PUSH	CS
	POP	DS
	MOV	SI,BX
	MOV	DI,OFFSET TmpBuf
	PUSH	DI
	MOV	CX,13
	PUSH	DI
	REP	MOVSB
	POP	DI
	PUSH	ES
	POP	DS
	MOV	SI,BX
	CMP	AL,0
	JNE	M64_04
	CALL	MainFil
M64_04:	POP	SI
	CALL	LetLin
	MOV	DI,OFFSET ViewFil
	MOV	CX,LenStr
	REP	MOVSB
	MOV	AL,0
	STOSB
	POP	AX
	MOV	DL,1
	JMP	V00_01

M66_00:	MOV	BX,OFFSET AutoMnu	; Auto menus
M66_01:	CALL	InvVar
	JMP	M60_02

M67_00:	MOV	BX,OFFSET Prompt	; Path prompt
	JMP	M66_01

M69_00:	CALL	BegBX			; Full screen
	CALL	SizDX
	MOV	BL,0
	MOV	DL,80
	CALL	ResUser
	MOV	BX,OFFSET FulScr
	CALL	InvVar
	JMP	M60_02

M70_00:	MOV	BX,OFFSET AutoSiz	; Auto directory sizes
	CMP	BYTE PTR ES:[BX],0
	JNZ	M66_01
	PUSH	DS
	MOV	DS:ChgFlg,1
	CALL	InvWCB
	MOV	DS:ChgFlg,1
	POP	DS
	JMP	M66_01

M71_00:	MOV	BX,OFFSET MiniSta	; Mini status
	JMP	M66_01

M72_00:	MOV	BX,OFFSET Clock		; Clock
	JMP	M66_01

M73_00:	MOV	BX,OFFSET LoadFl	; Memory allocation
	JMP	M66_01

	.CONST
SvSt0_S	DB	' Setup ',0
SvSt1_S	DB	'Do you wish to save',0
SvSt2_S	DB	'the current setup?',0
SvSt3_S	DB	'Save',0FFh,3,'Cancel',0
StEr1_S	DB	'There was an error while',0
StEr2_S	DB	'saving the setup file',0

	.DATA
SvSt_P	DB	Box_C, BoxCu_C, BoxCu_C, 0
	DB	4, 0
	DW	OFFSET WinBuf
	DW	OFFSET SvSt0_S
	DW	OFFSET SvSt1_S
	DW	OFFSET SvSt2_S
	DW	OFFSET SvSt3_S
SvSt_B	BOXHD	< SvSt_P, 528h, 0, 1, 4, 53, 0 >
	BOXMN	< 4, 0,0,0,0, 420h, TmpBuf, SvSt_D >
	DB	0
SetEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 0
	DW	OFFSET ErrBuf
	DW	OFFSET SvSt0_S
	DW	OFFSET StEr1_S
	DW	OFFSET StEr2_S
	DW	OFFSET Ok_S
SetEr_B	BOXHD	< SetEr_P, 928h, 0, 1, 0FFh, 0, 2 >
	BOXMN	< 4, 0,0,0,0, 426h, TmpErr, OkMn_D>
	DB	0
SvSt_D	DB	6, 0
	DB	8, 7
	DB	0

	.CODE
M75_00:	MOV	BP,OFFSET SvSt_B	; Save setup
	CALL	Dialog
	JNE	M75_01
	CALL	SaveSet
	JNC	M75_01
	CMP	AX,WORD PTR 53h
	JE	M75_01
	MOV	BP,OFFSET SetEr_B
	CALL	Dialog
M75_01:	JMP	Func04
