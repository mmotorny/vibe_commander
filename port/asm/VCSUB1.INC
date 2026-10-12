;--------------------- Subroutines ---------------------
;		Version :	09.09.1999
;-------------------------------------------------------

;----------------------- Timer -------------------------
; C: Timer_c of port/system.c, which returns the count in DX and AX.
Timer	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR Timer_c
	MOV	CX,DX
	MOV	DX,AX
	POP	AX
	RET
Timer	ENDP

;----------------- Clear keyboard buffer ---------------
; C: ClrKbd_c of port/system.c, which only C code calls.

;---------------------- Break test ---------------------
;	Return :	CY=1 - break
; C: TstBrk_c of port/system.c, which returns 1 for a break.
TstBrk	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR TstBrk_c
	NEG	AX			; CY for 1
	POP	AX
	RET
TstBrk	ENDP

;----------------------- Go to CASE --------------------
;			CS:SI - address of case table
;			   AX - key code
Case	PROC	NEAR
	PUSH	SI
	PUSH	CX
	OR	AH,AH
	JE	Case3
Case1:	MOV	CX,CS:[SI]
	ADD	SI,4
	JCXZ	Case3
	CMP	AL,CL
	JNE	Case1
	OR	CH,CH
	JE	Case2
	CMP	AH,CH
	JNE	Case1
Case2:	POP	CX
	ADD	SP,4
	JMP	WORD PTR CS:[SI-2]
Case3:	POP	CX
	POP	SI
	RET
Case	ENDP

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

;------------------ Set cursor on prompt ---------------
; C: Cursor1_c of port/window.c.
Cursor1	PROC	NEAR
	PUSH	AX
	CallC	Cursor1_c
	POP	AX
	RET
Cursor1	ENDP

;-------------- Get cursor size & set it ---------------
;	Return :	CX - cursor size
; C: ChgCur_c of port/window.c, which only C code calls.

;------------------- Make hidden cursor ----------------
; C: HidCur_c of port/window.c.
HidCur	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR HidCur_c
	POP	AX
	RET
HidCur	ENDP

;----------------------- Beep sound --------------------
; C: Beep_c of port/system.c, which only C code calls.

;---------------------- Init mouse ---------------------
; C: IniMous_c of port/mouse.c. It leaves all registers but AX, where the
; routine left AX, BX, CX and DX as the mouse driver did.
IniMous	PROC	NEAR
	CallC	IniMous_c
	RET
IniMous	ENDP

;--------------- Set type of mouse sursor --------------
;			   AL - cursor
; C: TypMous_c of port/mouse.c, which only C code calls.

;------------------ Save mouse cursor ------------------
; C: SavMous_c of port/mouse.c.
SavMous	PROC	NEAR
	PUSH	AX
	CallC	SavMous_c
	POP	AX
	RET
SavMous	ENDP

;------------------- Set mouse cursor ------------------
;			   AL - set/reset
; C: CurMous_c of port/mouse.c.
CurMous	PROC	NEAR
	PUSH	AX
	CallC	CurMous_c
	POP	AX
	RET
CurMous	ENDP

;------------------- Get mouse cursor ------------------
;	Return :	CH,CL - mouse cursor
;			   AX - mouse buttons
; C: GetMous_c of port/mouse.c, which takes CX and returns the buttons in
; AX and the place in DX.
GetMous	PROC	NEAR
	PUSH	DX
	CallC	GetMous_c
	MOV	CX,DX
	POP	DX
	OR	AX,AX			; ZR for no button, and no carry
	RET
GetMous	ENDP

;----------------- Mouse double click ------------------
;			BH,BL - base of selected point
;			DH,DL - size of selected point
;			   SI - cursor
;	Return :	   CY - continue
;			   NZ - action
; C: DoublMo_c of port/mouse.c, which only C code calls.

;--------------- Check cursor in window ----------------
;			CH,CL - cursor
;			BH,BL - begin of window
;			DH,DL - size of window
;	Return :	CY=0  - cursor inside the window
; C: InWind_c of port/mouse.c, which returns 1 for a place outside.
InWind	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR InWind_c
	NEG	AX			; CY for 1
	POP	AX
	RET
InWind	ENDP

;------------------ INT 10h with mouse -----------------
; C: Int10m_c of port/window.c, which only C code calls.

;------------- Convert coordinars to address -----------
;			BH,BL - coordinats
;	Return :	   DI - address
BasAdr	PROC	NEAR
	CALL	FAR PTR BasAdr_c
	RET
BasAdr	ENDP

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

;--------------------- Restore window ------------------
;			BH,BL - begin
;			DH,DL - size
;			   SI - address of buffer
; C: ResWin_c of port/window.c.
ResWin	PROC	NEAR
	PUSH	AX
	CallC	ResWin_c
	POP	AX
	RET
ResWin	ENDP

;----------------- Restore user screen -----------------
;			BH,BL - begin
;			DH,DL - size
; C: ResUser_c of port/window.c, which only C code calls.

;--------------------- Output window -------------------
;			BH,BL - begin
;			DH,DL - size
; C: Window_c of port/window.c.
Window	PROC	NEAR
	PUSH	AX
	CallC	Window_c
	POP	AX
	RET
Window	ENDP

;------------------------ Save screen ------------------
; C: SavScr_c of port/window.c, with MovScr, which moves the cells. Only C
; code calls it.

;------------------------ Boxes ------------------------
; The frame of a box, its shadow and its growing are C: MinBox, ShadBox,
; BoxDubl, BoxLine, StdLine and OncZoom of port/box.c, with the routines
; that were their own. Only C code calls them.

;---------------------- Put window ---------------------
;			BH,BL - begin-center
;			CS:DX - address of include line
;			CS:BP - pointer:
;				DB  NormCol,RevCol,BoldCol,UndrCol
;				DB  NumLns,NumVars
;				DW  SaveBuf
;				DW  Variable
;				DB  VarMode,VarLimit
;				. . .
;				DW  OFFSET Line1
;				. . .
;	Return:		BH,BL - begin
;			DH,DL - size
;			   SI - address of save buffer
; A box is C: DialBox_c of port/dialog.c, with DiBx30. The box goes there
; as its offset, and in CX the caller's DS, where the texts are that it
; puts into its lines. Where the box is comes back in three words on the
; stack: BX, DX and SI.
DialBox	PROC	NEAR
	PUSH	DI
	PUSH	CX
	PUSH	AX
	SUB	SP,6
	MOV	DI,SP
	MOV	CX,DS
	MOV	AX,BP
	CallC	DialBox_c
	POP	BX
	POP	DX
	POP	SI
	POP	AX
	POP	CX
	POP	DI
	RET
DialBox	ENDP

;--------------- Calculate string length ---------------
;			DS:SI - address of line
;			CS:DX - address of include line
;	Return :	   AX - length
LnStr	PROC	NEAR
	PUSH	CX
	PUSH	BX
	PUSH	ES
	CallLine LnStr_c, AX
	TakeLine BX, BX
	POP	ES
	POP	BX
	POP	CX
	RET
LnStr	ENDP

;-------------------- Output NG line -------------------
;			DS:SI - address of line
;			ES:DI - address of screen buffer
;			CS:DX - address of include line
;			CS:BP - address of color table
;			   AL - limit
NgLine	PROC	NEAR
	PUSH	CX
	PUSH	BX
	PUSH	ES
	CallLine NgLine_c, AX
	TakeLine AX, BX
	POP	ES
	POP	BX
	POP	CX
	RET
NgLine	ENDP

;--------------------- Center line ---------------------
;			BH,BL - center
;			DS:SI - address of line
;			CS:DX - address of include line
;			CS:BP - address of color table
CntLine	PROC	NEAR
	PUSH	CX
	PUSH	AX
	PUSH	ES
	CallLine CntLine_c, BX
	TakeLine BX, AX
	POP	ES
	POP	AX
	POP	CX
	RET
CntLine	ENDP

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

;--------------------- Input string --------------------
;			BH,BL - begin
;			ES:SI - string
;			   CX - length
;			   AH - number of color
; Showing a line that is typed into is C: InSt10_c of port/dialog.c. It
; leaves AX changed, as the routine did.
InSt10	PROC	NEAR
	CallC	InSt10_c
	RET
InSt10	ENDP

;------------------------ Menu -------------------------
;			CS:SI - control line
;			BH,BL - begin
;			DH,DL - size
;			   BP - color table
;			   AL - key bar
;			   CL - case
;	Return :	   CL - case
;			   AX - key code
; A line of buttons is C: MinMnu_c of port/dialog.c, with MnMn10 to
; MnMn60. The colours go there as their offset, in DI. CX comes back in
; DX: the case, and above it the key bar.
MinMnu	PROC	NEAR
	PUSH	DI
	PUSH	DX
	MOV	DI,BP
	CallC	MinMnu_c
	MOV	CX,DX
	POP	DX
	POP	DI
	RET
MinMnu	ENDP

;----------------------- Get color ---------------------
;			   AH - color number
;	Return :	   AH - color
Color	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BL,AL
	MOV	AL,AH
	CALL	FAR PTR Color_c
	MOV	AH,AL
	MOV	AL,BL
	POP	BX
	POP	DS
	RET
Color	ENDP

ColAddr	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	MOV	AL,AH
	CALL	FAR PTR ColAddr_c
	POP	AX
	POP	DS
	RET
ColAddr	ENDP

;------------------------- Shadow ----------------------
;			ES:DI - address of screen buffer
Shadow	PROC	NEAR
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Shadow_c
	POP	DS
	RET
Shadow	ENDP

;-------------------- Moving strings -------------------
;			CS:SI(B) -> ES:DI(W)
MvsBwc	PROC	NEAR
	PUSH	DX
	MOV	DX,CS
	CALL	FAR PTR MvsBw_c
	POP	DX
	ADD	SI,CX
	XOR	CX,CX
	MOV	AL,CS:[SI-1]
	RET
MvsBwc	ENDP
;			ES:SI(B) -> ES:DI(W)
MvsBwe	PROC	NEAR
	PUSH	DX
	MOV	DX,ES
	CALL	FAR PTR MvsBw_c
	POP	DX
	ADD	SI,CX
	XOR	CX,CX
	MOV	AL,ES:[SI-1]
	RET
MvsBwe	ENDP
;			(Color) -> ES:DI(W)
MvsColr	PROC	NEAR
	CALL	FAR PTR MvsColr_c
	XOR	CX,CX
	RET
MvsColr	ENDP
;			CS:SI(B) -> ES:DI(W)
MovDz	PROC	NEAR
	CALL	BasAdr
MovDz0:	PUSH	DX
	PUSH	DI
	MOV	DX,DS
	CALL	FAR PTR MovDz0_c
	POP	DX			; Where the text begins on the screen
	SUB	DX,DI
	NEG	DX
	SHR	DX,1			; Its length
	ADD	SI,DX
	INC	SI
	POP	DX
	MOV	AL,0
	RET
MovDz	ENDP
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
;			Clear rest of line DS:SI, legth CX
FilZero	PROC	NEAR
	PUSH	ES
	PUSH	AX
	PUSH	DS
	POP	ES
	CALL	FAR PTR FilZero_c
	POP	AX
	POP	ES
	MOV	AL,0
	XOR	CX,CX
	RET
FilZero	ENDP

;--------------------- Check CrLf ----------------------
;			   AX - word
;	Return :	   ZR - yes
; C: IsCrLf_c of port/misc.c, which returns 1 for yes. IsCrLfS takes the
; word from DS:SI first.
IsCrLfS	PROC	NEAR
	LODSW
IsCrLfS	ENDP

IsCrLf	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR IsCrLf_c
	CMP	AX,1			; ZR for 1
	POP	AX
	RET
IsCrLf	ENDP

;-------------------- String length --------------------
;			ES:DI - string
;	Return :	   CX - length
StrLen	PROC	NEAR
	CALL	FAR PTR StrLen_c
	MOV	AL,0
	RET
StrLen	ENDP

;--------------------- Upper letter --------------------
;			   AL - letter
;	Return :	   AL - letter
; C: UpCase_c of port/system.c, which leaves AH as it is.
UpCase	PROC	NEAR
	CallC	UpCase_c
	RET
UpCase	ENDP

;--------------------- Lower letter --------------------
;			   AL - letter
;	Return :	   AL - letter
LowCase	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BH,AH
	CALL	FAR PTR LowCase_c
	MOV	AH,BH
	POP	BX
	POP	DS
	RET
LowCase	ENDP

;----------------------- Find text ---------------------
;			DS:SI - string
FndLet	PROC	NEAR
	PUSH	ES
	PUSH	DS
	POP	ES
	CALL	FAR PTR FndLet_c
	POP	ES
	MOV	AL,DS:[SI]
	RET
FndLet	ENDP

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

;--------------------- Normal string -------------------
;			ES:SI - address of string
NormStr	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR NormStr_c
	POP	AX
	RET
NormStr	ENDP

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

;----------------- Higher & lower letters --------------
;			ES:SI - address of string
LetLin	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSH	ES
	POP	DS
	CALL	FAR PTR LetLin_c
	POP	AX
	POP	DS
	RET
LetLin	ENDP

;-------------------- Shirter string -------------------
;			ES:SI - address of string
;			   AX - limit
ShrtLin	PROC	NEAR
	PUSH	AX
	CALL	FAR PTR ShrtLin_c
	POP	AX
	RET
ShrtLin	ENDP

;----------------------- Main file ---------------------
;			NcPath + CS:SI -> ES:DI
MainFil	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	ES
	POP	DS
	MOV	DX,CS			; The name is in this segment
	CALL	FAR PTR MainFil_c
	POP	DX
	POP	DS
	ADD	SI,13
	XOR	CX,CX
	MOV	AL,0
	RET
MainFil	ENDP

;-------------- Output number into buffer --------------
;			ES:DI - address of buffer
;			BX CX - number
;			   AL - mode: b0-coma, b1-spaces
Number	PROC	NEAR
	CallC	Number_c
	RET
Number	ENDP

;-------------------- Output hex byte ------------------
;			   AL - code
HexByt	PROC	NEAR
	CALL	FAR PTR HexByt_c
	RET
HexByt	ENDP

;-------------------- Text hex number ------------------
;			   AL - ASCII byte
;	Return :	   AL - hex number, CY=0
;				     error, CY=1
TstHex	PROC	NEAR
	PUSH	DS
	PUSH	BX
	PUSH	ES
	POP	DS
	MOV	BH,AH
	CALL	FAR PTR TstHex_c
	NEG	AH			; CY if it is no digit
	MOV	AH,BH
	POP	BX
	POP	DS
	RET
TstHex	ENDP

;---------------------- Number input -------------------
;			ES:SI - string
;	Return :	   AX - number
;			 CY=1 - error
TxtNum	PROC	NEAR
	PUSH	BX
	PUSH	AX			; Room for the number
	MOV	BX,SP
	PUSH	SI
	CALL	FAR PTR TxtNum_c
	POP	AX			; Where the text begins
	CMP	AX,SI
	CMC				; CY if no digit is there
	POP	AX
	POP	BX
	RET
TxtNum	ENDP

;--------------- Convert name & extension --------------
;			DS:SI - address of file
;			ES:DI - convert buffer
Convert	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	AX
	MOV	DX,DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Convert_c
	POP	AX
	POP	DX
	POP	DS
	RET
Convert	ENDP

;-------------- Unconvert name & extension -------------
;			DS:SI - convert buffer
;			ES:DI - address of file
UnConv	PROC	NEAR
	PUSH	DX
	MOV	DX,DS
	CALL	FAR PTR UnConv_c
	POP	DX
	ADD	SI,11
	RET
UnConv	ENDP

;--------------------- Test mask file ------------------
;			DS:SI - address of file
;			ES:DI - converted mask
;	Return :	 CY=1 - yes
;			 CY=0 - no
TstMas	PROC	NEAR
	PUSH	DS
	PUSH	DX
	PUSH	AX
	MOV	DX,DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR TstMas_c
	SHR	AX,1			; CY if it fits
	POP	AX
	POP	DX
	POP	DS
	RET
TstMas	ENDP

;------------------------ Set DTA ----------------------
; C: SetDTA_c of port/system.c.
SetDTA	PROC	NEAR
	PUSH	AX
	CallC	SetDTA_c
	POP	AX
	RET
SetDTA	ENDP

;------------------- Test drive letter -----------------
;			   AL - drive letter
;	Return :	   ZR - yes
;			   NZ - no
; C: TstDrv_c of port/system.c, which only C code calls.

;--------------- Int 21h with fatal error --------------
; C: Intr21f_c of port/system.c. The registers go there on the stack, as
; struct DosRegs of port/vc.h has them, with the carry flag as a number in
; AX: a call of DOS that leaves the flag as it finds it comes back with
; what the caller had. AX, BX, CX and DX come back as DOS leaves them, and
; AX is not 0 afterwards if the carry flag is to be set. DOS is called
; with ES on the main data, which is found as the routine found it: no
; caller has DS there for sure. ES comes back as DOS leaves it.
Intr21	PROC	NEAR
	PUSH	ES
	PUSH	DI
	PUSH	SI
	PUSH	DS
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	SBB	AX,AX			; The carry flag as it comes
	MOV	BX,SP
	MOV	DS,CS:SecDat
	CALL	FAR PTR Intr21f_c
	NEG	AX			; CY for an error
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DS
	POP	SI
	POP	DI
	POP	ES
	RET
Intr21	ENDP

;------------------- Invert variable -------------------
;			ES:BX - address of variable
; C: InvVar_c of port/misc.c, which returns what the variable is then.
; AH stays.
InvVar	PROC	NEAR
	PUSH	CX
	MOV	CH,AH
	CALL	FAR PTR InvVar_c
	MOV	AH,CH
	POP	CX
	RET
InvVar	ENDP

;----------------------- Check sum ---------------------
;			DS:SI - address
;			   CX - length
;	Return :	   AX - check sum
; C: ChkSum_c of port/misc.c, which adds up in the caller's DS: the main
; data.
ChkSum	PROC	NEAR
	CALL	FAR PTR ChkSum_c
	RET
ChkSum	ENDP

;----------------- CALL to PSP segment -----------------
;			   AX - offset in PSP
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