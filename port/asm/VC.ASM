;-------------------------------------------------------
;		   Idea :	15.05.1991
;		  Begin :	16.05.1991
;		Version :	16.06.2000
;-------------------------------------------------------

	OPTION	OLDSTRUCTS		; Structure fields are global names
	OPTION	NOSCOPED		; Labels inside procedures are global

;-------------- The two parts of the program -----------
; The program is in two parts. One stays in the memory while a command
; runs: it begins with the PSP, and is C, port/resident.c. What is here
; of it is the order of its segments, which the linker takes from the
; first object file: RHEAD is the room of the PSP, which the file has as
; nothing, RTEXT its compiled code, the four that follow are
; those that the compiler makes for its data, of which the last, R_BSS,
; is the stack, and RGAP is the paragraph in which DOS keeps the header
; of the block of memory that follows. The other part, which goes while
; a command runs, begins behind them, at TrnBeg, and is all that
; follows. It begins with the constants of the C code, CONST2, and not
; with code: the machine of the tests is many times slower at every
; write to a page of the memory that has code which it has run, and the
; stack ends just in front. The segments are declared before the model
; so that the linker puts them first.
;
; Every offset counts from the PSP, here and in the data of the part
; that stays. That is by the groups. The assembly's begins with RHEAD
; and has the compiled code of the part that stays, RTEXT, and the
; constants of the C code. The data of the part that stays is in a group
; that the compiler makes for port/resident.c alone, R_GROUP, of the
; four segments that lie behind the code: RPSP, which is empty and the
; first of all, is of that group so that it begins where the other does.
RPSP	SEGMENT	PARA PUBLIC 'RPSP'
RPSP	ENDS
RHEAD	SEGMENT	PARA PUBLIC 'RHEAD'
RHEAD	ENDS
RTEXT	SEGMENT	BYTE PUBLIC 'RCODE'
RTEXT	ENDS
RCONST	SEGMENT	WORD PUBLIC 'FAR_DATA'
RCONST	ENDS
RCONST2	SEGMENT	WORD PUBLIC 'FAR_DATA'
RCONST2	ENDS
R_DATA	SEGMENT	WORD PUBLIC 'FAR_DATA'
R_DATA	ENDS
R_BSS	SEGMENT	WORD PUBLIC 'FAR_DATA'
R_BSS	ENDS
RGAP	SEGMENT	PARA PUBLIC 'RGAP'
RGAP	ENDS
CONST	SEGMENT	WORD PUBLIC 'CONST'
	PUBLIC	TrnBeg			; For tools/mkexe.c
TrnBeg	LABEL	BYTE
CONST	ENDS
CONST2	SEGMENT	WORD PUBLIC 'DATA'
CONST2	ENDS
	.MODEL	Tiny
DGROUP	GROUP	RHEAD, RTEXT, RGAP, CONST, CONST2
R_GROUP	GROUP	RPSP

;------------------- What is left here -------------------
; All the code of the program is C, port/*.c, and so are its data, its
; texts and its tables. What is left here is the order of the segments,
; above, the name that C code has for the main data, and the main data
; and a panel laid out, which nothing here uses any more: the C code has
; structures of its own for them, in port/vc.h.

RHEAD	SEGMENT
	PUBLIC	_md
_md	LABEL	BYTE			; The main data segment, to C code:
					; whatever is at offset 0 of DS
	ORG	100h			; Behind the PSP
RHEAD	ENDS

LenPath	EQU	68			; Length of path name
LenColr	EQU	40			; Length of color table
LenHist	EQU	512			; Length of hisory buffer
LenExt	EQU	512			; Length of extension file buffer
LenMenu	EQU	4000			; Length of menu buffer
LenStr	EQU	64			; Length of path & file string
LenBat	EQU	1000			; Length of full command line
LenHelp	EQU	3000			; Maximal length of help page
MinFls	EQU	256			; Minimum files in window
MinTree	EQU	768			; Minimum tree size
MaxLins	EQU	50			; Maximum screen lines
MaxProg	EQU	18			; Maximal resident quantaty
NmbStrs	EQU	32			; Number of stars

;------------------ Global structures ------------------
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
; C: port/resident.c, the part of the program that stays in the memory
; while a command runs, with its data, struct Stays of port/vc.h, and
; the stack, DataSav, at its end.

RGAP	SEGMENT
	DB	16	DUP(?)		; The header of the transit part's block
RGAP	ENDS

;-------------------------- Data -----------------------
; The texts, the boxes, the dialogs and the tables of the program are C:
; the files of constants, port/*_t.c.

	END
