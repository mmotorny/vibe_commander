;-------------------------------------------------------
;		   Idea :	15.05.1991
;		  Begin :	16.05.1991
;		Version :	16.06.2000
;-------------------------------------------------------

	OPTION	OLDSTRUCTS		; Structure fields are global names
	OPTION	NOSCOPED		; Labels inside procedures are global

;-------------- The two parts of the program -----------
; The program is in two parts. One stays in the memory while a command
; runs: it begins with the PSP, and is made of the four segments here,
; in this order. RHEAD is what of it is still assembly, RTEXT its
; compiled code, RDATA its data, which ends with the stack, and RGAP the
; paragraph in which DOS keeps the header of the block of memory that
; follows. The other part, which goes while a command runs, begins behind
; them, at TrnBeg, and is all that follows. It begins with CONST, the
; texts, and not with code: the machine of the tests is many times slower
; at every write to a page of the memory that has code which it has run,
; and the stack ends just in front. Behind the texts come the constants of
; the C code, CONST2, which the compiler puts into this group, and the
; tables here, _DATA, which are of the same class: a text that goes to C
; still stands in front of the code. The segments are declared before the
; model so that the linker puts them first, and they are of one group
; with the rest of the assembly, so that every offset here counts from
; the PSP.
RHEAD	SEGMENT	PARA PUBLIC 'RHEAD'
RHEAD	ENDS
RTEXT	SEGMENT	BYTE PUBLIC 'RCODE'
RTEXT	ENDS
RDATA	SEGMENT	WORD PUBLIC 'RDATA'
RDATA	ENDS
RGAP	SEGMENT	PARA PUBLIC 'RGAP'
RGAP	ENDS
CONST	SEGMENT	WORD PUBLIC 'CONST'
TrnBeg	LABEL	BYTE
CONST	ENDS
CONST2	SEGMENT	WORD PUBLIC 'DATA'
CONST2	ENDS
	.MODEL	Tiny
DGROUP	GROUP	RHEAD, RTEXT, RDATA, RGAP, CONST, CONST2

;------------------- Routines written in C -------------
; They are compiled into code segments of their own, which follow this
; one. C code expects DS to point at the main data segment and may change
; ES. What is called from here is the main loop, which calls everything
; else, the start of the program, and what a panel is read by.
	EXTERN	MainLoop_c:FAR, Exit_c:FAR
	EXTERN	Init00_c:FAR, Init11_c:FAR, IniData_c:FAR
	EXTERN	ReadTre_c:FAR
	EXTERN	ReDir_c:FAR, ReDr45_c:FAR, ReDr50_c:FAR, RdInfo_c:FAR
; The compiled code of the part that stays in the memory, port/resident.c:
; the interrupt handlers, which the code that starts the program names,
; and what the assembly of that part calls, near and in its own group.
	EXTERN	New21:FAR, New27:FAR, CtrlC:FAR, SetBrk:FAR, Exit01:FAR
	EXTERN	Release_r:NEAR

; In the part that stays in the memory, what is kept while a command
; runs: the copies of the two panels, of which the C code of the main
; loop names the first, and of the main data, for F21_30_c to fill.
	PUBLIC	WCB1sav, WCB2sav, ComnIni
; What port/resident.c keeps of the interrupts and of the programs that
; have stayed in the memory: its data is laid out here, in RDATA.
	PUBLIC	IntTab, SavePtr, Vect21, Vect22, Vect27, NameBuf, TrnAdr
; What it reads in the head of that part, and where it enters the part
; that goes.
	PUBLIC	RunPar, ChkFile, ImgEnd, TrnPrW, OvlPrm
	PUBLIC	RelTab, RelCnt, ResPrW, DatPrW, ResCnt
	PUBLIC	NoRun_S, FnFil_S, BadFl_S, Retr_S, Aloc_S
	PUBLIC	Init00, Init11

; Call a routine written in C from where ES points at the main data
; segment and DS may not.
CallC	MACRO	Name
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Name
	POP	DS
	ENDM

; Enter the main loop, which is C, at the place Entry: see MainGo. The
; registers go on the stack as struct Regs of port/main.c has them.
MainC	MACRO	Entry
	PUSH	AX			; Room for where the program goes on
	PUSH	DS
	PUSH	BP
	PUSH	DI
	PUSH	SI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	AX,Entry
	JMP	MainGo
	ENDM

RHEAD	SEGMENT
	PUBLIC	_md
_md	LABEL	BYTE			; The main data segment, to C code:
					; whatever is at offset 0 of DS
	ORG	100h			; Behind the PSP

;-------------- Layout of the linked program -----------
; This block is expected at 100h by tools/mkexe.c, which fills in the
; first three words and the last. Of the three between it reads the
; offsets that the linker has put there, and writes them back in
; paragraphs. The program is entered at Alloc0 of port/resident.c, which
; the linker is told and tools/mkexe.c writes into the header of the
; file.
ImgEnd	DW	0			; End of the program, paragraphs from PSP
RelTab	DW	0			; Relocation table, paragraphs from PSP
RelCnt	DW	0			; Number of relocations
TrnPrW	DW	OFFSET TrnBeg		; Transit part, paragraphs from PSP
ResPrW	DW	OFFSET LastDat		; Size of the resident part
DatPrW	DW	OFFSET IntTab		; Its data, paragraphs from PSP
ResCnt	DW	0			; Number of relocations of that part
RHEAD	ENDS

; Neither is known before the program is linked, since compiled code
; stands between the PSP and TrnBeg: they are read where SS is on the
; resident part, which is everywhere but in the interrupt handlers.
TrnPar	EQU	<WORD PTR SS:TrnPrW>
ResPar	EQU	<WORD PTR SS:ResPrW>

PrgName	EQU	'Volkov Commander'
HelpVer	EQU	'VC 4.05 Help'
VcVers	EQU	<'4.05'>
VcApiVr	EQU	42

MoDelay	EQU	40			; Mouse double click delay
; The original has 100h bytes of stack. Compiled code uses more of it than
; the assembly did, so the port has sixteen times that, and its use needs
; no care. The stack is the end of the resident part: see DataSav.
LenStk	EQU	1000h			; Stack size
LenPath	EQU	68			; Length of path name
LenColr	EQU	40			; Length of color table
LenHist	EQU	512			; Length of hisory buffer
LenExt	EQU	512			; Length of extension file buffer
LenMenu	EQU	4000			; Length of menu buffer
LenStr	EQU	64			; Length of path & file string
LenCmd	EQU	124			; Length of command line
LenBat	EQU	1000			; Length of full command line
LenHelp	EQU	3000			; Maximal length of help page
MinFls	EQU	256			; Minimum files in window
MinTree	EQU	768			; Minimum tree size
MinView	EQU	8000			; Minimum view buffer size
MaxLins	EQU	50			; Maximum screen lines
MaxProg	EQU	18			; Maximal resident quantaty
NmbStrs	EQU	32			; Number of stars

;------------------ Global structures ------------------
TYPES	UNION
  Byt	DB	?			; Byte definition
  Wrd	DW	?			; Word definition
TYPES	ENDS

; Far address structure definition.
FARAD	STRUC
  Offs	DW	?			; Offset
  Segm	DW	?			; Segment
FARAD	ENDS

; Long type structure definition.
LONG	STRUC
  Lo	DW	?			; Low word
  Hi	DW	?			; High word
LONG	ENDS

; Coordinates structure definition.
COORD	STRUC
  X	DB	?			; Column
  Y	DB	?			; Row
COORD	ENDS

; DTA structure definition.
DTAs	STRUC
	  DB	15h DUP(?)
  FilAttr DB	?
  FilTime DW	?
  FilDate DW	?
  FilSize DD	?
  FilName DB	13 DUP(?)
	  DB	?
DTAs	ENDS

; Execute program block structure definition.
EPBs	STRUC
  EnvSeg  DW	?			; Segment of new environment
  CmdLine DD	?			; Pointer to command line
  FCB1	  DD	?			; Pointer to FCB1
  FCB2	  DD	?			; Pointer to FCB2
EPBs	ENDS

; Country information structure definition.
CNTRY	STRUC
  DateFmt DW	?			; Date format
	  DB	5 DUP(?)		; Currency
  Sep1000 DB	2 DUP(?)		; Thousand separator
	  DB	2 DUP(?)		; Decimal separator
  DateSep DB	2 DUP(?)		; Date separator
  TimeSep DB	2 DUP(?)		; Time separator
	  DB	?			; Currency format
	  DB	?			; Number of meaning digits in currency
  TimeFmt DB	?			; Time format
  CaseMap DD	?			; Addres of case map
	  DB	2 DUP(?)		; Data separator
	  DB	8 DUP(?)		; Reserved
CNTRY	ENDS

; File entry in DskBuf.
FILENT	STRUC
  NameF   DB	13 DUP(?)
  AttrF   DB	?
  TimeF   DW	?
  DateF   DW	?
  SizeF   DD	?
  NumbF   DW	?
FILENT	ENDS

; Subdirectory entry of NCD file.
DIRENT	STRUC
  DirName  DB   13 DUP(?)		; Subdirectory name in ASCIIZ
  DirLevel DB	?			; Subdirectory level
  DirShift DB	?			; 1 in next entry - there is branch
  DirCont  DB	?			; 0 - end of branch 'À', 1 - 'Ã'
DIRENT	ENDS

; Structure of NCD file.
DIRTREE	STRUC
  TreHead  DB	   5 DUP(?)		; Tree file header 'PNCI',0
  TreeLen  DW	   ?			; Number of subdirectories
  TreSum1  DW	   ?			; Check sum of the header
  TreList  DB	   0 DUP(?)		; List of subdirectories
  TreSum2  DW	   ?			; Check sum of the list
DIRTREE	ENDS

; The dialog box header structure definition.
BOXHD	STRUC
  pDiBox  DW	?			; Pointer to DialBox structure
  BoxBase DW	?			; The dialog box begin-center
  SubFunc DW	?			; Subfunction
  bKeyBar DB	?			; KeyBar
  bMenu   DB	0FFh			; PutMnu
  bHelp   DB	0			; HlpPage
  bFlags  DB	0			; Flags
BOXHD	ENDS

; The dialog box header structure definition.
BOXMN	STRUC
  mType	  DB	?			; Menu type
  mUp	  DB	0			; Up link
  mDown	  DB	0			; Down link
  mLeft	  DB	0			; Left link
  mRight  DB	0			; Right link
  mBase	  DW	?			; Menu base
  mPoint  DW	?
  mCtrl	  DW	?
BOXMN	ENDS
mSelTot	=	BYTE PTR mCtrl

; The star structure definition.
STAR	STRUC
  StarClk  DB	   0 DUP(?)		; Time counter
  StarMod  DW	   ?			; Low byte - counter, High - explosion
  StarPos  DW	   ?			; Coordinates of star
STAR	ENDS

;--------------------- Data segment --------------------
MainDat	STRUC
ColTab	DB	6*LenColr DUP(?)	; Color table (6 pallets)
FulScr	DB	?			; Full screen
MenuVs	DB	?			; Menu always visible
KeyBar	DB	?			; Key bar
Prompt	DB	?			; Path prompt
InsDwn	DB	?			; Ins moves down
AutoMnu	DB	?			; Auto user menus
MiniSta	DB	?			; Mini status
Clock	DB	?			; Clock
AutoCD	DB	?			; Auto change dir
AutoSav	DB	?			; Auto save setup
ColTyp	DB	?			; Monochrome / Color / LapTop
BlDelay	DB	?			; Screen blank delay
LoadFl	DB	?			; Small mode
UnSlct	DB	?			; Unselect files when group operation
ErrSnd	DB	?			; Beep when error
LftMous	DB	?			; Left handed mouse
ZoomWin	DB	?			; Zomming windows
AltBar	DB	?			; Alt alone selects menu
EscBar	DB	?			; Esc toggles panels on/off
ExecTyp	DB	?			; Exec command without COMMAND.COM
AutoSiz	DB	?			; Auto calculate directory sizes
HidDirs	DB	?			; Show hidden subdirs in tree panel
QuitAck	DB	?			; Confirmation on Quit
SnowMod	DB	?			; Snow mode (2=auto)
AltView	DB	?			; Alt-F3 - external viewer
AltEdit	DB	?			; Alt-F4 - external editor
ClrBuf	DB	?			; Clear buffer flag
Active	DB	?			; Active panel
Ctrl_O	DB	?			; Visible status of unactive panel
ChkTSR	DB	?			; TSR manager
ChgTree	DB	?			; Auto rescan tree
DriveB	DB	?			; Show logic drive B: in 1 floppy system
DelayTm	DB	?, ?			; Auto change dir delay time
StarTab	DB	5  DUP (?)		; Star table
ResIni	DB	51 DUP (?)		; Reserved
CmnIni	=	$			; End of main data' part saved in VC.INI

LoadRes	DB	?			; Original LoadFl
ScrMod	DB	?			; Current screen mode
Mouse	DB	?			; 1 if Mouse was found
BreakFl	DB	?			; Break DOS's status
MenuFl	DB	?			; 1 to bring user menu automatically
SelMas	DB	13 DUP(?)		; Wildcard for selection
HstBuf	DB	LenHist DUP(?)		; History buffer
ExtMain	DB	LenExt DUP(?)		; Buffer for the main extension file
EPB	EPBs	<>			; Execute program block
MouseX	DW	?			; Mouse cursor X
MouseY	DW	?			; Mouse cursor Y
BatAdr	DW	?			; Pointer to command in buffer
BatPoin	DW	?			; Pointer to command
LastMnu	DW	?			; The last pull down menu item
OvlPth	DW	?			; Pointer to file name in environment
ComSpec	DW	?			; Pointer to COMSPEC= in environment
CrnPath	DB	LenPath DUP(?)		; Current directory in the tree panel
ViewFil	DB	LenStr+1 DUP(?)		; Source filename
ToFile	DB	LenStr+1 DUP(?)		; Target filename
FindStr	DB	LenStr+1 DUP(?)		; FileFind wildcard
SrchStr	DB	LenStr+1 DUP(?)		; String for search
DosChg	DB	5 DUP(?)		; Reserved for change drive or /C_ 
DosBuf	DB	LenBat DUP(?)		; Command line
CmnSave	=	$			; End of main data' part saved in memory

DosLin	DB	?			; Line number of DOS's command line
DosCol	DB	?			; Current column of DOS's command line
BegCol	DB	?			; Start position of DOS's command line
OutCol	DB	?			; Start position for output com. line
ScrPage	DB	?			; Current screen page number
TstEGA	DB	?			; 1 if EGA or VGA present
Lines	DB	?			; Number of lines on screen
Snow	DB	?			; Snow on/off
TopView	DB	?			; TopView output
DesqVie	DB	?			; Running under DESQVIEW
WinLeav	DB	?			; Leave time under Windows
CrntCol	DB	?			; Color which DOS currently uses
Quote	DB	?			; Quote next character
BarPrev	DB	?			; Previous Key Bar
SftErr	DB	?			; Shift error boxes down
BlocEr	DB	?			; Disable critical error box
PathEr	DB	?			; Error when reading current path
FlagCD	DB	?			; 1 when needs auto CD
TreeTyp	DB	?
WinMous	DB	?
MousEnt	DB	?
OldFlg	DB	?
HlpPage	DB	?
ZoomOne	DB	?
OneFile	DB	?			; Do one file and exit
ErrDat	DB	?
VerDOS	DW	?
WCBcrn	DW	?
WCBsec	DW	?
WCB1seg	DW	?
WCB2seg	DW	?
ViewSeg	DW	?
FreMem1	DW	?
FreMem2	DW	?
Memorys	DW	?
HstAdr	DW	?
TreAdr	DW	?
TreFrst	DW	?
TreCrnt	DW	?
ClstSiz	DW	?
Total	DW	?
FreeDsk	DW	?
InfoLen	DW	?
Rand1	DW	?
Rand2	DW	?
FncAddr	DW	?
MousPos	DW	?
MousTim	DD	?
AutoTim	DD	?
Screen	DD	?			; Address of screen
ErrVct	DD	?			; Original vector 24h
BrkVct	DD	?			; Original vector 1Bh
Volume	DB	12 DUP(?)		; Volume label
DosPth	DB	LenPath DUP(?)		; The current path
NcPath	DB	LenPath DUP(?)		; The path from VC= variable
TempDir	DB	LenPath DUP(?)		; The path from TEMP= variable
Country	CNTRY	<>			; Country information
DTA	DTAs	<>			; Data tramsnir area
LowTab	DB	128 DUP(?)		; Lower case table
TmpBuf	DB	256 DUP(?)
TmpErr	DB	256 DUP(?)
Stars	STAR	NmbStrs DUP(<>)
MenuBuf	DB	LenMenu DUP(?)
InfBuf	DB	(MaxLins-13)* 38 DUP(?)
	EVEN
ScrBuf	DW	 MaxLins    * 80 DUP(?)
UserScr	DW	(MaxLins-1) * 80 DUP(?)
WinBuf	DW	(MaxLins-2) * 77 DUP(?)
ErrBuf	DW	(MaxLins-2) * 77 DUP(?)
ErrBuf1	DW	(MaxLins-2) * 77 DUP(?)
HelpBuf	DB	LenHelp	DUP(?)
TreeBuf	DB	MinTree*SIZE DirEnt+SIZE DirTree DUP(?)
MainDat	ENDS

;-------------------- WCB definition -------------------
WCBdef	STRUC
WinTyp	DB	?
BrfFul	DB	?
Visible	DB	?
Hidden	DB	?
MasTyp	DB	?
SortTyp	DB	?
Ctrl_L	DB	?
DskMas	DB	13	DUP(?)
WinPath	DB	LenPath	DUP(?)
WCBini	=	$			; End of WCB part saved in VC.INI

CurAdr	DW	?
First	DW	?
EndBuf	DW	?
WCBsave	=	$			; End of WCB part saved in memory

ExtBuf	DB	LenExt	DUP(?)
EndFul	DW	?
ChgFlg	DB	?
SizDir	DB	?
WCBsize	=	$			; End of WCB

DskBuf	DB	MinFls * SIZE FilEnt DUP(?)
WCBdef	ENDS

;------------------- Begin of program ------------------
RHEAD	SEGMENT
RunPar	DW	0
ChkFile	DW	0
Command	DB	'COMMAND.COM', 0
OvlPrm	DB	10, 'VVV', 0, 0, 0, 0, 0, 0, 0, 0Dh
NoRun_S	DB	'There is not enough memory to run this command', 13,10,0
FnFil_S	DB	13,10, "Can't find the file: ", 0
BadFl_S	DB	13,10, "Bad or missing ", 0
Retr_S	DB	13,10, "Press ENTER to try again or ESC to abort", 13,10,0
Aloc_S	DB	'Program too big to fit in memory', 13,10,0

;------------------- Execute commands ------------------
LoadSiz	EQU	WORD PTR DosBuf+0

; C: port/resident.c. The part that stays has the outermost loop of the
; program there, Stay, which calls the part that goes, far: at Init00
; when the program starts and at Init11 after every command. That part
; comes back by a far jump to one of the three labels here, as it always
; has, with the stack as it was when it was called: so a far return from
; here goes back into Stay, with what to do in AX. They go when what
; jumps here returns by itself.
Exec:	PUSH	CS			; Run the commands of DosBuf
	POP	DS
	MOV	AX,100h
	RETF
AlocEr:	PUSH	CS			; No memory for the main data
	POP	DS
	MOV	AX,200h
	RETF
Exit00:	PUSH	CS			; Leave, with AL for DOS
	POP	DS
	MOV	AH,0
	RETF

;-------- Programs that stay in the memory, and DOS's memory --------
; C: port/resident.c, which has the interrupt handlers New21 and New27,
; CtrlC, what they keep of the programs that have stayed in the memory,
; and Release, which the map of the memory still calls, through CallPSP:
; near, in RTEXT, with DS on this segment.
;			   DS - PrgSeg
Release	PROC	NEAR
	PUSH	DS
	PUSH	AX
	MOV	AX,DS
	PUSH	CS
	POP	DS
	CALL	Release_r
	POP	AX
	POP	DS
	RETF
Release	ENDP

RHEAD	ENDS

;--------------------- Release data --------------------
RDATA	SEGMENT
IntTab	DD	256	DUP(?)
SavePtr	DD	?
Vect21	DD	?
Vect22	DD	?
Vect27	DD	?
TrnAdr	DD	?			; Where Stay calls the transit part
Counter	DW	?
PrgList	DW	MaxProg	DUP(?)
NameBuf	DB	12	DUP(?)

;------------------ Data save segment ------------------
WCB1sav	DB	WCBsave + 13 * MinFls	DUP(?)	; WCB1
WCB2sav	DB	WCBsave + 13 * MinFls	DUP(?)	; WCB2
ComnIni	DB	CmnSave			DUP(?)	; Common data
	PUBLIC	DataSav, VcStack	; For the tests: how long the stack is
DataSav	DB	LenStk		DUP(?)	; Local stack
VcStack:
LastDat:
RDATA	ENDS

RGAP	SEGMENT
	DB	16	DUP(?)		; The header of the transit part's block
RGAP	ENDS

;-------------------------- Data -----------------------
; The texts, the boxes, the dialogs and the tables of the program are C:
; the files of constants, port/*_t.c.
	.DATA
Back_C	EQU	0
Cursr_C	EQU	1
Slct_C	EQU	2
CurSl_C	EQU	3
Title_C	EQU	4
KeyNu_C	EQU	5
KeyTi_C	EQU	6
Menu_C	EQU	7
MnCur_C	EQU	8
MnuBx_C	EQU	9
MnuTx_C	EQU	10
MnuBr_C	EQU	11
MnuCu_C	EQU	12
MnuCB_C	EQU	13
MnuDr_C	EQU	14
MnuMi_C	EQU	15
Box_C	EQU	16
BoxBr_C	EQU	17
BoxCu_C	EQU	18
Conf_C	EQU	19
CnfRe_C	EQU	20
CnfBo_C	EQU	21
CnfUn_C	EQU	22
Hist_C	EQU	23
HstCu_C	EQU	24
HlBrd_C	EQU	25
HlpMn_C	EQU	26
Help_C	EQU	27
HlpBo_C	EQU	28
HlpRe_C	EQU	29
HlpUn_C	EQU	30
Err_C	EQU	31
ErInv_C	EQU	32
ErBr_C	EQU	33
Clock_C	EQU	34
Arrow_C	EQU	35
Sky1_C	EQU	36
Sky2_C	EQU	37
ShadFl	EQU	38

;--------------------- Main module ---------------------
	.CODE

;------------------ Read configuration -----------------
; C: Init00_c, Init11_c and IniData_c of port/start.c. The part of the
; program that stays in the memory calls here, far, and is come back to
; by a far jump to one of three labels of its own (see Exec in RHEAD): at
; Init00 when the program has been started, at Init11 when a command has
; run. It has relocated this part by then: Reloc of port/resident.c.
; CmdIni of VCFUNC.INC jumps to Init11. The stack is in that part every
; time,
; and its segment is what the C is told of it. Until Init11 has asked DOS
; for the main data there is none: the C does without DS up to there.
; What it reads, writes and names of the assembly is made public here.
	PUBLIC	SecDat
	PUBLIC	Counter, Vect21, Vect22, Vect27, RelTab
Init00:	CLD
	MOV	AX,SS
	CALL	FAR PTR Init00_c

;------------ Allocate Secondary Data Segment ----------
; The block of the memory that this code is in goes to the C in DX. What
; comes back is the segment of the main data, or 0 if there is too
; little memory: the part that stays says so and ends the program.
; DX is then how many paragraphs there were, and goes back to that
; part as it is, through AlocEr there: the original has them in BX
; when it says so, where the BIOS takes a page.
Init11:	CLD
	MOV	AX,SS
	MOV	DX,CS
	ADD	DX,TrnPar
	CALL	FAR PTR Init11_c
	OR	AX,AX
	JNZ	IniOk
	MOV	AX,OFFSET AlocEr
	PUSH	SS
	PUSH	AX
	RETF
IniOk:	MOV	DS,AX
	CALL	FAR PTR IniData_c
	PUSH	DS
	POP	ES

;----------------------- Set cursor --------------------
; From here on the program is C: MainLoop_c of port/main.c, the main
; loop, which calls every command. The code above falls into it. The
; function gets the number of the place where the loop is entered, 0 for
; its start, and room on the stack for the registers that go round it
; (struct Regs of port/vc.h): MainC, at the top of this file, pushes them.
; It returns when the program goes on outside it, with the place in AX, a
; number of Main_T.
Init40:	MainC	0

MainGo:	MOV	BX,SP
	PUSH	ES
	POP	DS
	CLD
	CALL	FAR PTR MainLoop_c
	PUSH	DS
	POP	ES
	MOV	BX,AX
	ADD	BX,BX
	MOV	AX,CS:Main_T[BX]
	MOV	BX,SP
	MOV	SS:[BX+16],AX
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	SI
	POP	DI
	POP	BP
	POP	DS
	RET				; To the place of Main_T

; Where the program goes on when the loop returns: it starts over, has a
; command run first, or leaves. port/main.c has these as ASM_.
	.DATA
Main_T	DW	CmdIni, CmdExe, Exit

	.CODE
; An entry for the C code of the loop (see port/main.c for what it takes
; and returns). Read what a panel shows: ReDir, with the panel in DX and
; in AL whether to keep the cursor and the marks. AX is not 0 if it set
; the carry flag.
	PUBLIC	ReDir_a
ReDir_a	PROC	FAR
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	DS,DX
	CALL	ReDir
	SBB	AX,AX
	POP	DS
	RET
ReDir_a	ENDP

	INCLUDE	VCMEM.INC

;------------------- Start over, or run -----------------
; Enter does not come back when it runs something: F21_30_c of
; port/enter.c has saved what the program needs in the part that stays in
; the memory, and given the main data back to DOS. That part runs the
; commands: Exec there, which gets in ES the block of the memory that the
; rest of the program is in, to give it back to DOS as well. With nothing
; to run the program starts over at once.
CmdExe:	PUSH	SS
	POP	DS
	MOV	AX,CS
	ADD	AX,TrnPar
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

;------------------------ Exit -------------------------
; C: Exit_c of port/main.c puts the screen back. The part of the program
; that stays in the memory ends it.
Exit:	CallC	Exit_c
	MOV	AX,OFFSET Exit00
	PUSH	SS
	PUSH	AX
	MOV	AL,0
	RETF

;--------------------- Fatal error ---------------------
	.DATA
SecDat	DW	0

	.CODE
; C: FatErr of port/main.c, an interrupt function of the compiler's. Its
; texts and its boxes are C too, port/main_t.c. SecDat stays here, which
; is where it finds the main data.

;-------------------- Not ready error ------------------
;			   AL - Drive letter
;	Return:		   AL - Drive letter
; C: ErrRdy_c of port/main.c, which only C code calls, with its box in
; port/main_t.c.

	INCLUDE	VCSUB1.INC
	INCLUDE	VCSUB3.INC

	.CONST
	ASSUME	CS:DGROUP
Last:

	END
