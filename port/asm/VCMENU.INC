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
	DW	M17_00			; Drive...
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

; The commands of the menus are C: port/commands.c. A command is a function
; there. It gets the panel, which the assembly has in DS, and CX as the
; menu bar or the key has left it, from the bar the menu in CL and the
; item in CH, and returns in AX where the program goes on, a number of
; MnuNx_T below. CX and DS are then as they were.
MnuC	MACRO	Name
	MOV	AX,DS
	CallC	Name
	JMP	MnuEnd
	ENDM

; A command of the Left or the Right menu has that panel in DS and finds
; the current one on the stack, where the stub of the bar has pushed it,
; and so do M01_01, M03_00 and M06_00, where the keys of the panels go on
; (CmdNx_T in VCFUNC.INC), and M06_10, where the box of the drives does.
; They took it off before they jumped on: here the stub does.
PanC	MACRO	Name
	MOV	AX,DS
	CallC	Name
	JMP	PanEnd
	ENDM

M01_00:	PanC	M01_00_c		; Brief / Full
M01_01:	PanC	M01_01_c		; CL - the order, CH - brief or full
M03_00:	PanC	M03_00_c		; Info / Tree
M06_00:	PanC	M06_00_c		; Panel on/off
M06_10:	PanC	M06_10_c		; Panel on
M08_00:	PanC	M08_00_c		; Sort
M16_01:	PanC	M16_01_c		; Filter...
; Drive... goes on in the box of the drives, whose stub, F62_01 of
; VCFUNC.INC, takes the panel in AX and the menu to mark in DX.
M17_00:	MOV	AX,DS
	MOV	DL,CL
	MOV	DH,0
	JMP	F62_01

M16_00:	MnuC	M16_00_c		; Ctrl-F - filter
M16_10:	MnuC	M16_10_c		; Ctrl-H - hidden files
M21_00:	MnuC	M21_00_c		; Help
M41_00:	MnuC	M41_00_c		; Alt F10 - Tree
M42_00:	MnuC	M42_00_c		; Alt F6 - Directory sizes
F69_00:	MnuC	F69_00_c		; Alt F9 - EGA lines
M49_00:	MnuC	M49_00_c		; Compare directories
M60_00:	MnuC	M60_00_c		; Configuration
M61_00:	MnuC	M61_00_c		; Advanced options
M53_00:	MnuC	M53_00_c		; Menu file edit
M62_00:	MnuC	M62_00_c		; Extension file edit
M63_00:	MnuC	M63_00_c		; Viewer file
M64_00:	MnuC	M64_00_c		; Editor file
M66_00:	MnuC	M66_00_c		; Auto menus
M67_00:	MnuC	M67_00_c		; Path prompt
M69_00:	MnuC	M69_00_c		; Full screen
M70_00:	MnuC	M70_00_c		; Auto directory sizes
M71_00:	MnuC	M71_00_c		; Mini status
M72_00:	MnuC	M72_00_c		; Clock
M73_00:	MnuC	M73_00_c		; Memory allocation
M75_00:	MnuC	M75_00_c		; Save setup

; Where a command goes on: at the place of this table that it returned.
	.DATA
MnuNx_T	DW	Func04, Init50, MnuF24, MnuV00

	.CODE
PanEnd:	POP	DS
MnuEnd:	MOV	BX,AX
	ADD	BX,BX
	JMP	CS:MnuNx_T[BX]
MnuF24:	PUSH	DS			; The tree in a box: go to the
	PUSH	ES			; directory at TmpBuf, and have
	MOV	DX,OFFSET TmpBuf	; the current panel show it
	JMP	F24_01
MnuV00:	MOV	AX,CX			; Edit ViewFil, with the menu that
	MOV	DL,1			; the command came from marked
	JMP	V00_01

; Entries for the C code of these commands to routines of files that are
; being translated elsewhere (see FromC in VC.ASM, and port/commands.c for
; what they take and return).
	FromC	ScrOffs
	FromC	SaveSet, <SBB DX,DX>	; CY as a number, next to AX

; The BIOS for the screen, called as the original calls it here, with the
; mouse pointer left as it is. AX, BX, CX and DX come from four words at
; SS:BX and are put back there as the BIOS leaves them.
	PUBLIC	Int10_a
Int10_a	PROC	FAR
	PUSH	BP
	MOV	BP,BX
	MOV	AX,[BP]
	MOV	BX,[BP+2]
	MOV	CX,[BP+4]
	MOV	DX,[BP+6]
	PUSH	BP
	INT	10h
	POP	BP
	MOV	[BP],AX
	MOV	[BP+2],BX
	MOV	[BP+4],CX
	MOV	[BP+6],DX
	POP	BP
	RET
Int10_a	ENDP

; The boxes and the dialogs of the commands, and their texts, stay here:
; DialBox and Dialog read them in this segment, and the C code says where
; they are. ExFl_S is copied with the 80 bytes that begin with it.
	PUBLIC	Filt_B, DiSiz_P, CmpDi_B, Conf_B, Chk1_S, Sets_B, Chk2_S
	PUBLIC	MnLoc_B, ExFl_S, UsMnu_S, ExtFl_S, SvSt_B, SetEr_B

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
