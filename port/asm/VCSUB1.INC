;--------------------- Subroutines ---------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;----------------- Clear keyboard buffer ---------------
; C: ClrKbd_c of port/system.c, which only C code calls.

;----------------------- Go to CASE --------------------
; Case, which jumped by a table of keys, is gone: every such table is a
; switch in C now, the last of them that of the main loop's keys, Func12
; of port/main.c.

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
; fields, InpStr, Select and FlipFl, which only C code calls.

; The code segment of the assembly, where the boxes and the dialogs are
; that C code reads.
	PUBLIC	AsmSeg_a
AsmSeg_a PROC	FAR
	MOV	AX,CS
	RET
AsmSeg_a ENDP



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