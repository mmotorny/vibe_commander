;--------------------- Subroutines ---------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;----------------- Clear keyboard buffer ---------------
; C: ClrKbd_c of port/system.c, which only C code calls.

;----------------------- Go to CASE --------------------
; Case, which jumped by a table of keys, is gone: every such table is a
; switch in C now, the last of them that of the main loop's keys, Func12
; of port/main.c.

;------------------ Check video mode -------------------
;			   ZR - ok
; C: ChkMode_c of port/window.c. It returns 1 for ok, and puts AX and BH
; as the BIOS leaves them into the two words on the stack, struct
; VideoMode of port/vc.h: the code that starts the program goes on with
; the mode in AL, the columns in AH and the page in BH.
ChkMode	PROC	NEAR
	PUSH	BX
	PUSH	AX
	MOV	BX,SP
	CALL	FAR PTR ChkMode_c
	CMP	AX,1			; ZR for 1
	POP	AX
	POP	BX
	RET
ChkMode	ENDP

;----------------------- Set cursor --------------------
;			AL,AH - x, y
; C: Cursor_c of port/window.c.
Cursor	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR Cursor_c
	POP	AX
	RET
Cursor	ENDP

;-------------- Get cursor size & set it ---------------
;	Return :	CX - cursor size
; C: ChgCur_c of port/window.c, which only C code calls.

;----------------------- Beep sound --------------------
; C: Beep_c of port/system.c, which only C code calls.

;--------------- Set type of mouse sursor --------------
;			   AL - cursor
; C: TypMous_c of port/mouse.c, which only C code calls.

;----------------- Mouse double click ------------------
;			BH,BL - base of selected point
;			DH,DL - size of selected point
;			   SI - cursor
;	Return :	   CY - continue
;			   NZ - action
; C: DoublMo_c of port/mouse.c, which only C code calls.

;------------------ INT 10h with mouse -----------------
; C: Int10m_c of port/window.c, which only C code calls.

;-------------------- Screen Offset --------------------
; C: ScrOffs_c of port/window.c, which notes the offset in the main data,
; where the callers have DS, and returns it in DI. AX and BH stay, where
; the routine left what the BIOS says of the mode there.
ScrOffs	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR ScrOffs_c
	POP	AX
	RET
ScrOffs	ENDP

;--------------- Shift windows in EGA lines ------------
; C: SftEGA_c of port/window.c, which only C code calls.

;---------------------- Save window --------------------
;			BH,BL - begin
;			DH,DL - size
;			   SI - address of buffer
; C: SavWin_c of port/window.c, which only C code calls.

;----------------- Restore user screen -----------------
;			BH,BL - begin
;			DH,DL - size
; C: ResUser_c of port/window.c, which only C code calls.

;------------------------ Save screen ------------------
; C: SavScr_c of port/window.c, with MovScr, which moves the cells. Only C
; code calls it.

;------------------------ Boxes ------------------------
; The frame of a box, its shadow and its growing are C: MinBox, ShadBox,
; BoxDubl, BoxLine, StdLine and OncZoom of port/box.c, with the routines
; that were their own. Only C code calls them.

;------------------------ Dialog -----------------------
;			CS:DX - address of include line
;			CS:BP - pointer:
;				DW  BoxPoint, BegCenter, SecondBox
;				DB  KeyBar, Menu, HlpPage, Status
;					DB  Type
;					DB  Up, Down, Left, Right
;					DW  Begin, Point, Control
;					...
;				DB  0
;				Status:	b0 - Beep
;					b1 - ClrKbd
;					b2 - SftEGA
;					b3 - F10-Tree
;					b4 - disable DialBox
; A dialog is C: Dialog_c of port/dialog.c, with the routines of its
; fields, InpStr, Select and FlipFl. The dialog goes there as its offset,
; and in CX the caller's DS, where the fields have what they set. The
; answer comes back in AX, and callers go by the zero flag for AL.
Dialog	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	MOV	CX,DS
	MOV	AX,BP
	CallC	Dialog_c
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	CMP	AL,0
	RET
Dialog	ENDP

; The code segment of the assembly, where the boxes and the dialogs are
; that C code reads.
	PUBLIC	AsmSeg_a
AsmSeg_a PROC	FAR
	MOV	AX,CS
	RET
AsmSeg_a ENDP

; The routine that a dialog has for Enter and F10: the near routine at
; SI, with AX, BX and DI as Dialog has them, in BP the dialog, which
; comes in DX, and in DS what comes in CX, the DS of whoever asked for
; the dialog. AX comes back as the routine leaves it, and DX is not 0 if
; it set the carry flag.
	PUBLIC	DialSub_a
DialSub_a PROC	FAR
	PUSH	BP
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	BP,DX
	MOV	DS,CX
	CALL	SI
	SBB	DX,DX
	POP	DS
	POP	BP
	RET
DialSub_a ENDP

;			Add file name
AddFile	PROC	NEAR
	CALL	FAR PTR AddFile_c
	MOV	AL,'\'
	RET
AddFile	ENDP
;			DS:SI(B) -> ES:DI(B)
MvzLin0	PROC	NEAR
	MOV	CX,LenPath
MvzLin0	ENDP

MvzLine	PROC	NEAR
	PUSH	DX
	PUSH	DI
	MOV	DX,DS
	CALL	FAR PTR MvzLine_c
	POP	DX			; Where the copy begins
	ADD	SI,CX
	ADD	CX,DX			; What the search for the end
	SUB	CX,DI			; left of the count
	DEC	CX
	POP	DX
	MOV	AL,0
	RET
MvzLine	ENDP

;------------------------ Cut path ---------------------
;			DS:SI - path
CutPath	PROC	NEAR
	PUSH	ES
	PUSH	DS
	POP	ES
	CALL	FAR PTR CutPath_c
	POP	ES
	RET
CutPath	ENDP

;---------------------- Higher letter ------------------
;			ES:SI - address of string
PathLin	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR PathLin_c
	POP	AX
	POP	DS
	RET
PathLin	ENDP

;------------------- Test drive letter -----------------
;			   AL - drive letter
;	Return :	   ZR - yes
;			   NZ - no
; C: TstDrv_c of port/system.c, which only C code calls.

;----------------- CALL to PSP segment -----------------
;			   AX - offset in PSP
; A far call, by way of the stack, of a routine in the part of the program
; that stays in the memory. It is here for as long as assembly makes such
; calls: the code that starts the program, for OutStr and SetBrk, and the
; entry by which C code has a program released, Release_a of VCMEM.INC.
; C code that calls such a routine does so through a far pointer.
CallPSP	PROC	NEAR
	POP	BP
	PUSH	CS
	PUSH	BP
	PUSH	SS
	PUSH	AX
	RETF
CallPSP	ENDP

;------------------ Page Up, Page Down -----------------
; C: PgUp_c and PgDn_c of port/misc.c, which only C code calls.
