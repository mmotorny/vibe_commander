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
; What the box shows is in port/find.c, but for these two, each of which
; the command copies with the 15 bytes that begin with it.
	PUBLIC	FnFl6_S, FnFl7_S
FnFl6_S	DB	' files',0
FnFl7_S	DB	' found.',0

	.DATA
; The dialog that asks what to look for and the menu of a search that is
; over, with its colours: C code hands them to Dialog and MinMnu.
	PUBLIC	Find_B, Find6_D, FnFl_P
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
; The command is C: F67_00_c of port/find.c. It is jumped to, and jumps on
; by what its function returns in AX (TO_ in port/vc.h); for F24_02, where
; a panel is read whose directory is another now, DX is that panel.
F67_00:	CallC	F67_00_c		; Alt F7 - file find
	CMP	AX,4
	JE	F67_01
	JMP	F1Next
F67_01:	MOV	DS,DX
	JMP	F24_02
