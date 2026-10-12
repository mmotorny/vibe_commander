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

; The keys of the panels, from here to F21_00, are C: port/keys.c. A
; command is a function there. It gets the panel, which the assembly has
; in DS, and returns in AL where the program goes on, a number of CmdNx_T
; below. CmdC is the stub of a command: it keeps the key, which F73_00
; takes in AX.
CmdC	MACRO	Name
	PUSH	AX
	MOV	AX,DS
	CallC	Name
	JMP	CmdEnd
	ENDM

; The mouse comes in here, from Func40 on in VC.ASM, with the numbers of
; the cursor's entry in SI, of the first one shown in DI and of the last
; one in DX, and with its buttons in BP.
F00_60:	PUSH	AX			; Scroll & select
	MOV	AX,DS
	MOV	CX,BP
	CallC	F00_60_c
	JMP	CmdEnd
F00_05:	PUSH	AX
	MOV	AX,DS
	MOV	CX,BP
	CallC	F00_05_c
	JMP	CmdEnd
F01_05:	PUSH	AX
	MOV	AX,DS
	MOV	CX,BP
	CallC	F01_05_c
	JMP	CmdEnd

F00_00:	CmdC	F00_00_c		; Up
F01_00:	CmdC	F01_00_c		; Down
F02_00:	CmdC	F02_00_c		; Left
F03_00:	CmdC	F03_00_c		; Right
F04_00:	CmdC	F04_00_c		; PgUp
F05_00:	CmdC	F05_00_c		; PgDn
F06_00:	CmdC	F06_00_c		; Home
F07_00:	CmdC	F07_00_c		; End
F08_00:	CmdC	F08_00_c		; Ins

; The dialog that asks for a mask and the box that says no file is like
; it stay here: Dialog reads them in this segment, and the C code says
; where they are.
	PUBLIC	Sel_S, Unsl_S, Inv_S, Sel_B, ErSel_B
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
F76_00:	CmdC	F76_00_c		; Grey *
F76_01:	CmdC	F76_01_c		; the same, from the menu
F09_00:	CmdC	F09_00_c		; Grey +
F10_00:	CmdC	F10_00_c		; Grey -
F78_00:	CmdC	F78_00_c		; ^M - restore marks
F11_00:	CmdC	F11_00_c		; ^O
F15_00:	CmdC	F15_00_c		; ^U
F16_00:	CmdC	F16_00_c		; ^B
F20_00:	CmdC	F20_00_c		; Tab

; These go on in the routines of a panel's menu, M01_01, M03_00 and
; M06_00 of VCMENU.INC, which take the DS of the command off the stack
; when they are done. The menu itself comes in at F17_01, with DS
; pushed and on the panel of the menu.
F12_00:	PUSH	DS			; ^P
	CmdC	F12_00_c
F13_00:	PUSH	DS			; ^F1
	CmdC	F13_00_c
F14_00:	PUSH	DS			; ^F2
	CmdC	F14_00_c
F17_00:	PUSH	DS			; ^R
F17_01:	CmdC	F17_01_c
F79_00:	PUSH	DS			; ^Z - tree
	CmdC	F79_00_c
F18_00:	PUSH	DS			; ^L - info
	CmdC	F18_00_c

; Where a command goes on: at the place of this table that it returned
; in AL (see port/keys.c, and NEXT_ in port/vc.h for those of the last
; line). For the routines of the menu DX is the panel, and
; AH a byte of theirs: five less the kind of panel for M03_00, and for
; M06_00 the word of the menu bar that is lit meanwhile. For V00_00 DL is
; 1 for the editor. The key is on the stack.
	.DATA
CmdNx_T	DW	Func07, Func05, Func04, CmdF01, F73_00, F25_00, F26_00
	DW	Init40, Init50, CmdM01, CmdM03, CmdM06, CmdPp4, CmdPp5
	DW	CmdIni, CmdExe, V00_00, CmdV02, Exit

	.CODE
CmdEnd:	MOV	BL,AL
	MOV	BH,0
	ADD	BX,BX
	MOV	CH,AH
	POP	AX
	JMP	CS:CmdNx_T[BX]
CmdF01:	MOV	DS,ES:WCBcrn		; Tab has made another the current one
	JMP	Func01
CmdM01:	MOV	DS,DX
	MOV	CL,DS:SortTyp
	MOV	CH,DS:BrfFul
	JMP	M01_01
CmdM03:	MOV	DS,DX
	JMP	M03_00
CmdM06:	MOV	DS,DX
	MOV	CL,CH
	JMP	M06_00
CmdPp4:	POP	DS
	JMP	Func04
CmdPp5:	POP	DS
	JMP	Func05
; A command is run by the part of the program that stays in the memory,
; where F21_30_c has saved what it needs: Exec there, which gets in ES the
; block of the memory that the rest of the program is in, to give it back
; to DOS. With nothing to run the program starts over at once.
CmdExe:	PUSH	SS
	POP	DS
	MOV	AX,CS
	ADD	AX,WORD PTR ((Init00-Zero) SHR 4)
	MOV	ES,AX
	MOV	AX,OFFSET Exec
	PUSH	DS
	PUSH	AX
	RETF
CmdIni:	PUSH	SS
	PUSH	SS
	POP	DS
	POP	ES
	JMP	Init11
; The editor on the dirinfo of an Info panel, in the window that F34_10_c
; has set.
CmdV02:	MOV	DL,1
	MOV	AL,2
	JMP	V00_02

; Make the other panel the current one
;			   DS - WCBseg of the current one
;	Return :	   DS - WCBseg of the one that is current now
F20_02	PROC	NEAR
	MOV	AX,DS
	CallC	F20_02_c
	MOV	DS,ES:WCBcrn
	RET
F20_02	ENDP

; Enter, the keys that change the directory and the commands before and
; after are C: port/enter.c. F21_10 is where the mouse comes in with a
; second click, F21_25 where the history does with a command to run,
; F21_30 the user menu, with the commands of a line in DosBuf, and F21_31
; the program when it has started: the main loop goes there and to
; F21_10, port/main.c.
F21_00:	CmdC	F21_00_c		; Enter
F21_10:	CmdC	F21_10_c
F21_25:	CmdC	F21_25_c
F21_30:	CmdC	F21_30_c
F21_31:	CmdC	F21_31_c
F22_00:	CmdC	F22_00_c		; ^PgUp
F23_00:	CmdC	F23_00_c		; ^PgDn
F24_00:	CmdC	F24_00_c		; ^\
; The tree in its box, M41_00 of VCMENU.INC, comes in here with the path
; of a directory in the main data at DX, and with DS and ES pushed.
F24_01:	POP	AX
	POP	DS
	PUSH	AX
	MOV	AX,DS
	CallC	F24_01_c
	JMP	CmdEnd
; Where the file search comes in when it has gone to a file.
F24_02:	CmdC	F24_02_c
F25_00:	CmdC	F25_00_c		; ^E
F26_00:	CmdC	F26_00_c		; ^X
F27_00:	CmdC	F27_00_c		; ^Enter

; What the part of the program that stays in the memory keeps while a
; command runs, for F21_30_c to fill.
	PUBLIC	ComnIni, WCB1sav, WCB2sav

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
; F10 is C: F30_00_c of port/enter.c, which has Dialog show this box.
	PUBLIC	Quit_B, Name_S
F30_00:	CmdC	F30_00_c		; F10 - quit

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

F75_00:	CmdC	F75_00_c		; Auto CD / Quick view

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

	.CONST
List_F	DB	'vc.lst',0

	.CODE
; The commands of a line of a menu or of an extension file are made by C,
; Run_00_c of port/usermenu.c, which only C code calls. Here the user menu
; goes on: with 1 in AX there is a command to run.
Run_0A:	OR	AX,AX
	JNE	Run_0B
	JMP	Func04
Run_0B:	JMP	F21_30

