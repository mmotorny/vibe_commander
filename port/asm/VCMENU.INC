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
