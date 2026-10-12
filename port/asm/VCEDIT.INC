;-------------------------------------------------------
;		Version :	16.10.1999
;-------------------------------------------------------

; The texts, boxes and dialogs of the editor are in port/edit_t.c.

EditBuf	EQU	MenuBuf			; Edit line buffer
UndoBuf	EQU	ErrBuf			; Undo buffer
LenEdit	EQU	LenMenu			; Length of edit buffer
	; The length of undo buffer must be greate then edit buffer

EdData	STRUC
	DB	TmpBuf+100 DUP(?)
EdHelp	DB	0 DUP(?)		; Mini help
EdOffst	DD	?			; Offset of begin of the current line
EdEndBf	DD	?			; Offset of end of part of current line
EdSizLn	DW	?			; Length of part of line in buffer
EdUpdat	DB	?			; Update line buffer
EdWhole	DB	?			; The whole line allocates buffer
EdCursr	DW	?			; Offset of the cursor in line buffer
EdCol1	DW	?			; Cursor position for Up & Down
EdUpDwn	DB	?			; Up & Down flag
EdHex1	DB	?			; Cursor digit pointer for Up & Down
EdScrn	DB	?			; Output all edit window
EdCurLn	DB	?			; It doesn't need to output line
EdUndo	DW	?			; Length of line in undo buffer
	DB	77 DUP(?)
EdWrite	DB	?			; File was written
EdSpace	DW	?			; Edit buffer size in paragraphs
	DB	?
EdStat	DB	?			; Status line
EdHexAs	DB	?			; Hex / ASCII field in HEX mode
EdHxCur	DB	?			; Cursor points 1 or 2 digit in HEX
EdLen	DD	?			; Size of file in edit buffer
EdCol	DW	?			; Cursor position
EdLine	DW	?			; Cursor line
	DW	?
EdTxHex	DB	?			; Text / Hex mode
EdChng	DB	?			; File was changed
	DD	?
EdCol0	DW	?			; First column displayed on the screen
EdLin0	DW	?			; First line displayed on the screen
	DW	?
EdBase	DW	?			; The base of the edit window
EdSize	DW	?			; The size of the edit window
EdFirst	DD	?			; Offset of first line on the screen
EdShape	DW	?			; Original cursor shape
EdData	ENDS

	.CODE
; The editor is C: T00_00_c of port/edit.c, with all that a key does. The
; viewer's code jumps here when it has opened the file to edit, with the
; handle in BX, and in AX, DX and SI where its box is. The answer says
; where that code goes on.
T00_00:	PUSH	BX
	CALL	FAR PTR T00_00_c
	POP	BX
	PUSH	DS
	POP	ES
	CMP	AX,1
	JE	T00_01
	JA	T00_02
	JMP	V00_10			; View the file instead
T00_01:	JMP	V13_00			; Not read
T00_02:	MOV	CL,DS:EdWrite		; Edited
	JMP	V13_01
