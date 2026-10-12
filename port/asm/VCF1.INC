;-------------------------------------------------------
;		Version :	16.10.1999
;-------------------------------------------------------

; The texts, the boxes and the dialogs of this file are C, in
; port/dirfunc_t.c. Two dialogs there name a routine of this file.
	PUBLIC	F38_10, M30_60

	.CODE
; The commands of this file are C, in port/dirfunc.c. Each is jumped to,
; from the table of the keys or from a menu, with the panel in DS, and
; jumps on by what its function returns.
F37_00:	MOV	AX,DS			; F7 - make directory
	CallC	F37_00_c
	JMP	F1Next

F47_00:	CallC	F47_00_c		; Shift F7 - make directory
	JMP	F1Next

; Where a command that is C goes on: AX says where (TO_ in port/vc.h).
F1Next:	DEC	AX
	JNS	F1Nx1
	JMP	Func04
F1Nx1:	JNZ	F1Nx2
	JMP	FncSel
F1Nx2:	DEC	AX
	JNZ	F1Nx3
	JMP	FncExt
F1Nx3:	JMP	FncEx1

F38_00:	MOV	AX,DS			; F8 - delete
	CallC	F38_00_c
	JMP	F1Next

; Dialog calls this when the box for selected files is answered.
;			   AL - the answer
;	Return :	   AL - the answer to go by, CY=0
F38_10	PROC	NEAR
	CallC	F38_10_c
	CLC
	RET
F38_10	ENDP

F48_00:	CallC	F48_00_c		; Shift F8 - delete
	JMP	F1Next

F58_00:	MOV	AX,DS			; ^F8 - delete one file
	CallC	F58_00_c
	JMP	F1Next

M30_00:	MOV	AX,DS			; Attributes...
	CallC	M30_00_c
	JMP	F1Next

; Dialog calls this when the dialog of attributes is answered.
;			   AL - the answer
;	Return :	   AL - the answer to go by
;			 CY=1 - a date or a time was refused
M30_60	PROC	NEAR			; Test date & time
	PUSH	DX
	CallC	M30_60_c		; DX: not 0 if one was refused
	NEG	DX
	POP	DX
	RET
M30_60	ENDP

F54_00:	CallC	F54_00_c		; Change label
	JMP	F1Next

	.DATA
	ASSUME	CS:DGROUP

;	Return :	   AX - 1 all done, 0 not, 100h to stop
;			   DS - main data
DirFunc	PROC	NEAR			; Find files in directory
	PUSH	ES
	POP	DS
	CALL	FAR PTR DirFunc_c
	RET
DirFunc	ENDP

; Going through the selected files of a panel is C, and so is the end of
; a command that may have changed what is on the disk: FncSel_c, FncExt_c
; and FncEx1_c of port/dirfunc.c, which answer 1 if the panels are to be
; read again. They are jumped to.
FncSel:	CallC	FncSel_c		; Selected files and directories
	JMP	SHORT FncEx0
FncExt:	CallC	FncExt_c		; Exit
	JMP	SHORT FncEx0
FncEx1:	CallC	FncEx1_c
FncEx0:	OR	AX,AX
	JNE	FncEx2
	JMP	Func04
FncEx2:	JMP	Init50

	.CODE
; The files of a mask are gone through in C: FncMas_c of port/dirfunc.c.
;			   BX - a title for "Can't find file", or 0
FncMas:	MOV	AX,BX			; Masked files
	CallC	FncMas_c
	JMP	FncExt

;			ES:SI - a path on the drive
;	Return :	 CY=1 - error
;			   AX - what DOS answered, with CY or without:
;				^R in a tree goes on with it
DelTree	PROC	NEAR
	CMP	ES:ChgTree,0
	JZ	DelTr5
DelTr0:	PUSH	DX			; Whether the tree is read again
	MOV	AX,SI			; by itself or not
	CallC	DelTr0_c		; DX: not 0 for an error
	NEG	DX
	POP	DX
DelTr5:	RET
DelTree	ENDP
