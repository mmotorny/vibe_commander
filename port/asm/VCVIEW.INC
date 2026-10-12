;-------------------------------------------------------
;		Version :	14.06.2000
;-------------------------------------------------------

; The texts, boxes and dialogs of the viewer are in port/view_t.c.

HexLen	EQU	WORD PTR HelpBuf[190]	; Count of hex pattern

ViewDat	STRUC
	 DB	TmpBuf+200 DUP(?)
ViSpace	 DW	?			; Edit buffer size in paragraphs
ViewEdit DB	?			; View / Edit
ViStatus DB	?			; Output status line
ViLoaded DW	?			; Bytes loaded in buffer
ViLen	 DD	?			; Size of file
ViOffset DD	?			; Offset of first loaded byte in buffer
ViKeyBar DW	?			; Saved KeyBar & Clock
ViTxHex	 DB	?			; Text / Hex mode
ViWrap	 DB	?			; Wrap / Unwrap
ViFirst	 DW	?			; Offset of first line of screen
ViHandle DW	?			; File handle
ViCol	 DW	?			; First column displayed on the screen
ViPoint	 DW	?			; Point to address of first line
ViEndTab DW	?			; Point to end of line table
ViBase	 DW	?			; View window base
ViSize	 DW	?			; View window size
ViSrPan	 DB	?			; String was found in Hex/ASCII panel
ViSrLen	 DB	?			; Length of the found string
ViTmp1	 DW	?			; First byte of search string
ViTmp2	 DW	?			; Last byte of search string
ViSearch DD	?			; Saerch offset
Vi100pc	 DB	?			; 100% was displayed
ViewDat	ENDS

	.CODE
; The commands that view or edit a file are C: port/view.c, with F34_30
; and F34_40, which read a line of an extension file. F34_10 is where the
; mouse comes in with a click on the dirinfo of an Info panel.
F43_00:	CmdC	F43_00_c		; View...
F44_00:	CmdC	F44_00_c		; Edit...
F63_00:	CmdC	F63_00_c		; Internal viewer
F64_00:	CmdC	F64_00_c		; Internal editor
F33_00:	CmdC	F33_00_c		; View
F34_00:	CmdC	F34_00_c		; Edit
F34_10:	CmdC	F34_10_c		; Edit 'dirinfo'

; Opening a file for the viewer or the editor, the viewer and the way back
; to the panels are C: V00_02_c, V00_10_c, V13_00_c and V13_01_c of
; port/view.c. They are jumped to, with ES on the main data.
;	View or edit the file ViewFil
;			   AL - the item of the menu bar to mark
;			   DL - 0 to view, 1 to edit
V00_00:	MOV	AL,2
V00_01:	MOV	BX,1			; In all of the screen
	JMP	V00_03
V00_02:	XOR	BX,BX			; In the window that ViBase and ViSize say
V00_03:	PUSH	ES
	POP	DS
	SUB	SP,8			; The file, and where the box is
	MOV	DI,SP
	CALL	FAR PTR V00_02_c
	MOV	CX,AX			; 0 - nothing, 1 - the editor, 2 - the viewer
	POP	BX
	POP	AX
	POP	DX
	POP	SI
	JCXZ	V00_04
	DEC	CX
	JNZ	V00_10
	JMP	T00_00
V00_04:	JMP	Func04
;	The viewer, also for a file that is too large for the editor
;			   BX - handle
V00_10:	CallC	V00_10_c
	JMP	Init50
;	Close the file and go back to the panels
V13_00:	CallC	V13_00_c
	JMP	Init50
;	Go back to the panels
;			   CL - not 0 if a file was written
V13_01:	MOV	AX,CX
	CallC	V13_01_c
	JMP	Init50
