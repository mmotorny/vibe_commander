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