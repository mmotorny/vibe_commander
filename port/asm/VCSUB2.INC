;--------------------- Subroutines ---------------------
;		Version :	16.10.1999
;-------------------------------------------------------

;-------------------- Output menu bar ------------------
;			   AL - Case
; C: PtMnu1_c of port/bars.c. The names of the menus stay here: the menu
; reads them too, to find the one that the mouse is on (F40_50).
	.CONST
PtMnu_S	DB	0FFh,4,'Left',0FFh,4,'Files',0FFh,4,'Commands'
	DB	0FFh,4,'Options',0FFh,4,'Right',0FFh,31,0

	.CODE

;--------------- Output path in the middle -------------
; C: PutPath_c of port/panel.c, which only C code calls. The title of a
; tree stays here: boxes of the assembly name it.
	.CONST
Tree_S	DB	' Tree ',0

	.CODE
;------------ Find iten in extension file ------------
; C: FndItem_c of port/extfile.c, which only C code calls.

; Input0, which waits for the key, is C too, and only C code calls it.

; LeaveTm, which tells DOS that the program is idle, and Blank, the
; screen blanker, are C: port/blank.c. Only C code calls them, and Rnd of
; port/input.c, which Blank called here.

; EdSub1, which finds where a word begins, is C too: only C code calls it.

