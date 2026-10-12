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
; and the stack ends just in front. The segments are declared before the
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
	.MODEL	Tiny
DGROUP	GROUP	RHEAD, RTEXT, RDATA, RGAP, CONST

;------------------- Routines written in C -------------
; They are compiled into a code segment of their own, which follows this
; one, and are called through the routines of the same names here. C code
; expects DS to point at the main data segment and may change ES.
	EXTERN	AddFile_c:FAR, MvzLine_c:FAR
	EXTERN	CutPath_c:FAR
	EXTERN	PathLin_c:FAR
	EXTERN	AdrNum_c:FAR, NumAdr_c:FAR, ChgAdr_c:FAR
	EXTERN	F00_60_c:FAR, F00_00_c:FAR, F00_05_c:FAR
	EXTERN	F01_00_c:FAR, F01_05_c:FAR, F02_00_c:FAR, F03_00_c:FAR
	EXTERN	F04_00_c:FAR, F05_00_c:FAR, F06_00_c:FAR, F07_00_c:FAR
	EXTERN	F08_00_c:FAR, F76_00_c:FAR, F76_01_c:FAR, F09_00_c:FAR
	EXTERN	F10_00_c:FAR, F78_00_c:FAR, F11_00_c:FAR, F12_00_c:FAR
	EXTERN	F13_00_c:FAR, F14_00_c:FAR, F15_00_c:FAR, F16_00_c:FAR
	EXTERN	F17_01_c:FAR, F79_00_c:FAR, F18_00_c:FAR, F20_00_c:FAR
	EXTERN	F20_02_c:FAR
	EXTERN	F39_00_c:FAR
	EXTERN	M01_00_c:FAR, M01_01_c:FAR, M03_00_c:FAR, M06_00_c:FAR
	EXTERN	M06_10_c:FAR, M08_00_c:FAR, M16_00_c:FAR, M16_01_c:FAR
	EXTERN	M16_10_c:FAR, M21_00_c:FAR, M41_00_c:FAR, M42_00_c:FAR
	EXTERN	F69_00_c:FAR, M49_00_c:FAR, M60_00_c:FAR, M61_00_c:FAR
	EXTERN	M53_00_c:FAR, M62_00_c:FAR, M63_00_c:FAR, M64_00_c:FAR
	EXTERN	M66_00_c:FAR, M67_00_c:FAR, M69_00_c:FAR, M70_00_c:FAR
	EXTERN	M71_00_c:FAR, M72_00_c:FAR, M73_00_c:FAR, M75_00_c:FAR
	EXTERN	ReadTre_c:FAR
	EXTERN	ReDir_c:FAR, ReDr45_c:FAR, ReDr50_c:FAR, RdInfo_c:FAR
	EXTERN	DiSi10_c:FAR
	EXTERN	V00_02_c:FAR, V00_10_c:FAR, V13_00_c:FAR, V13_01_c:FAR
	EXTERN	T00_00_c:FAR
	EXTERN	Help00_c:FAR
	EXTERN	F32_00_c:FAR, F62_01_c:FAR, F68_00_c:FAR
	EXTERN	F82_01_c:FAR, F74_00_c:FAR, F73_00_c:FAR, F77_00_c:FAR
	EXTERN	F80_00_c:FAR
	EXTERN	FindFil_c:FAR, F67_00_c:FAR
	EXTERN	F38_10_c:FAR, F38_40_c:FAR, M30_50_c:FAR
	EXTERN	M30_60_c:FAR, DirFunc_c:FAR, DelTr0_c:FAR
	EXTERN	F37_00_c:FAR, F47_00_c:FAR, F38_00_c:FAR, F48_00_c:FAR
	EXTERN	F58_00_c:FAR, M30_00_c:FAR, F54_00_c:FAR, FncMas_c:FAR
	EXTERN	Dialog_c:FAR
	EXTERN	C00_00_c:FAR, F35_01_c:FAR, F45_01_c:FAR, F35_61_c:FAR
	EXTERN	F65_00_c:FAR
	EXTERN	MainLoop_c:FAR, Exit_c:FAR
	EXTERN	HookVct_c:FAR
	EXTERN	ChkMode_c:FAR, Cursor_c:FAR
	EXTERN	ScrOffs_c:FAR
	EXTERN	F21_00_c:FAR, F21_10_c:FAR, F21_25_c:FAR, F21_30_c:FAR
	EXTERN	F21_31_c:FAR, F22_00_c:FAR, F23_00_c:FAR, F24_00_c:FAR
	EXTERN	F24_01_c:FAR, F24_02_c:FAR, F25_00_c:FAR, F26_00_c:FAR
	EXTERN	F27_00_c:FAR, F30_00_c:FAR, F75_00_c:FAR
	EXTERN	F43_00_c:FAR, F44_00_c:FAR, F63_00_c:FAR, F64_00_c:FAR
	EXTERN	F33_00_c:FAR, F34_00_c:FAR, F34_10_c:FAR
	EXTERN	FncSel_c:FAR, FncExt_c:FAR, FncEx1_c:FAR

; Texts here that C code shows too.
	PUBLIC	Title_S, Var_S, Tree_S, Mask_S, Ext_F
; The name of the tree file, and the boxes shown while the tree is read
; and saved, which C code hands to OncZoom.
	PUBLIC	Tree_F, Tree_P, TrSav_P
; Boxes and a text that C code hands to DialBox and Dialog.
	PUBLIC	SavFi_P, SavEr_B, Prot_B, EdFl_S
; A text, a box, and buttons with their colours, of the help.
	PUBLIC	Help_S, HlpEr_B, HlMn1_D, HlMn2_D, HlpMn_P
; The scan codes that begin the speed search, the box of a drive that is
; not ready, the routine for the critical errors of DOS, and in the part
; that stays in the memory the routine for Ctrl-Break and the copy of the
; first panel, all of which the C code of the main loop names.
	PUBLIC	AltSr_T, NoRd_P, FatErr, CtrlC, WCB1sav

; Call a routine written in C from where ES points at the main data
; segment and DS may not.
CallC	MACRO	Name
	PUSH	DS
	PUSH	ES
	POP	DS
	CALL	FAR PTR Name
	POP	DS
	ENDM

; An entry by which C code calls the near routine Name: Name_a, called far
; with DS on the main data. The assembly keeps ES there, so the entry puts
; it there too. Registers go in and come back as the routine has them. C
; code cannot see flags: a routine that answers in one names instructions
; for After that put the answer into a register.
FromC	MACRO	Name, After
	PUBLIC	Name&_a
Name&_a	PROC	FAR
	PUSH	DS
	POP	ES
	CALL	Name
	After
	RET
Name&_a	ENDP
	ENDM

; Hand a line of text to the C function Name: the registers that describe
; it are pushed, First of them last, and the function gets BX on them and
; DS on the main data (see struct LineArgs in port/line.c). TakeLine pops
; them again, as the function has left them, and with them DS. Scratch is
; a register to pop into what is of no use.
CallLine MACRO	Name, First
	PUSH	CS
	PUSH	DX
	PUSH	CS
	PUSH	BP
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	First
	MOV	BX,SP
	PUSH	ES
	POP	DS
	CALL	FAR PTR Name
	ENDM

TakeLine MACRO	First, Scratch
	POP	First
	POP	DI
	POP	SI
	POP	DS
	POP	BP
	POP	Scratch
	POP	DX
	POP	Scratch
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
Start:	JMP	Alloc0

;-------------- Layout of the linked program -----------
; This block is expected at 103h by tools/mkexe.c, which fills in the
; first three words. Of the last three it reads the offsets that the
; linker has put there, and writes them back in paragraphs.
ImgEnd	DW	0			; End of the program, paragraphs from PSP
RelTab	DW	0			; Relocation table, paragraphs from PSP
RelCnt	DW	0			; Number of relocations
TrnPrW	DW	OFFSET TrnBeg		; Transit part, paragraphs from PSP
ResPrW	DW	OFFSET LastDat		; Size of the resident part
DatPrW	DW	OFFSET IntTab		; Its data, paragraphs from PSP
RHEAD	ENDS

; Neither is known before the program is linked, since compiled code
; stands between the PSP and TrnBeg: they are read where SS is on the
; resident part, which is everywhere but in the interrupt handlers.
TrnPar	EQU	<WORD PTR SS:TrnPrW>
ResPar	EQU	<WORD PTR SS:ResPrW>

; Move DX paragraphs from segment BX to segment BP, which may overlap it
; from above. Uses no stack. Changes AX, CX, DX, SI, DI, DS and ES.
MovPars	MACRO
	LOCAL	Next, Part
	STD
Next:	MOV	CX,800h
	CMP	DX,CX
	JAE	Part
	MOV	CX,DX
Part:	SUB	DX,CX
	MOV	AX,BX
	ADD	AX,DX
	MOV	DS,AX
	MOV	AX,BP
	ADD	AX,DX
	MOV	ES,AX
	SHL	CX,1
	SHL	CX,1
	SHL	CX,1
	SHL	CX,1
	MOV	SI,CX
	DEC	SI
	MOV	DI,SI
	REP	MOVSB
	OR	DX,DX
	JNZ	Next
	CLD
	ENDM

PrgName	EQU	'Volkov Commander'
HelpVer	EQU	'VC 4.05 Help'
VcVers	EQU	<'4.05'>
VcSuff	EQU	'freeware'
VcApiVr	EQU	42

MoDelay	EQU	40			; Mouse double click delay
MiliSec	EQU	60			; auto CD time : sec x 0.01
Sec	EQU	0			;                sec x 1
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
  DirCont  DB	?			; 0 - end of branch 'А', 1 - 'Г'
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

Exec:	TEST	DS:ComnIni.LoadFl,7Fh
	JZ	Exec01
	CALL	GetStat
	MOV	AH,49h
	INT	21h
	CALL	SetStat
Exec01:	PUSH	DS
	POP	ES
	MOV	AX,OFFSET ComnIni.DosBuf
	MOV	DS:ComnIni.BatAdr,AX
Exec02:	CLD
	MOV	DL,DS:ComnIni.BreakFl
	MOV	AX,3301h
	INT	21h
	MOV	DI,DS:ComnIni.BatAdr
	LEA	SI,[DI-4]
	PUSH	DI
	MOV	CX,0FFFFh
	MOV	AL,0
	REPNE	SCASB
	MOV	DS:ComnIni.BatAdr,DI
	POP	DI
	MOV	CX,LenCmd
	REPNE	SCASB
	DEC	DI
	MOV	BYTE PTR DS:[DI],0Dh
	LEA	CX,[DI-1]
	CMP	AL,DS:ComnIni.ExecTyp
	JE	Exec04
	INC	SI
	INC	SI
	INC	SI
	SUB	CX,SI
	MOV	DS:[SI],CL
	CMP	DS:ComnIni.DosChg[1],0
	JE	Exec03
	MOV	DS:ComnIni.BatPoin,SI
	MOV	SI,OFFSET ComnIni.DosChg
	INT	2Eh
	CLI
	MOV	SP,CS
	MOV	SS,SP
	MOV	SP,OFFSET VcStack
	STI
	PUSH	CS
	POP	DS
	MOV	SI,DS:ComnIni.BatPoin
	MOV	DS:ComnIni.DosChg[1],0
Exec03:	INT	2Eh
	CLI
	MOV	SP,CS
	MOV	SS,SP
	MOV	SP,OFFSET VcStack
	STI
	CALL	SetBrk0
	JMP	Exec07
Exec04:	SUB	CX,SI
	MOV	CH,'/'
	MOV	DS:[SI],CX
	MOV	WORD PTR DS:[SI+2],' C'
	MOV	BX,OFFSET ComnIni.EPB
	MOV	DS:[BX].CmdLine.Offs,SI
	MOV	DS:[BX].CmdLine.Segm,DS
Exec05:	MOV	DI,OFFSET Command
	MOV	DX,DS:ComnIni.ComSpec
	OR	DX,DX
	JE	Exec06
	MOV	DS,DS:[2Ch]
	MOV	AX,4B00h
	INT	21h
	CALL	SetBrk0
	JNC	Exec07
	MOV	DS,ES:[2Ch]
	MOV	DI,ES:ComnIni.ComSpec
	CMP	AX,WORD PTR 3
	JBE	Exec06
	MOV	SI,OFFSET NoRun_S
	PUSH	CS
	CALL	OutStr0
	JMP	Exec10
Exec06:	MOV	SI,OFFSET FnFil_S
	CALL	NotFind
	PUSH	ES
	POP	DS
	JNC	Exec05
	JMP	Exec10
Exec07:	PUSH	ES
	POP	DS
	MOV	BX,DS:ComnIni.BatAdr
	CMP	BYTE PTR DS:[BX],0
	JE	Exec10
	CMP	BYTE PTR DS:[BX],'@'
	JNE	Exec08
	INC	BX
	MOV	DS:ComnIni.BatAdr,BX
	JMP	Exec02
Exec08:	MOV	DI,OFFSET IntTab
	PUSH	DI
	MOV	AX,0A0Dh
	STOSW
	MOV	AH,19h
	INT	21h
	MOV	DL,AL
	INC	DX
	ADD	AL,'A'
	STOSB
	CMP	DS:ComnIni.Prompt,0
	JE	Exec09
	MOV	AX,'\:'
	STOSW
	MOV	SI,DI
	MOV	AH,47h
	INT	21h
	MOV	CX,0FFFFh
	MOV	AL,0
	REPNE	SCASB
	DEC	DI
Exec09:	MOV	AX,'>'
	STOSW
	POP	SI
	PUSH	CS
	CALL	OutStr
	MOV	SI,DS:ComnIni.BatAdr
	PUSH	CS
	CALL	OutStr
	MOV	AX,0E0Dh
	INT	10h
	MOV	AX,0E0Ah
	INT	10h
	JMP	Exec02
Exec10:	MOV	AL,DS:ComnIni.LoadRes
	XCHG	AL,DS:ComnIni.LoadFl
	TEST	AL,7Fh
	JNZ	Exec11
	JMP	NEAR PTR Exec15
Exec11:	CALL	GetStat			; Load VC.EXE
	MOV	BX,CS:ImgEnd
	SUB	BX,TrnPar
	MOV	AH,48h
	INT	21h
	CALL	SetStat
	SUB	AX,TrnPar
	MOV	CS:RunPar,AX
	JNC	Exec12
	JMP	AlocEr
; The original stores the size through DS, which is on the environment
; when ENTER brings it here to try again: a word written into other data.
; ES is on the resident part either way. See harness/deviations.py.
Exec12:	MOV	AX,CS:ImgEnd
	MOV	ES:ComnIni.LoadSiz,AX
	MOV	SI,OFFSET OvlPrm
	MOV	BL,CS:[SI]
	MOV	BH,0
	MOV	CS:[SI+BX-1],CS
	MOV	BX,OFFSET ComnIni.EPB
	MOV	ES:[BX].CmdLine.Offs,SI
	MOV	ES:[BX].CmdLine.Segm,CS
	MOV	DX,ES:ComnIni.OvlPth
	MOV	DS,CS:[2Ch]
	MOV	AX,4B00h
	INT	21h
	PUSH	CS
	POP	ES
	CLD
	JNC	Exec14
	CMP	AL,3
	JA	AlocEr
Exec13:	MOV	DI,ES:ComnIni.OvlPth	; Error: VC.EXE not found
	MOV	DS,CS:[2Ch]
	MOV	SI,OFFSET BadFl_S
	CALL	NotFind
	JNC	Exec12
	MOV	AL,1
	JMP	Exit00
Exec14:	MOV	AH,4Dh
	INT	21h
	OR	AX,AX
	JNE	Exec13
	PUSH	ES
	POP	DS
	MOV	AX,DS:ComnIni.LoadSiz
	CMP	AX,DS:ChkFile
	JNE	Exec13
Exec15:	PUSH	CS:RunPar
	MOV	AX,OFFSET Init11
	PUSH	AX
	RETF

AlocEr:	MOV	SI,OFFSET Aloc_S
	PUSH	CS
	CALL	OutStr0
	MOV	AL,1
Exit00:	MOV	BX,CS:Vect22.Offs
	MOV	CS:[0Ah].Offs,BX
	XOR	BX,BX
	XCHG	BX,CS:Vect22.Segm
	MOV	CS:[0Ah].Segm,BX
Exit01:	PUSH	AX
	PUSH	CS
	POP	ES
	MOV	DL,ES:ComnIni.BreakFl
	MOV	AX,3301h
	INT	21h
	MOV	CX,ES:Counter
	JCXZ	Exit03
Exit02:	MOV	SI,ES:Counter
	SHL	SI,1
	MOV	DS,ES:PrgList[SI-2]
	PUSH	CS
	CALL	Release
	LOOP	Exit02
Exit03:	CMP	ES:ComnIni.ChkTSR,0
	JZ	Exit04
	LDS	DX,ES:Vect21
	MOV	AX,2521h
	INT	21h
	LDS	DX,ES:Vect27
	MOV	AX,2527h
	INT	21h
Exit04:	POP	AX
	CMP	ES:Vect22.Segm,0
	JNE	Exit05
	MOV	AH,4Ch
	INT	21h
Exit05:	JMP	ES:Vect22

NotFind	PROC	NEAR
	PUSH	DS
	PUSH	CS
	CALL	OutStr0
	POP	DS
	MOV	SI,DI
	PUSH	CS
	CALL	OutStr
	PUSH	DS
	MOV	SI,OFFSET Retr_S
	PUSH	CS
	CALL	OutStr0
	POP	DS
NotFn1:	MOV	AH,0
	INT	16h
	CMP	AL,0Dh
	JE	NotFn2
	CMP	AL,1Bh
	JNE	NotFn1
	STC
NotFn2:	RET
NotFind	ENDP

OutStr0	PROC	NEAR
	PUSH	CS
	POP	DS
OutStr:	PUSH	BP
	PUSH	BX
	MOV	BL,7
	LODSB
	MOV	AH,0Eh
	INT	10h
	POP	BX
	POP	BP
	CMP	BYTE PTR DS:[SI],0
	JNE	OutStr
	RETF
OutStr0	ENDP

SetBrk0	PROC	NEAR
	CLD
	PUSH	SS
	POP	ES
	POP	BP
	PUSH	CS
	PUSH	BP
SetBrk0	ENDP

SetBrk	PROC	NEAR
	PUSH	DS
	PUSH	AX
	PUSHF
	PUSH	SS
	POP	DS
	MOV	AX,3300h
	INT	21h
	MOV	DS:ComnIni.BreakFl,DL
	MOV	DL,0
	MOV	AX,3301h
	INT	21h
	MOV	DX,OFFSET CtrlC
	MOV	AX,2523h
	INT	21h
	POPF
	POP	AX
	POP	DS
	RETF
SetBrk	ENDP

GetStat	PROC	NEAR
	MOV	CX,0FFFFh
	MOV	AX,5802h
	INT	21h
	JC	GetSt1
	MOV	CL,AL
	MOV	AX,5800h
	INT	21h
	MOV	CH,AL
	MOV	BX,1
	MOV	AX,5803h
	INT	21h
	MOV	BX,80h
	MOV	AX,5801h
	INT	21h
GetSt1:	MOV	BP,CX
	RET
GetStat	ENDP

SetStat	PROC	NEAR
	PUSH	AX
	PUSHF
	CMP	BP,0FFFFh
	JE	SetSt1
	MOV	BX,BP
	MOV	BL,BH
	MOV	BH,0
	MOV	AX,5801h
	INT	21h
	MOV	BX,BP
	MOV	BH,0
	MOV	AX,5803h
	INT	21h
SetSt1:	POPF
	POP	AX
	RET
SetStat	ENDP

CtrlC:	IRET

New21	PROC	FAR			; INT 21h
	CMP	AH,4Bh
	JE	Fnc4B
	CMP	AH,31h
	JE	Fnc31
	CMP	AH,49h
	JE	Fnc49
	CMP	AH,2Bh
	JE	VCapi
Old21:	JMP	CS:Vect21
Fnc4B:	CALL	ExeProg
	JMP	Old21
Fnc31:	CALL	AddProg
	JNC	Old21
	MOV	AH,4Ch
	JMP	Old21
Fnc49:	CALL	FreeBlk
	JMP	Old21
VCapi:	CMP	CX,'VC'
	JNE	Old21
	CMP	DX,'OM'
	JNE	Old21
	PUSH	CS
	POP	ES
	CMP	AL,1
	MOV	BX,OFFSET WCB1sav
	JB	VCapi1
	MOV	BX,OFFSET WCB2sav
	JE	VCapi1
	MOV	BX,OFFSET ComnIni
VCapi1:	MOV	AX,VcApiVr*100h
	CLC
	RET	2
New21	ENDP

New27	PROC	FAR			; INT 27h
	ADD	DX,0Fh
	SHR	DX,1
	SHR	DX,1
	SHR	DX,1
	SHR	DX,1
	CALL	AddProg
	JNC	New27a
	MOV	AH,0
	JMP	Old21
New27a:	SHL	DX,1
	SHL	DX,1
	SHL	DX,1
	SHL	DX,1
	JMP	CS:Vect27
New27	ENDP

;------------- Save vectors & program name -------------
;			DS:DX - file name
ExeProg	PROC	NEAR C
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	AX
	CLD
	PUSH	CS
	POP	ES
	PUSH	DS			; Copy vectors & SAVE PTR
	XOR	SI,SI
	MOV	DS,SI
	MOV	DI,OFFSET IntTab
	MOV	CX,256*2
	CLI
	REP	MOVSW
	MOV	SI,4A8h
	MOVSW
	MOVSW
	STI
	POP	DS
	MOV	SI,DX			; Save file name
ExPrg1:	MOV	CX,SI
ExPrg2:	LODSB
	CMP	AL,':'
	JE	ExPrg1
	CMP	AL,'\'
	JE	ExPrg1
	CMP	AL,0
	JNE	ExPrg2
	MOV	SI,CX
	MOV	DI,OFFSET NameBuf
	MOV	CX,12
	REP	MOVSB
	POP	AX
	POP	CX
	POP	DI
	POP	SI
	POP	ES
	RET
ExeProg	ENDP

;------------- Add resident program to list ------------
;			   DX - number of paragraphs
AddProg	PROC	NEAR
	PUSH	DS
	PUSH	ES
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	BX
	PUSH	AX
	STI
	CMP	CS:Counter,MaxProg
	JB	AdPrg1
	MOV	AX,0E07h
	INT	10h
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET IntTab
	XOR	DI,DI
	MOV	ES,DI
	MOV	CX,2*256
	CLD
	CLI
	REP	MOVSW
	MOV	DI,4A8h
	MOVSW
	MOVSW
	STI
	STC
	JMP	AdPrg6
AdPrg1:	MOV	AH,62h			; Get current PSP
	INT	21h
	MOV	ES,BX
	CALL	FreeBlk
	MOV	BX,DX
	ADD	BX,(2+12+1+5*256+4+2 +15) SHR 4
	MOV	AH,4Ah
	INT	21h
	CMC
	JNC	AdPrg6
	MOV	AX,ES
	MOV	CX,ES
	ADD	CX,DX
	MOV	ES,CX
	PUSH	CS
	POP	DS
	CLD
	CLI
	XOR	DI,DI			; PSP -> PrgSeg
	STOSW
	MOV	SI,OFFSET NameBuf	; File name -> PrgSeg
	MOV	CX,12
	REP	MOVSB
	PUSH	DX			; Changed vectors -> PrgSeg
	PUSH	DI
	INC	DI
	XOR	BX,BX
	XOR	DX,DX
	MOV	DS,BX
	MOV	SI,OFFSET IntTab
AdPrg2:	LODS	CS:Wrd
	MOV	CX,AX
	LODS	CS:Wrd
	CMP	CX,DS:[BX].Offs
	JNE	AdPrg3
	CMP	AX,DS:[BX].Segm
	JE	AdPrg4
AdPrg3:	CMP	DL,22h
	JE	AdPrg4
	PUSH	AX			; Save vector
	MOV	AL,DL
	STOSB
	MOV	AX,CX
	STOSW
	POP	AX
	STOSW
	INC	DH
AdPrg4:	ADD	BX,4
	INC	DL
	JNE	AdPrg2
	POP	BX
	MOV	ES:[BX],DH
	PUSH	CS
	POP	DS
	POP	DX
	MOVSW				; SAVE PTR -> PrgSeg
	MOVSW
	MOV	CX,DI			; check sum -> PrgSeg
	XOR	SI,SI
	XOR	BX,BX
	MOV	AH,0
AdPrg5:	LODS	ES:Byt
	ADD	BX,AX
	LOOP	AdPrg5
	MOV	AX,BX
	STOSW
	MOV	BX,DS:Counter		; Save PrgSeg
	SHL	BX,1
	MOV	DS:PrgList[BX],ES
	INC	DS:Counter
	STI				; Add PrgSeg size
	DEC	DI
	MOV	CL,4
	SHR	DI,CL
	INC	DI
	ADD	DX,DI
AdPrg6:	POP	AX
	POP	BX
	POP	CX
	POP	DI
	POP	SI
	POP	BP
	POP	ES
	POP	DS
	RET
AddProg	ENDP

;------------------ Free memory block ------------------
;			   ES - segment for diallocation
FreeBlk	PROC	NEAR
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	AX
	CLD
	MOV	SI,OFFSET PrgList
	MOV	DI,SI
	MOV	CX,CS:Counter
	JCXZ	FreBl3
FreBl1:	MOV	DS,CS:[SI]
	CALL	ChkBlk
	LODS	CS:Wrd
	JNE	FreBl2
	PUSH	AX
	MOV	AX,ES
	CMP	AX,DS:[0]
	POP	AX
	JE	FreBl2
	MOV	CS:[DI],AX
	INC	DI
	INC	DI
FreBl2:	LOOP	FreBl1
	SUB	SI,DI
	SHR	SI,1
	SUB	CS:Counter,SI
FreBl3:	POP	AX
	POP	CX
	POP	DI
	POP	SI
	POP	DS
	RET
FreeBlk	ENDP

;---------------------- Check sum ----------------------
;			   DS - PrgSeg
ChkBlk	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	CX
	MOV	AL,DS:[0Eh]
	MOV	AH,5
	MUL	AH
	ADD	AX,WORD PTR (2+12+1+4)
	MOV	CX,AX
	XOR	SI,SI
	XOR	DI,DI
	MOV	AH,0
ChkBl1:	LODSB
	ADD	DI,AX
	LOOP	ChkBl1
	CMP	DI,DS:[SI]
	POP	CX
	POP	DI
	POP	SI
	RET
ChkBlk	ENDP

;------------------- Release programs ------------------
;			   DS - PrgSeg
Release	PROC	NEAR
	PUSH	DS
	PUSH	ES
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	CALL	GetStat
	CLD
	CALL	ChkBlk			; Check sum
	JNE	Relea5
	XOR	AX,AX			; Restore vectors
	MOV	ES,AX
	MOV	SI,0Eh
	LODSB
	MOV	CX,AX
	CLI
	JCXZ	Relea2
Relea1:	LODSB
	MOV	DI,AX
	SHL	DI,1
	SHL	DI,1
	MOVSW
	MOVSW
	LOOP	Relea1
Relea2:	MOV	DI,4A8h			; Restore SAVE PTR
	MOVSW
	MOVSW
	STI
	MOV	SI,DS:[0]		; Release blocks
	MOV	AH,52h
	INT	21h
	MOV	DS,ES:[BX-2]
Relea3:	CMP	SI,DS:[1]
	JNE	Relea4
	MOV	AX,DS
	INC	AX
	MOV	ES,AX
	MOV	AH,49h
	INT	21h
Relea4:	MOV	AX,DS
	INC	AX
	ADD	AX,DS:[3]
	CMP	BYTE PTR DS:[0],'M'
	MOV	DS,AX
	JE	Relea3
Relea5:	CALL	SetStat
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	ES
	POP	DS
	RETF
Release	ENDP

;---------------------- The entry ----------------------
; DOS has loaded the file as it is laid out: the transit part lies where
; it will run, behind the resident part and the paragraph of RGAP, and
; the stack is the resident part's from the start, by the header of the
; file (see tools/mkexe.c). A second copy, loaded to bring the transit
; part back after a command, moves it into the memory of the first.
; The check sum is that of the program but for the data of the resident
; part: the stack is there, and in use.
Alloc0:	CLD
	XOR	BX,BX			; Check sum of the whole program
	MOV	BP,DS
	ADD	BP,10h
	MOV	DX,SS:DatPrW
	SUB	DX,10h
	CALL	Alloc1
	MOV	BP,ES
	ADD	BP,TrnPar
	MOV	DX,ES:ImgEnd
	SUB	DX,TrnPar
	CALL	Alloc1
	PUSH	ES
	POP	DS
	JMP	AllocB
Alloc1:	MOV	AH,0			; DX paragraphs from BP on
AllocC:	MOV	DS,BP
	XOR	SI,SI
	MOV	CX,16
AllocA:	LODSB
	ADD	BX,AX
	LOOP	AllocA
	INC	BP
	DEC	DX
	JNZ	AllocC
	RET
AllocB:
	MOV	SI,80h
	MOV	DI,OFFSET OvlPrm
	MOV	CL,DS:[DI]
	DEC	CX
	REPE	CMPSB
	JNE	Alloc3
	LODSW
	CMP	BYTE PTR DS:[SI],0Dh
	JNE	Alloc3
	MOV	ES,AX
	XCHG	BX,ES:ComnIni.LoadSiz
	MOV	BP,ES:RunPar
	ADD	BP,TrnPar
	MOV	DX,BX
	SUB	DX,TrnPar
	PUSH	BX
	MOV	BX,DS
	ADD	BX,TrnPar
	MovPars
	POP	BX
	MOV	AX,4C00h
	SUB	BX,CS:ImgEnd
	JE	Alloc2
	INC	AX
Alloc2:	INT	21h
Alloc3:	MOV	DS:ChkFile,BX
	MOV	AH,30h			; Check DOS version
	INT	21h
	CMP	AL,3
	JB	Alloc4
	XCHG	AL,AH
	CMP	AX,314h
	JAE	Alloc5
	MOV	SI,OFFSET Vers_S
	PUSH	CS
	CALL	OutStr0
	MOV	AX,4C02h
	INT	21h
Alloc4:	INT	20h
Alloc5:	MOV	BX,ResPar		; Allocate segments
	MOV	DX,DS:ImgEnd
	SUB	DX,TrnPar
	ADD	BX,DX
	INC	BX
	MOV	AH,4Ah
	INT	21h
	JNC	Alloc6
	MOV	BX,DX			; Allocate transit block
	MOV	AH,48h
	INT	21h
	JC	Alloc9
	MOV	BP,AX
	MOV	BX,DS
	ADD	BX,TrnPar
	MovPars
	CLI
	PUSH	CS
	POP	SS
	MOV	SP,OFFSET VcStack
	STI
	PUSH	CS
	POP	DS
	PUSH	CS			; Compress resident block
	POP	ES
	MOV	BX,ResPar
	MOV	AH,4Ah
	INT	21h
	MOV	ES,BP
	JMP	Alloc7
Alloc6:	MOV	BP,DS
	ADD	BP,ResPar
	INC	BP
	MOV	BX,DS
	ADD	BX,TrnPar
	CLI
	PUSH	CS
	POP	SS
	MOV	SP,OFFSET VcStack
	MovPars
	MOV	ES,BP
	MOV	BX,ResPar

;-------- Split into resident and transit parts --------
	MOV	AX,ES
	DEC	AX
	PUSH	AX
	MOV	AX,CS
	DEC	AX
	MOV	DS,AX
	MOV	AL,'M'
	XCHG	AL,DS:[0]
	XCHG	BX,DS:[3]
	SUB	BX,DS:[3]
	DEC	BX
	POP	DS
	MOV	DS:[0],AL
	MOV	DS:[1],CS
	MOV	DS:[3],BX
	STI
	PUSH	CS
	POP	DS
Alloc7:	XOR	BP,BP
	MOV	AX,ES
	SUB	AX,TrnPar
	MOV	DS:RunPar,AX
	PUSH	AX
	CALL	SetStat
	MOV	AX,OFFSET Init00
	PUSH	AX
	RETF

Alloc9:	MOV	SI,OFFSET Aloc_S	; Error
	PUSH	CS
	CALL	OutStr0
	CALL	SetStat
	MOV	AX,4C01h
	INT	21h

RHEAD	ENDS

;--------------------- Release data --------------------
RDATA	SEGMENT
IntTab	DD	256	DUP(?)
SavePtr	DD	?
Vect21	DD	?
Vect22	DD	?
Vect27	DD	?
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
	.CONST
Title_S	DB	0FFh,14,'The ',PrgName,', Version ',VcVers,0
Suff_S	DB	' ',VcSuff,0
Copyr_S	DB	13,10, 'Copyright (C) 1991-2000 Vsevolod V. Volkov',0
CrLf_S	DB	13,10, '$'
Retrn_S	DB	13,10, 'Press ENTER to return ', 0
	DB	13,10, 'to the ',PrgName, 0
NoIni_S	DB	    'VC.INI is not correct.'
	DB	13,10, 0
Vers_S	DB	13,10, 'This program requires DOS 3.20 or later.'
	DB	13,10, 0
Comsp	DB	'COMSPEC='
NcEnv	DB	'VC='
TempEnv	DB	'TEMP='

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

Palets	DB	07h,07h,1Bh,07h,07h,07h	;  0 - Main windows
	DB	70h,70h,30h,70h,70h,70h	;  1 - Cursor
	DB	0Fh,0Fh,1Eh,0Fh,0Fh,0Fh	;  2 - Selected files
	DB	7Fh,09h,3Eh,09h,70h,70h	;  3 - Cursor for selected files
	DB	0Fh,0Fh,1Eh,0Fh,0Fh,0Fh	;  4 - "Name   Size ..."
	DB	07h,07h,07h,07h,07h,07h	;  5 - Key bar numbers
	DB	70h,70h,30h,70h,70h,70h	;  6 - Key bar titles
	DB	70h,70h,30h,70h,70h,70h	;  7 - Menu bar
	DB	0Fh,0Fh,0Fh,0Fh,0Fh,0Fh	;  8 - Menu bar cursor
	DB	70h,70h,30h,70h,70h,70h	;  9 - Menu box border
	DB	70h,70h,3Fh,70h,70h,70h	; 10 - Menu box text
	DB	7Fh,70h,3Eh,70h,70h,70h	; 11 - Menu box high letters
	DB	07h,0Fh,0Fh,0Fh,0Fh,0Fh	; 12 - Menu box text cursor
	DB	0Fh,0Fh,0Eh,0Fh,0Fh,0Fh	; 13 - Menu box high letters cursor
	DB	70h,70h,30h,70h,70h,70h	; 14 - Menu box dark lines
	DB	70h,70h,33h,70h,70h,70h	; 15 - Menu box dark lines '-'
	DB	07h,07h,70h,07h,07h,07h	; 16 - Function box
	DB	0Fh,0Fh,7Eh,0Fh,0Fh,0Fh	; 17 - Function box bright
	DB	70h,70h,30h,70h,70h,70h	; 18 - Function box cursor
	DB	07h,07h,3Fh,07h,07h,07h	; 19 - Configuration window
	DB	70h,70h,0Fh,70h,70h,70h	; 20 - Configuration cursor
	DB	0Fh,0Fh,3Eh,0Fh,0Fh,0Fh	; 21 - Configuration window bright
	DB	7Fh,09h,0Eh,09h,70h,70h	; 22 - Configuration cursor bright
	DB	70h,70h,3Fh,70h,70h,70h	; 23 - History window
	DB	07h,07h,0Fh,07h,07h,07h	; 24 - History cursor
	DB	07h,07h,13h,07h,07h,07h	; 25 - Menu help border
	DB	70h,70h,30h,70h,70h,70h	; 26 - Menu help window
	DB	07h,07h,30h,07h,07h,07h	; 27 - Help window
	DB	0Fh,0Fh,3Fh,0Fh,0Fh,0Fh	; 28 - Help window bold
	DB	70h,70h,0Fh,70h,70h,70h	; 29 - Help reversive
	DB	0Fh,09h,3Eh,09h,0Fh,0Fh	; 30 - Help underline
	DB	07h,07h,4Fh,07h,07h,07h	; 31 - Error windows
	DB	70h,70h,70h,70h,70h,70h	; 32 - Error window cursor
	DB	0Fh,0Fh,4Eh,0Fh,0Fh,0Fh	; 33 - Error window bright
	DB	70h,70h,30h,70h,70h,70h	; 34 - Clock
	DB	0Fh,0Fh,1Eh,0Fh,0Fh,0Fh	; 35 - Arrows in view/edit
	DB	07h,07h,0Bh,07h,07h,07h	; 36 - Screen blank
	DB	0Fh,0Fh,0Fh,0Fh,0Fh,0Fh	; 37 - Screen blank light stars
	DB	02h,02h,01h,02h,00h,00h	; 38 - Shadow flag
	DB	02h,02h,02h,02h,02h,02h	; 39 - Blink / High background / None
DefInit	DB	1			; FulScr
	DB	0			; MenuVs
	DB	1			; KeyBar
	DB	1			; Prompt
	DB	1			; InsDwn
	DB	0			; AutoMnu
	DB	1			; MiniSta
	DB	0			; Clock
	DB	1			; AutoCD
	DB	1			; AutoSav
	DB	1			; ColTyp
	DB	5			; BlDelay
	DB	1			; LoadFl
	DB	1			; UnSlct
	DB	1			; ErrSnd
	DB	0			; LftMous
	DB	1			; ZoomWin
	DB	1			; AltBar
	DB	1			; EscBar
	DB	0			; ExecTyp
	DB	0			; AutoSiz
	DB	1			; HidDirs
	DB	1			; QuitAck
	DB	2			; SnowMod
	DB	1			; AltView
	DB	1			; AltEdit
	DB	1			; ClrBuf
	DB	1			; Active
	DB	0			; Ctrl_O
	DB	1			; ChkTSR
	DB	1			; ChgTree
	DB	1			; DriveB
	DB	MiliSec, Sec		; DelayTm
	DB	'ъщ'			; StarTab
DefWCB	DB	0			; WinTyp
	DB	0			; BrfFul
	DB	1			; Visible
	DB	1			; Hidden
	DB	0			; MasTyp
	DB	8			; SortTyp
	DB	0			; Ctrl_L
	DB	0			; DskMas

;--------------------- Main module ---------------------
	.CODE

;------------------ Read configuration -----------------
Init00:	CALL	Reloc
	MOV	ES,DS:[2Ch]
	MOV	DI,OFFSET WCB1sav + WCBsave
	MOV	DX,DI
	CALL	MainPth
	PUSH	DI
	PUSH	DX
	MOV	SI,OFFSET Init_F
	MOV	BP,OFFSET ComnIni - (3 + 2*WCBini)
	MOV	CX, 3 + 2*WCBini + CmnIni + 2 + 1
	CALL	ReadOne
	MOV	BX,0
	JC	Init03
	DEC	CX
	SUB	AX,CX
	JNE	Init02
	DEC	CX
	DEC	CX
	MOV	SI,BP
	XOR	DX,DX
Init01:	LODSB
	ADD	DX,AX
	LOOP	Init01
	LODSW
	CMP	AX,DX
	JNE	Init02
	MOV	DI,BP
	MOV	AL,'V'
	MOV	CL,3
	REPE	SCASB
	JNE	Init02
	MOV	SI,DI
	MOV	DI,OFFSET WCB1sav
	MOV	CL,WCBini
	PUSH	CX
	REP	MOVSB
	POP	CX
	MOV	DI,OFFSET WCB2sav
	REP	MOVSB
	JMP	Init04
Init02:	MOV	BL,1

;--------------------- Default init --------------------
Init03:	PUSH	DS
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET Palets
	MOV	DI,OFFSET ComnIni
	MOV	CX,OFFSET ResIni
	REP	MOVSB
	MOV	CL,CmnIni-ResIni
	MOV	AL,0
	REP	STOSB
	MOV	SI,OFFSET DefWCB
	MOV	DI,OFFSET WCB1sav
	MOV	CL,WCBini
	REP	MOVSB
	POP	DS
	MOV	AH,19h
	INT	21h
	MOV	DL,AL
	INC	DX
	ADD	AL,'A'
	MOV	DI,OFFSET WCB1sav.WinPath
	STOSB
	MOV	AX,'\:'
	STOSW
	MOV	SI,DI
	MOV	AL,0
	STOSB
	MOV	AH,47h
	INT	21h
	MOV	SI,OFFSET WCB1sav
	MOV	DI,OFFSET WCB2sav
	MOV	CX,WCBini
	REP	MOVSB
	MOV	DS:WCB1sav.Visible,CL

;--------------------- Output Report -------------------
Init04:	CALL	ChkMode
	JZ	Init05
	MOV	AX,3
	INT	10h
Init05:
	PUSH	CS			; Output message
	POP	DS
	MOV	DX,OFFSET ComnIni.ExtMain
	MOV	DI,DX
	MOV	SI,OFFSET Title_S+2
	CALL	MvzLin0
	MOV	SI,OFFSET Suff_S
	CALL	MvzLin0
	MOV	SI,OFFSET Copyr_S
	CALL	MvzLin0
	MOV	SI,OFFSET CrLf_S
	CALL	MvzLin0
	PUSH	ES
	POP	DS
	MOV	AH,9
	INT	21h
	OR	BL,BL			; Error in INI file
	JE	Init09
	MOV	SI,OFFSET NoIni_S
	CALL	OutStr1

;--------------- Read main extension file --------------
Init09:	POP	DX
	POP	DI
	MOV	SI,OFFSET Ext_F
	MOV	BP,OFFSET ComnIni.ExtMain
	MOV	CX,LenExt-1
	CALL	ReadOne
	JC	Init10
	ADD	BP,AX
Init10:	MOV	BYTE PTR DS:[BP],1Ah

;---------------- Initialize main data -----------------
	MOV	BX,OFFSET ComnIni
	MOV	SI,OFFSET WCB1sav
	MOV	DI,OFFSET WCB2sav
	MOV	AX,OFFSET DskBuf
	MOV	DS:[SI].CurAdr,AX
	MOV	DS:[DI].CurAdr,AX
	MOV	DS:[SI].First,AX
	MOV	DS:[DI].First,AX
	MOV	DS:[SI].EndBuf,AX
	MOV	DS:[DI].EndBuf,AX
	XOR	AX,AX
	MOV	DS:Counter,AX
	MOV	DS:[BX].CrnPath,AL
	MOV	DS:[BX].ViewFil,AL
	MOV	DS:[BX].ToFile,AL
	MOV	DS:[BX].SrchStr,AL
	MOV	DS:[BX].HstBuf.Wrd,AX
	MOV	DS:[BX].LastMnu,1
	DEC	AX
	MOV	DS:[BX].MouseX,AX
	MOV	DS:[BX].MouseY,AX
	MOV	DS:[BX].LoadRes,AL
	MOV	AL,DS:[BX].AutoMnu
	MOV	DS:[BX].MenuFl,AL
	PUSH	DS
	PUSH	CS
	POP	DS
	MOV	SI,OFFSET Mask_S+1
	LEA	DI,[BX].SelMas
	MOV	CX,4
	REP	MOVSB
	MOV	SI,OFFSET Mask_S
	LEA	DI,[BX].FindStr
	MOV	CL,5
	REP	MOVSB
	POP	DS
	MOV	AX,OFFSET SetBrk
	CALL	CallPSP
	MOV	AX,OFFSET Exit01
	XCHG	AX,DS:[0Ah].Offs
	MOV	DS:Vect22.Offs,AX
	MOV	AX,DS
	XCHG	AX,DS:[0Ah].Segm
	MOV	DS:Vect22.Segm,AX
	CMP	DS:[BX].ChkTSR,0
	JZ	Init11
	MOV	AX,3521h
	INT	21h
	MOV	DS:Vect21.Offs,BX
	MOV	DS:Vect21.Segm,ES
	XOR	AX,AX
	MOV	ES,AX
	CLI
	MOV	ES:[4*21h].Offs,OFFSET New21
	MOV	ES:[4*21h].Segm,DS
	STI
	MOV	AX,3527h
	INT	21h
	MOV	DS:Vect27.Offs,BX
	MOV	DS:Vect27.Segm,ES
	MOV	DX,OFFSET New27
	MOV	AH,25h
	INT	21h

;------------ Allocate Secondary Data Segment ----------
Init11:	CALL	Reloc
	MOV	AX,CS
	ADD	AX,TrnPar
	MOV	ES,AX
	MOV	BX,0FFFFh
	MOV	AH,4Ah
	INT	21h
	MOV	SI,BX
	CMP	AX,WORD PTR 8
	JE	Init12
	XOR	SI,SI
Init12:	MOV	BX,SS:RelTab		; The relocation table has been used
	SUB	BX,TrnPar
	MOV	AH,4Ah
	INT	21h
	PUSH	DS
	POP	ES
	MOV	BX,0FFFFh
	MOV	AH,48h
	INT	21h
	MOV	AH,48h
	INT	21h
	MOV	ES,AX
	MOV	DX,((SIZE MainDat-1) SHR 4)+1
	MOV	DI,((SIZE WCBdef -1) SHR 4)+1
	MOV	AX,((   MinView  -1) SHR 4)+1
	ADD	AX,DX
	ADD	AX,DI
	ADD	AX,DI
	CMP	AX,BX
	JBE	Init13
	MOV	AX,OFFSET AlocEr
	PUSH	SS
	PUSH	AX
	RETF
Init13:	MOV	ES:FreMem2,BX
	CMP	SI,BX
	JAE	Init14
	MOV	SI,BX
Init14:	MOV	ES:FreMem1,SI
	MOV	AX,ES
	ADD	AX,DX
	MOV	ES:WCB1seg,AX
	ADD	AX,DI
	MOV	ES:WCB2seg,AX
	ADD	AX,DI
	MOV	ES:ViewSeg,AX
	MOV	CS:SecDat,ES

;------------------ Restore data segment ---------------
	CLD
	XOR	DI,DI
	MOV	SI,OFFSET ComnIni
	MOV	CX,CmnSave
	REP	MOVSB
	PUSH	ES
	MOV	ES,ES:WCB1seg
	MOV	SI,OFFSET WCB1sav
	CALL	Expand
	POP	ES
	PUSH	ES
	MOV	ES,ES:WCB2seg
	MOV	SI,OFFSET WCB2sav
	CALL	Expand
	POP	DS

;------------------- Set Error Vectors -----------------
	CALL	HookVct
	PUSH	ES
	POP	DS

;------------------ Set Display Segment ----------------
	CALL	ChkMode
	JZ	Init17
	MOV	BL,AH
	MOV	CX,2000h
	MOV	AH,1
	INT	10h
	PUSH	DS
	MOV	SI,OFFSET Retrn_S
	CALL	OutStr1
	INC	SI
	CMP	BL,80
	JB	Init15
	INC	SI
	INC	SI
Init15:	CALL	OutStr1
	POP	DS
Init16:	MOV	AH,0
	INT	16h
	CMP	AL,0Dh
	JNE	Init16
	MOV	AL,DS:ScrMod
	MOV	AH,0
	INT	10h
	MOV	AH,0Fh
	INT	10h
	AND	AL,7Fh
Init17:	MOV	DS:ScrMod,AL
	MOV	DS:ScrPage,BH

;------------------ Correct cursor size ----------------
	MOV	AH,3
	INT	10h
	CMP	CH,0Fh
	JBE	Init18
	MOV	CX,607h
Init18:	CMP	CX,67h
	JNE	Init19
	MOV	CX,607h
Init19:	CMP	DS:ScrMod,7
	JNE	Init20
	CMP	CX,607h
	JNE	Init20
	MOV	CX,0B0Ch
Init20:	MOV	AH,1
	INT	10h

;---------------- Initialize data segment --------------
IniSave	EQU	WCB1sav

	MOV	DX,OFFSET Country	; Country
	MOV	AX,3800h
	INT	21h
	MOV	DI,OFFSET TmpBuf	; Lower case table
	MOV	SI,OFFSET LowTab
	MOV	CX,80h
	PUSH	DI
	PUSH	CX
Init21:	MOV	AL,CL
	NEG	AL
	MOV	DS:[SI],AL
	INC	SI
	CALL	DS:Country.CaseMap
	STOSB
	LOOP	Init21
	POP	CX
	POP	SI
	MOV	BH,0
	MOV	AH,80h
Init22:	LODSB
	CMP	AL,AH
	JE	Init23
	CMP	AL,80h
	JB	Init23
	MOV	BL,AL
	MOV	DS:LowTab[BX-128],AH
Init23:	INC	AH
	LOOP	Init22
	AND	DS:LoadFl,7Fh
	MOV	AX,SS:[2]		; Memorys
	MOV	DS:Memorys,AX
	XOR	AX,AX
	MOV	SS:IniSave,AL
	MOV	DS:DosBuf,AL		; DosBuf
	MOV	BX,OFFSET EPB		; EPB
	MOV	DS:[BX].EnvSeg,AX
	MOV	DS:[BX].FCB1.Offs,5Ch
	MOV	DS:[BX].FCB1.Segm,ES
	MOV	DS:[BX].FCB2.Offs,6Ch
	MOV	DS:[BX].FCB2.Segm,ES
	MOV	ES,SS:[2Ch]		; ComSpec
	PUSH	DS
	PUSH	CS
	POP	DS
	XOR	DI,DI
Init24:	CMP	AL,ES:[DI]
	JE	Init25
	MOV	SI,OFFSET Comsp
	MOV	CX,8
	REPE	CMPSB
	JE	Init26
	DEC	DI
	MOV	CH,0FFh
	REPNE	SCASB
	JMP	Init24
Init25:	XOR	DI,DI
Init26:	POP	DS
	MOV	DS:ComSpec,DI
	XOR	DI,DI			; OvlPth
Init27:	CMP	AL,ES:[DI]
	JE	Init28
	MOV	CH,0FFh
	REPNE	SCASB
	JMP	Init27
Init28:	ADD	DI,3
	MOV	DS:OvlPth,DI
	MOV	DI,OFFSET NcPath	; NcPath
	PUSH	ES
	PUSH	DI
	CALL	MainPth
	POP	SI
	CALL	PathLin
	POP	ES
	MOV	DI,OFFSET TempDir	; TempDir
	MOV	SI,OFFSET TempEnv
	MOV	CX,5
	CALL	MnPth0
	XOR	AX,AX
	MOV	DS:DosCol,AL		; DosCol
	MOV	DS:BegCol,AL		; BegCol
	MOV	DS:OutCol,AL		; OutCol
	MOV	DS:Quote,AL		; Quote
	MOV	DS:SftErr,AL		; SftErr
	MOV	DS:BlocEr,AL		; BlocEr
	MOV	DS:TstEGA,AL		; TstEGA
	MOV	DS:WinMous,AL		; WinMous
	MOV	DS:WinLeav,AL		; WinLeav
	MOV	DS:HstAdr,OFFSET HstBuf	; HstAdr
	MOV	DS:BarPrev,0FFh		; BarPrev
	MOV	AH,19h			; DosChg
	INT	21h
	MOV	DS:DosChg[0],AL
	MOV	AH,12h			; TstEGA
	MOV	BX,0FF10h
	INT	10h
	CMP	BX,0FF10h
	JE	Init29
	MOV	DS:TstEGA,1
Init29:	MOV	AL,25			; Lines
	MOV	DL,0
	CMP	DS:TstEGA,DL
	JE	Init30
	PUSH	ES
	PUSH	AX
	MOV	BH,0
	MOV	AX,1130h
	INT	10h
	POP	AX
	POP	ES
	OR	DL,DL
	JE	Init30
	INC	DX
	MOV	AL,MaxLins
	CMP	AL,DL
	JBE	Init30
	MOV	AL,DL
Init30:	MOV	DS:Lines,AL
	MOV	AH,1			; Snow
	CMP	DS:ScrMod,7
	JE	Init33
	CMP	DS:SnowMod,1
	JE	Init33
	MOV	DX,03DAh
	MOV	BX,8000h
	JB	Init31
	CMP	DS:TstEGA,BL
	JNE	Init33
Init31:	STI
	DEC	BX
	JE	Init33
	MOV	CX,7			; Const2_1 ???
	CLI
Init32:	IN	AL,DX
	TEST	AL,1
	JZ	Init31
	LOOP	Init32
	MOV	AH,0
Init33:	STI
	MOV	DS:Snow,AH
	CALL	ScrOffs			; Screen Offset
	MOV	BX,0B800h		; Screen Segment
	CMP	DS:ScrMod,7
	JNE	Init34
	MOV	BH,0B0h
Init34:	MOV	ES,BX			; Virtual screen buffer
	MOV	AH,0FEh
	INT	10h
	MOV	CX,ES
	PUSH	DS
	POP	ES
	MOV	AL,0
	CMP	CX,BX
	JE	Init35
	MOV	AL,1
	MOV	DS:Snow,AL
Init35:	MOV	DS:Screen.Segm,CX
	MOV	DS:TopView,AL
	MOV	CX,'DE'			; Check DESQWIEV
	MOV	DX,'SQ'
	MOV	AX,2B01h
	INT	21h
	INC	AX
	MOV	DS:DesqVie,AL
	XOR	BX,BX			; VerDOS
	MOV	AX,3306h
	INT	21h
	XCHG	AX,BX
	OR	AX,AX
	JNE	Init36
	MOV	AH,30h
	INT	21h
Init36:	CMP	AX,WORD PTR 4
	JNE	Init37
	CMP	BH,0FFh
	JNE	Init37
	MOV	AH,1
Init37:	MOV	DS:VerDOS,AX
	MOV	SI,DS:WCB1seg		; WCBseg
	MOV	DI,DS:WCB2seg
	PUSH	DS
	MOV	DS,SI
	MOV	DS:ChgFlg,1
	MOV	DS,DI
	MOV	DS:ChgFlg,1
	POP	DS
	CMP	DS:Active,0
	JE	Init38
	XCHG	DI,SI
Init38:	MOV	DS:WCBcrn,SI
	MOV	DS:WCBsec,DI

;---------------------- Set DOS color ------------------
	CALL	CurNl
	MOV	AX,0E0Dh
	INT	10h
	MOV	AX,0E0Ah
	INT	10h
	CALL	CurNl
	MOV	AH,8
	INT	10h
	PUSH	AX
	MOV	DL,' '
	MOV	AH,6
	INT	21h
	MOV	AX,0E0Dh
	INT	10h
	MOV	AH,8
	INT	10h
	MOV	DS:CrntCol,AH
	POP	AX
	MOV	BL,AH
	MOV	CX,1
	MOV	AH,9
	INT	10h

;----------------------- Set cursor --------------------
; From here on the loop is C: MainLoop_c of port/main.c, which is entered
; at one of several places and returns where the program goes on. See
; MainGo below.
Init40:	MainC	0

;---------------- Relocate the transit part -------------
; The program moves and reloads its transit part itself, so DOS cannot
; relocate it. The linker's relocation table is appended to the program by
; tools/mkexe.c and applied here, once to every copy that is loaded.
Reloc	PROC	NEAR
	PUSHF
	CMP	CS:RelDone,0
	JNE	Reloc3
	INC	CS:RelDone
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	SI
	PUSH	DI
	PUSH	DS
	PUSH	ES
	CLD
	MOV	BX,CS
	MOV	AX,BX
	ADD	AX,SS:RelTab
	MOV	DS,AX
	XOR	SI,SI
	MOV	CX,SS:RelCnt
	JCXZ	Reloc2
Reloc1:	LODSW
	MOV	DI,AX
	LODSW
	ADD	AX,BX
	MOV	ES,AX
	ADD	ES:[DI],BX
	LOOP	Reloc1
Reloc2:	POP	ES
	POP	DS
	POP	DI
	POP	SI
	POP	CX
	POP	BX
	POP	AX
Reloc3:	POPF
	RET
Reloc	ENDP

CurNl	PROC	NEAR
	MOV	AH,0Fh
	INT	10h
	MOV	AH,3
	INT	10h
	MOV	AH,DS:Lines
	CMP	DH,AH
	JB	CurNl1
	DEC	AH
	MOV	AL,0
	JMP	Cursor
CurNl1:	RET
CurNl	ENDP

OutStr1	PROC	NEAR
	PUSH	CS
	POP	DS
	MOV	AX,OFFSET OutStr
	JMP	CallPSP
OutStr1	ENDP

;-------------------------------------------------------
; The main loop is C: MainLoop_c of port/main.c. It is no routine: the
; code above falls into it, commands jump back into it at the labels
; here, and it jumps to a command. So each label hands the function the
; number of the place, and all the registers as a structure on the stack
; (struct Regs there): what a command leaves in them goes round the loop
; to the next command. The function returns in AX where the program goes
; on, a number of Main_T, and has made of the registers what that place
; gets, with the current panel for DS. MainC, at the top of this file,
; pushes the registers.
Init50:	MainC	1			; Draw the panels, reading them
Func01:	MainC	2			; Draw the panel in DS
Func02:	MainC	3			; The screen buffer to the screen
Func04:	MainC	4			; The menu bar
Func05:	MainC	5			; The setup saved if it has changed
Func07:	MainC	6			; The next key
Func12:	MainC	7			; The command of the key in AX

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

; Where the program goes on: the command line, the places that the mouse
; goes to, and from F00_00 on the other commands of the keys, in the
; order that the table of the keys had them in, which Func12 of
; port/main.c knows them by.
	.DATA
Main_T	DW	F73_00, F77_00, F11_00, F39_00, F20_00, F34_10
	DW	F00_60, F00_05, F01_05, F21_10, F21_31
	DW	F00_00, F01_00, F02_00, F03_00, F04_00, F05_00, F06_00
	DW	F07_00, F08_00, F09_00, F10_00, F76_00, F21_00, F27_00
	DW	F74_00, F78_00, F12_00, F13_00, F14_00, F15_00, F16_00
	DW	F17_00, M16_00, M16_10, M30_00, M49_00, F18_00, F79_00
	DW	F22_00, F23_00, F24_00, F25_00, F26_00, F80_00, F81_00
	DW	F82_00, F30_00, F32_00, F33_00, F34_00, F35_00, F36_00
	DW	F37_00, F38_00, F40_00, F42_00, F43_00, F44_00, F45_00
	DW	F46_00, F47_00, F48_00, M75_00, F54_00, F55_00, F56_00
	DW	F58_00, F61_00, F62_00, F63_00, F64_00, F65_00, M42_00
	DW	F67_00, F68_00, F69_00, M41_00, F75_00

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

	INCLUDE	VCFUNC.INC
	INCLUDE	VCMENU.INC
	INCLUDE	VCF1.INC
	INCLUDE	VCCOPY.INC
	INCLUDE	VCVIEW.INC
	INCLUDE	VCEDIT.INC
	INCLUDE	VCHELP.INC
	INCLUDE	VCMEM.INC
	INCLUDE	VCFIND.INC

;------------------------ Exit -------------------------
; C: Exit_c of port/main.c puts the screen back. The part of the program
; that stays in the memory ends it.
Exit:	CallC	Exit_c
	MOV	AX,OFFSET Exit00
	PUSH	SS
	PUSH	AX
	MOV	AL,0
	RETF

;			   DS - main data
;	Return :	   ES - main data
HookVct	PROC	NEAR
	CALL	FAR PTR HookVct_c
	PUSH	DS
	POP	ES
	RET
HookVct	ENDP

;--------------------- Fatal error ---------------------
	.CONST
Err_S	DB	' Error ',0
OnDr_S	DB	'on drive A',0
OnDv_S	DB	'on device '
Errs_S	DB	'Attempt to write on write-protected disk.',0
	DB	'Unknown disk drive.',0
	DB	'Drive or device not ready.',0
	DB	'Unknown command.',0
	DB	'Data error.',0
	DB	'Bad request.',0
	DB	'Seek error.',0
	DB	'Non-DOS disk.',0
	DB	'Sector not found.',0
	DB	'Printer out of paper.',0
	DB	'Write fault.',0
	DB	'Read fault.',0
	DB	'General failure.',0
	DB	'Sharing violation.',0
	DB	'Lock violation.',0
	DB	'Invalid disk change.',0
	DB	'FCB unavailable.',0
	DB	'Sharing buffer overflow.',0
	DB	'Code page mismatch.',0
	DB	'Out of input.',0
	DB	'Insufficient disk space.',0
	DB	'Unknown error.',0
FatEr_S	DB	'File allocation table bad.',0
RtAb_S	DB	' Retry   Abort ',0
Ok_S	DB	' Ok ',0
Var_S	DB	'^V',0
Inc_S	DB	'^I',0

	.DATA
RtAb_D	DB	7, 0
	DB	7, 8
	DB	0
FtEr1_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf1
	DW	TmpErr+1
	DB	0, 70
	DW	Err_S
	DW	Var_S
	DW	Inc_S
	DW	RtAb_S
FtEr2_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 1
	DW	ErrBuf1
	DW	TmpErr+1
	DB	0, 70
	DW	Err_S
	DW	Var_S
	DW	Inc_S
	DW	Ok_S
FtEr1_B	BOXHD	< FtEr1_P, 928h, 0, 2, 0FFh, 0FFh, 7>
	BOXMN	< 4, 0,0,0,0, 420h, TmpErr, RtAb_D>
	DB	0
FtEr2_B	BOXHD	< FtEr2_P, 928h, 0, 2, 0FFh, 0FFh, 7>
	BOXMN	< 4, 0,0,0,0, 426h, TmpErr, OkMn_D>
	DB	0
OkMn_D	DB	4, 0
	DB	0
SecDat	DW	0
RelDone	DB	0			; 1 when this copy has been relocated

	.CODE
FatErr:	PUSH	DS
	PUSH	ES
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	CLD
	MOV	DS,CS:SecDat
	PUSH	DS
	POP	ES
	MOV	BL,DS:ErrDat
	OR	BL,DS:BlocEr
	JZ	FtErr1
	MOV	AL,3
	JMP	FtErr9
FtErr1:	PUSH	AX
	PUSH	ES
	PUSH	DI
	MOV	DS,BP
	MOV	BX,SI
	TEST	BYTE PTR DS:[BX+5],80h
	PUSH	CS
	POP	DS
	MOV	DI,OFFSET TmpErr+1
	MOV	SI,OFFSET Err_S+1
	MOV	CX,6
	REP	MOVSB
	JNZ	FtErr2
	MOV	SI,OFFSET OnDr_S
	MOV	CL,11
	REP	MOVSB
	ADD	ES:[DI-2],AL
	TEST	AH,80h
	JZ	FtErr3
	POP	DI
	MOV	DI,OFFSET FatEr_S
	XOR	AX,AX
	JMP	FtErr5
FtErr2:	MOV	SI,OFFSET OnDv_S
	MOV	CL,10
	REP	MOVSB
	MOV	DS,BP
	LEA	SI,[BX+10]
	MOV	CL,8
	PUSH	DI
	PUSH	CX
	REP	MOVSB
	MOV	AL,' '
	STOSB
	POP	CX
	POP	DI
	INC	CX
	REPNE	SCASB
	DEC	DI
	MOV	AX,':'
	STOSW
FtErr3:	PUSH	CS
	POP	ES
	POP	CX
	XOR	AX,AX
	MOV	DI,OFFSET Errs_S
	JCXZ	FtErr5
	CMP	CX,15h
	JB	FtErr4
	MOV	CX,15h
FtErr4:	PUSH	CX
	MOV	CL,80
	REPNE	SCASB
	POP	CX
	LOOP	FtErr4
FtErr5:	POP	ES
	POP	CX
	PUSH	ES
	POP	DS
	XOR	BX,BX
	XCHG	BL,DS:Clock
	XCHG	BH,DS:BlDelay
	MOV	CL,DS:HlpPage
	DEC	AX
	MOV	DS:AutoTim.Lo,AX
	MOV	BP,OFFSET FtEr1_B
	TEST	CH,10h
	JNZ	FtErr6
	MOV	BP,OFFSET FtEr2_B
FtErr6:	MOV	DX,DI
	CALL	Dialog
	TEST	CH,10h
	JZ	FtErr7
	CMP	AL,0
	MOV	AL,1
	JE	FtErr8
FtErr7:	MOV	AL,3
FtErr8:	MOV	DS:Clock,BL
	MOV	DS:BlDelay,BH
	MOV	DS:HlpPage,CL
	CMP	AL,1
	JE	FtErr9
	MOV	DS:ErrDat,1
FtErr9:	CBW
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	BP
	POP	ES
	POP	DS
	IRET

;-------------------- Not ready error ------------------
;			   AL - Drive letter
;	Return:		   AL - Drive letter
	.CONST
NoRd1_S	DB	"Can't read the disk in drive ^V:",0
NoRd2_S	DB	"Press ENTER to try again, ESC to abort,",0
NoRd3_S	DB	"or enter a different drive letter here ^r^V^r:",0
	.DATA
NoRd_P	DB	Err_C, ErInv_C, 0, 0
	DB	4, 1
	DW	ErrBuf
	DW	TmpErr+1
	DB	0, 55
	DW	Err_S
	DW	NoRd1_S
	DW	NoRd2_S
	DW	NoRd3_S

	.CODE
; C: ErrRdy_c of port/main.c, which only C code calls. Its box stays
; here, where DialBox reads it.

MainPth	PROC	NEAR
	MOV	SI,OFFSET NcEnv
	MOV	CX,3
MnPth0:	PUSH	DS
	PUSH	DI
	PUSH	CS
	POP	DS
	XOR	DI,DI
	MOV	AL,0
MnPth1:	CMP	AL,ES:[DI]
	JE	MnPth2
	PUSH	DI
	PUSH	SI
	PUSH	CX
	REPE	CMPSB
	POP	CX
	POP	SI
	POP	DI
	JE	MnPth3
	PUSH	CX
	MOV	CH,0FFh
	REPNE	SCASB
	POP	CX
	JMP	MnPth1
MnPth2:	LEA	SI,[DI+3]
	PUSH	ES
	POP	DS
	POP	DI
	POP	ES
	CMP	DI,OFFSET TempDir
	JNE	MnPth9
	PUSH	ES
	POP	DS
	MOV	SI,OFFSET NcPath
MnPth9:	MOV	CX,LenPath
	PUSH	DI
	PUSH	CX
	REP	MOVSB
	POP	CX
	POP	SI
	PUSH	ES
	POP	DS
	CALL	CutPath
	MOV	DI,SI
	JMP	MnPth4
MnPth3:	MOV	SI,DI
	ADD	SI,CX
	PUSH	ES
	POP	DS
	POP	DI
	POP	ES
	MOV	CX,LenPath-2
	CALL	MvzLine
	CALL	AddFile
MnPth4:	PUSH	ES
	POP	DS
	MOV	BYTE PTR DS:[DI],0
	RET
MainPth	ENDP

ReadOne	PROC	NEAR
	PUSH	CX
	PUSH	CS
	POP	DS
	MOV	CX,13
	REP	MOVSB
	PUSH	ES
	POP	DS
	MOV	AX,3D20h
	INT	21h
	POP	CX
	JC	RdOne1
	MOV	BX,AX
	MOV	DX,BP
	MOV	AH,3Fh
	INT	21h
	PUSHF
	PUSH	AX
	MOV	AH,3Eh
	INT	21h
	POP	AX
	POP	DX
	JC	RdOne1
	PUSH	DX
	POPF
RdOne1:	RET
ReadOne	ENDP

Expand	PROC	NEAR
	XOR	DI,DI
	MOV	CX,WCBsave
	REP	MOVSB
	MOV	DI,OFFSET DskBuf
	MOV	AX,ES:EndBuf
	SUB	AX,DI
	XOR	DX,DX
	MOV	CX,SIZE FilEnt
	DIV	CX
	MOV	CX,AX
	JCXZ	Expnd2
Expnd1:	PUSH	DI
	PUSH	CX
	MOV	CX,12
	REP	MOVSB
	MOV	AL,0
	STOSB
	MOVSB
	POP	CX
	POP	DI
	ADD	DI,SIZE FilEnt
	LOOP	Expnd1
Expnd2:	RET
Expand	ENDP

	INCLUDE	VCSUB1.INC
	INCLUDE	VCSUB2.INC
	INCLUDE	VCSUB3.INC

	.CONST
	ASSUME	CS:DGROUP
Last:

;------------- Arithmetic for the compiled code --------
; What the compiler calls for numbers of 32 bits, near and in its own code
; segment. The C library has these, and the program has no C library.
CTEXT	SEGMENT	BYTE PUBLIC 'CCODE'
	ASSUME	CS:CTEXT
	PUBLIC	__U4M, __U4D

;			DX:AX - number
;			CX:BX - number
;	Return :	DX:AX - product, the low 32 bits of it
__U4M	PROC	NEAR
	PUSH	SI
	PUSH	DI
	MOV	SI,AX
	XCHG	AX,DX			; High by low
	MUL	BX
	MOV	DI,AX
	MOV	AX,SI			; Low by high
	MUL	CX
	ADD	DI,AX
	MOV	AX,SI			; Low by low
	MUL	BX
	ADD	DX,DI
	POP	DI
	POP	SI
	RET
__U4M	ENDP

;			DX:AX - number
;			CX:BX - what to divide it by
;	Return :	DX:AX - quotient
;			CX:BX - remainder
__U4D	PROC	NEAR
	OR	CX,CX
	JNZ	U4D1
	MOV	CX,AX			; By 16 bits: a half at a time
	MOV	AX,DX
	XOR	DX,DX
	DIV	BX
	XCHG	AX,CX
	DIV	BX
	MOV	BX,DX
	MOV	DX,CX
	XOR	CX,CX
	RET
U4D1:	PUSH	SI			; By more: a bit at a time
	PUSH	DI
	PUSH	BP
	XOR	SI,SI			; SI:DI - remainder
	XOR	DI,DI
	MOV	BP,32
U4D2:	SHL	AX,1
	RCL	DX,1
	RCL	DI,1
	RCL	SI,1
	JC	U4D3
	CMP	SI,CX
	JB	U4D4
	JA	U4D3
	CMP	DI,BX
	JB	U4D4
U4D3:	SUB	DI,BX
	SBB	SI,CX
	INC	AX
U4D4:	DEC	BP
	JNZ	U4D2
	MOV	BX,DI
	MOV	CX,SI
	POP	BP
	POP	DI
	POP	SI
	RET
__U4D	ENDP
CTEXT	ENDS

	END	Start
