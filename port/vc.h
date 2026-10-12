/*
 * Volkov Commander in C: what the translated parts share.
 *
 * The program is being translated from assembly a few routines at a time,
 * and both halves run as one program. The C code therefore lives in the
 * world the assembly built, which is not the one a C compiler assumes:
 *
 *   - DS points at the main data segment, a block that the program
 *     allocates when it starts. It holds one struct MainDat and nothing
 *     else, so C code has no static data of its own.
 *   - Constants sit in the code segment, as they do in the assembly.
 *     Declare them CODE. String literals are not available, but to give
 *     such a constant its contents.
 *   - The stack is in a third segment. A pointer to a local variable must
 *     be declared STACK.
 *   - Each file panel has a segment to itself, reached by far pointers.
 *
 * Routines keep the names they have in the assembly source, and data keeps
 * its layout: the structures below mirror those of port/asm/VC.ASM.
 */
#ifndef VC_H
#define VC_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define CODE  __based(__segname("_CODE"))   /* a constant, in the code segment */
#define STACK __based(__segname("_STACK"))  /* a pointer to a local variable */

/* Write a 16-bit value to a buffer of bytes, low byte first, and step the
   pointer past it. */
#define PUT16(buffer, value) (*(u16 *)(buffer) = (value), (buffer) += 2)

/* Fails to compile when a structure is not laid out as in the assembly. */
#define LAID_OUT(type, size) typedef char type##_size[sizeof(struct type) == (size) ? 1 : -1]

#define LenColr 40                      /* Length of color table */
#define LenHist 512                     /* Length of hisory buffer */
#define LenExt  512                     /* Length of extension file buffer */
#define LenMenu 4000                    /* Length of menu buffer */
#define LenStr  64                      /* Length of path & file string */
#define LenBat  1000                    /* Length of full command line */
#define LenHelp 3000                    /* Maximal length of help page */
#define LenPath 68                      /* Length of path name */
#define MinFls  256                     /* Minimum files in window */
#define MinTree 768                     /* Minimum tree size */
#define MaxLins 50                      /* Maximum screen lines */
#define NmbStrs 32                      /* Number of stars */

#pragma pack(push, 1)

/* Execute program block. */
struct EPBs {
    u16 EnvSeg;                         /* Segment of new environment */
    u32 CmdLine;                        /* Pointer to command line */
    u32 FCB1;                           /* Pointer to FCB1 */
    u32 FCB2;                           /* Pointer to FCB2 */
};

/* Country information, as DOS returns it. */
struct CNTRY {
    u16 DateFmt;                        /* Date format */
    u8  Currency[5];
    u8  Sep1000[2];                     /* Thousand separator */
    u8  DecimalSep[2];
    u8  DateSep[2];                     /* Date separator */
    u8  TimeSep[2];                     /* Time separator */
    u8  CurrencyFmt;
    u8  CurrencyDigits;
    u8  TimeFmt;                        /* Time format */
    u32 CaseMap;                        /* Addres of case map */
    u8  DataSep[2];
    u8  Reserved[8];
};

/* Data transfer area of the DOS file search. */
struct DTAs {
    u8  Reserved[0x15];
    u8  FilAttr;
    u16 FilTime;
    u16 FilDate;
    u32 FilSize;
    u8  FilName[13];
    u8  Unused;
};

/* File entry in DskBuf. */
struct FILENT {
    u8  NameF[13];
    u8  AttrF;
    u16 TimeF;
    u16 DateF;
    u32 SizeF;
    u16 NumbF;
};

/* Subdirectory entry of NCD file. */
struct DIRENT {
    u8  DirName[13];                    /* Subdirectory name in ASCIIZ */
    u8  DirLevel;                       /* Subdirectory level */
    u8  DirShift;                       /* 1 in next entry - there is branch */
    u8  DirCont;                        /* 0 - end of branch, 1 - it goes on */
};

/* Structure of NCD file. */
struct DIRTREE {
    u8  TreHead[5];                     /* Tree file header 'PNCI',0 */
    u16 TreeLen;                        /* Number of subdirectories */
    u16 TreSum1;                        /* Check sum of the header */
    u16 TreSum2;                        /* Check sum of the list */
};
/* The list of subdirectories begins where TreSum2 does: TreList there. */

/* A star of the screen saver. */
struct STAR {
    u16 StarMod;                        /* Low byte - counter, High - explosion */
    u16 StarPos;                        /* Coordinates of star */
};

/* The main data segment. */
struct MainDat {
    u8  ColTab[6 * LenColr];            /* Color table (6 pallets) */
    u8  FulScr;                         /* Full screen */
    u8  MenuVs;                         /* Menu always visible */
    u8  KeyBar;                         /* Key bar */
    u8  Prompt;                         /* Path prompt */
    u8  InsDwn;                         /* Ins moves down */
    u8  AutoMnu;                        /* Auto user menus */
    u8  MiniSta;                        /* Mini status */
    u8  Clock;                          /* Clock */
    u8  AutoCD;                         /* Auto change dir */
    u8  AutoSav;                        /* Auto save setup */
    u8  ColTyp;                         /* Monochrome / Color / LapTop */
    u8  BlDelay;                        /* Screen blank delay */
    u8  LoadFl;                         /* Small mode */
    u8  UnSlct;                         /* Unselect files when group operation */
    u8  ErrSnd;                         /* Beep when error */
    u8  LftMous;                        /* Left handed mouse */
    u8  ZoomWin;                        /* Zomming windows */
    u8  AltBar;                         /* Alt alone selects menu */
    u8  EscBar;                         /* Esc toggles panels on/off */
    u8  ExecTyp;                        /* Exec command without COMMAND.COM */
    u8  AutoSiz;                        /* Auto calculate directory sizes */
    u8  HidDirs;                        /* Show hidden subdirs in tree panel */
    u8  QuitAck;                        /* Confirmation on Quit */
    u8  SnowMod;                        /* Snow mode (2=auto) */
    u8  AltView;                        /* Alt-F3 - external viewer */
    u8  AltEdit;                        /* Alt-F4 - external editor */
    u8  ClrBuf;                         /* Clear buffer flag */
    u8  Active;                         /* Active panel */
    u8  Ctrl_O;                         /* Visible status of unactive panel */
    u8  ChkTSR;                         /* TSR manager */
    u8  ChgTree;                        /* Auto rescan tree */
    u8  DriveB;                         /* Show logic drive B: in 1 floppy system */
    u8  DelayTm[2];                     /* Auto change dir delay time */
    u8  StarTab[5];                     /* Star table */
    u8  ResIni[51];                     /* Reserved */
    /* End of main data' part saved in VC.INI */

    u8  LoadRes;                        /* Original LoadFl */
    u8  ScrMod;                         /* Current screen mode */
    u8  Mouse;                          /* 1 if Mouse was found */
    u8  BreakFl;                        /* Break DOS's status */
    u8  MenuFl;                         /* 1 to bring user menu automatically */
    u8  SelMas[13];                     /* Wildcard for selection */
    u8  HstBuf[LenHist];                /* History buffer */
    u8  ExtMain[LenExt];                /* Buffer for the main extension file */
    struct EPBs EPB;                    /* Execute program block */
    u16 MouseX;                         /* Mouse cursor X */
    u16 MouseY;                         /* Mouse cursor Y */
    u16 BatAdr;                         /* Pointer to command in buffer */
    u16 BatPoin;                        /* Pointer to command */
    u16 LastMnu;                        /* The last pull down menu item */
    u16 OvlPth;                         /* Pointer to file name in environment */
    u16 ComSpec;                        /* Pointer to COMSPEC= in environment */
    u8  CrnPath[LenPath];               /* Current directory in the tree panel */
    u8  ViewFil[LenStr + 1];            /* Source filename */
    u8  ToFile[LenStr + 1];             /* Target filename */
    u8  FindStr[LenStr + 1];            /* FileFind wildcard */
    u8  SrchStr[LenStr + 1];            /* String for search */
    u8  DosChg[5];                      /* Reserved for change drive or /C_ */
    u8  DosBuf[LenBat];                 /* Command line */
    /* End of main data' part saved in memory */

    u8  DosLin;                         /* Line number of DOS's command line */
    u8  DosCol;                         /* Current column of DOS's command line */
    u8  BegCol;                         /* Start position of DOS's command line */
    u8  OutCol;                         /* Start position for output com. line */
    u8  ScrPage;                        /* Current screen page number */
    u8  TstEGA;                         /* 1 if EGA or VGA present */
    u8  Lines;                          /* Number of lines on screen */
    u8  Snow;                           /* Snow on/off */
    u8  TopView;                        /* TopView output */
    u8  DesqVie;                        /* Running under DESQVIEW */
    u8  WinLeav;                        /* Leave time under Windows */
    u8  CrntCol;                        /* Color which DOS currently uses */
    u8  Quote;                          /* Quote next character */
    u8  BarPrev;                        /* Previous Key Bar */
    u8  SftErr;                         /* Shift error boxes down */
    u8  BlocEr;                         /* Disable critical error box */
    u8  PathEr;                         /* Error when reading current path */
    u8  FlagCD;                         /* 1 when needs auto CD */
    u8  TreeTyp;
    u8  WinMous;
    u8  MousEnt;
    u8  OldFlg;
    u8  HlpPage;
    u8  ZoomOne;
    u8  OneFile;                        /* Do one file and exit */
    u8  ErrDat;
    u16 VerDOS;
    u16 WCBcrn;
    u16 WCBsec;
    u16 WCB1seg;
    u16 WCB2seg;
    u16 ViewSeg;
    u16 FreMem1;
    u16 FreMem2;
    u16 Memorys;
    u16 HstAdr;
    u16 TreAdr;
    u16 TreFrst;
    u16 TreCrnt;
    u16 ClstSiz;
    u16 Total;
    u16 FreeDsk;
    u16 InfoLen;
    u16 Rand1;
    u16 Rand2;
    u16 FncAddr;
    u16 MousPos;
    u32 MousTim;
    u32 AutoTim;
    u32 Screen;                         /* Address of screen */
    u32 ErrVct;                         /* Original vector 24h */
    u32 BrkVct;                         /* Original vector 1Bh */
    u8  Volume[12];                     /* Volume label */
    u8  DosPth[LenPath];                /* The current path */
    u8  NcPath[LenPath];                /* The path from VC= variable */
    u8  TempDir[LenPath];               /* The path from TEMP= variable */
    struct CNTRY Country;               /* Country information */
    struct DTAs DTA;                    /* Data tramsnir area */
    u8  LowTab[128];                    /* Lower case table */
    u8  TmpBuf[256];
    u8  TmpErr[256];
    struct STAR Stars[NmbStrs];
    u8  MenuBuf[LenMenu];
    u8  InfBuf[(MaxLins - 13) * 38];
    u8  Even;
    u16 ScrBuf[MaxLins * 80];
    u16 UserScr[(MaxLins - 1) * 80];
    u16 WinBuf[(MaxLins - 2) * 77];
    u16 ErrBuf[(MaxLins - 2) * 77];
    u16 ErrBuf1[(MaxLins - 2) * 77];
    u8  HelpBuf[LenHelp];
    u8  TreeBuf[MinTree * sizeof(struct DIRENT) + sizeof(struct DIRTREE)];
};

/* Window control block: a file panel, in a segment of its own. */
struct WCBdef {
    u8  WinTyp;
    u8  BrfFul;
    u8  Visible;
    u8  Hidden;
    u8  MasTyp;
    u8  SortTyp;
    u8  Ctrl_L;
    u8  DskMas[13];
    u8  WinPath[LenPath];
    /* End of WCB part saved in VC.INI */

    u16 CurAdr;
    u16 First;
    u16 EndBuf;
    /* End of WCB part saved in memory */

    u8  ExtBuf[LenExt];
    u16 EndFul;
    u8  ChgFlg;
    u8  SizDir;
    /* End of WCB */

    struct FILENT DskBuf[MinFls];
};

#pragma pack(pop)

LAID_OUT(CNTRY, 0x20);
LAID_OUT(DTAs, 0x2C);
LAID_OUT(FILENT, 0x18);
LAID_OUT(MainDat, 0xF48B);
LAID_OUT(WCBdef, 0x1A62);

/* The main data segment, which DS points at. */
extern struct MainDat __near md;

/* Texts of the assembly that C code shows too. They are in its code
   segment, so naming one costs a relocation. */
extern const u8 __far Title_S[];        /* the program's name and version,
                                           behind two bytes of markup */
extern const u8 __far Var_S[];          /* a line that is the text at
                                           TmpErr+1 */
extern const u8 __far Tree_S[];         /* the title of the tree */
#pragma aux Title_S "*"
#pragma aux Var_S "*"
#pragma aux Tree_S "*"

/* The numbers of the colours of a panel, for Color. */
#define Back_C  0                       /* what is neither of the others */
#define Cursr_C 1                       /* under the cursor */
#define Slct_C  2                       /* selected */
#define CurSl_C 3                       /* selected, under the cursor */
#define Title_C 4                       /* the titles of the columns */

/* The registers of a call of DOS. DS goes with DX: a pointer to what DOS
   takes there, a name or a buffer, sets both. ES goes to DOS only by
   Int21e of dos.c: otherwise DOS is called with it on the main data,
   and it is what DOS leaves there, for the calls that answer with an
   address in ES:BX. */
struct DosRegs {
    u16 ax, bx, cx;
    const u8 __far *name;               /* DX, and DS behind it */
    u16 si, di;
    u16 es;                             /* as DOS leaves it */
};

/* The registers of a call of the BIOS for the screen. */
struct BiosRegs {
    u16 ax, bx, cx, dx;
};

/* Routines that are still assembly, for C code to call. Each has an entry
   in the assembly, the routine's name with _a, which is called far and
   calls the routine (see FromC in port/asm/VC.ASM). They take and return
   what the routine's registers hold, so a value in a high byte stays
   there; flags do not come back. */
/* A call of what FncAddr names, for a file with these attributes at
   TmpBuf: the high byte of what it returns is not 0 to stop. FncAddr has
   the address of a routine of the assembly, which is the stub of a C
   function: DiSi10_s is that of DiSi10 of read.c, which DirSize has
   DirFunc call for every file. */
u16 FncCall(u16 attributes);
extern const u8 __far DiSi10_s[];
#pragma aux FncCall "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux DiSi10_s "DiSi10"
/* RestVct gives the vectors of the critical error and of Ctrl-Break back
   to whoever had them, and HookVct takes them again: it returns where
   its routine for Ctrl-Break is, in the part of the program that stays
   in the memory. Quit puts back what DOS had on the screen before the
   program drew over it, but for the line of the key bar, which is blank
   if there is one. ErrRdy says that the disk of a drive cannot be read
   and asks for another drive: it returns the letter in the low byte, or
   0 there for none. See each in main.c. */
void RestVct(void);
u16 HookVct(void);
void Quit(void);
u16 ErrRdy(u16 letter);
#pragma aux RestVct "*_c" __far modify exact [ax dx]
#pragma aux HookVct "*_c" __far value [ax] modify exact [ax]
#pragma aux Quit    "*_c" __far modify exact [ax]
#pragma aux ErrRdy  "*_c" __far parm [ax] value [ax] modify exact [ax]
/* The setup file: see each in setup.c. MakeIni writes into MenuBuf what
   the file holds, and returns how long that is. SaveSet writes it to the
   file: 1 in the high word if it could not, with what DOS answered in
   the low one, or 53h there for a drive that is not ready. */
u16 MakeIni(void);
u32 SaveSet(void);
#pragma aux MakeIni "*_c" __far value [ax] modify exact [ax]
#pragma aux SaveSet "*_c" __far value [dx ax] modify exact [ax dx]

/* What the setup file holds: three letters V, of each panel what a
   WCBdef has before CurAdr, of the main data what it has before LoadRes,
   and the sum of all these bytes. */
#define WCBini  ((u16)(u32)&((struct WCBdef __far *)0)->CurAdr)
#define CmnIni  ((u16)&((struct MainDat *)0)->LoadRes)
#define INI_LEN (3 + 2 * WCBini + CmnIni + 2)

/* DOS, the BIOS, the mouse driver and the machine: the functions of
   dos.c, which has every software interrupt of the C code and what each
   is for. Nothing else calls one.

   The part of the program that stays in the memory while a command runs
   (resident.c) cannot call them: they are of the part that goes, and
   not in the memory then. So the build compiles dos.c a second time, into
   that part, with RESIDENT defined as it is for resident.c: the names of
   the second copy end otherwise, and resident.c calls those. */
#ifdef RESIDENT
#define DOS_NAME "*_r"
#else
#define DOS_NAME "*_c"
#endif
struct MouseAt {                        /* what the mouse driver says: */
    u16 buttons;                        /* the buttons held, the left one 1,
                                           the right one 2, the middle 4 */
    u16 x, y;                           /* where the pointer is, in dots */
};
u16 Int21d(struct DosRegs STACK *regs);
u16 Int21f(struct DosRegs STACK *regs, u16 carry);
u16 Int21e(struct DosRegs STACK *regs);
void Int10(struct BiosRegs STACK *regs);
void TopView(u16 __far *cells, u16 count);
u16 __far *TopScr(u16 __far *screen);
u32 Int16(u16 ax);
u32 Int2F(u16 ax, u16 bx, u16 cx);
u16 Int33(u16 ax, u16 bx, u16 cx, u16 dx);
void MousAt(struct MouseAt STACK *state);
u16 CaseMap(u16 letter);
u16 InPort(u16 port);
void IntOff(void);
void IntOn(void);
void Int28(void);
void DvPause(void);
#pragma aux Int21d  DOS_NAME __far parm [bx] value [ax] modify exact [ax]
#pragma aux Int21f  DOS_NAME __far parm [bx] [ax] value [ax] modify exact [ax]
#pragma aux Int21e  DOS_NAME __far parm [bx] value [ax] modify exact [ax]
#pragma aux Int10   DOS_NAME __far parm [bx] modify exact [ax]
#pragma aux TopView DOS_NAME __far parm [es di] [cx] modify exact [ax]
#pragma aux TopScr  DOS_NAME __far parm [es di] value [es di] modify exact [ax di es]
#pragma aux Int16   DOS_NAME __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux Int2F   DOS_NAME __far parm [ax] [bx] [cx] value [dx ax] modify exact [ax dx]
#pragma aux Int33   DOS_NAME __far parm [ax] [bx] [cx] [dx] value [ax] modify exact [ax]
#pragma aux MousAt  DOS_NAME __far parm [di] modify exact [ax]
#pragma aux CaseMap DOS_NAME __far parm [ax] value [ax] modify exact [ax]
#pragma aux InPort  DOS_NAME __far parm [ax] value [ax] modify exact [ax]
#pragma aux IntOff  DOS_NAME __far modify exact [ax]
#pragma aux IntOn   DOS_NAME __far modify exact [ax]
#pragma aux Int28   DOS_NAME __far modify exact [ax]
#pragma aux DvPause DOS_NAME __far modify exact [ax]

/* The registers as an interrupt function of the compiler saves them on
   the stack and puts them back: its first parameter. */
struct Saved {
    u16 gs, fs, es, ds, di, si, bp, sp, bx, dx, cx, ax;
};

/* A line as the assembly hands it over: the registers that describe it,
   pushed in this order by CallLine in port/asm/VC.ASM. */
struct LineArgs {
    u16 value;                          /* AX or BX: see each routine */
    u16 *screen;                        /* DI: where in the screen buffer */
    const u8 __far *line;               /* DS:SI */
    const u8 __far *colors;             /* CS:BP: the numbers of four colours */
    const u8 __far *include;            /* CS:DX */
};

/* What AdrNum hands back, in the order in which its stub takes it off the
   stack. */
struct Place {
    u16 cursor;                         /* the number of the cursor's entry */
    u16 first;                          /* of the first one shown */
    u16 last;                           /* of the last one there is */
    u16 lines;                          /* lines of the panel that show
                                           entries */
};

/* What Number() does about thousands. */
#define NUM_SEPARATED 1                 /* put the country's separator in */
#define NUM_PADDED    2                 /* fill with spaces up to full width */

/* Routines that are C, for C code in other files than theirs to call.
   The assembly calls them too, by these conventions (see the stubs in
   port/asm). */
u16 Color(u16 number);                  /* screen.c */
u16 *BasAdr(u16 place);
u16 __far *MvsColr(u16 __far *screen, u16 count, u16 color);
u16 __far *MovDz0(u16 __far *screen, const u8 __far *text);
#pragma aux Color   "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux BasAdr  "*_c" __far parm [bx] value [di] modify exact [di]
#pragma aux MvsColr "*_c" __far parm [es di] [cx] [ax] value [es di] modify exact [di es]
#pragma aux MovDz0  "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]
u16 __far *HexByt(u16 __far *screen, u16 byte);         /* digits.c */
u16 TstHex(u16 digit);
#pragma aux HexByt "*_c" __far parm [es di] [ax] value [es di] modify exact [di es]
#pragma aux TstHex "*_c" __far parm [ax] value [ax] modify exact [ax]
void Convert(const u8 __far *name, u8 *buffer);         /* mask.c */
u16 TstMas(const u8 __far *name, const u8 *mask);
#pragma aux Convert "*_c" __far parm [dx si] [di] modify exact [ax]
#pragma aux TstMas  "*_c" __far parm [dx si] [di] value [ax] modify exact [ax]
void NgLine(struct LineArgs STACK *args);               /* line.c */
void CntLine(struct LineArgs STACK *args);
#pragma aux NgLine  "*_c" __far parm [bx] modify exact [ax bx cx dx es]
#pragma aux CntLine "*_c" __far parm [bx] modify exact [ax bx cx dx es]
u8 __far *AddFile(u8 __far *end);                       /* string.c */
u8 __far *MvzLine(u8 __far *to, const u8 __far *from, u16 length);
u16 LowCase(u16 letter);
const u8 __far *CutPath(const u8 __far *path);
void PathLin(u8 __far *text);
void LetLin(u8 __far *text);
void ShrtLin(u8 __far *text, u16 limit);
#pragma aux AddFile "*_c" __far parm [es di] value [es di] modify exact [di es]
#pragma aux MvzLine "*_c" __far parm [es di] [dx si] [cx] value [es di] modify exact [di es]
#pragma aux LowCase "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux CutPath "*_c" __far parm [es si] value [es si] modify exact [si es]
#pragma aux PathLin "*_c" __far parm [es si] modify exact [ax]
#pragma aux LetLin  "*_c" __far parm [es si] modify exact [ax]
#pragma aux ShrtLin "*_c" __far parm [es si] [ax] modify exact [ax]
u8 *Number(u8 *buffer, u16 high, u16 low, u16 mode);    /* format.c */
u16 *SzDtTm(const struct FILENT __far *file, u16 *screen, u16 color);
#pragma aux Number  "*_c" __far parm [di] [bx] [cx] [ax] value [di] modify exact [di]
#pragma aux SzDtTm  "*_c" __far parm [es si] [di] [ax] value [di] modify exact [di es]
void SetMask(u16 panel);                                /* files.c */
void Sort(u16 panel);
u16 FnFile(u16 panel, const u8 __far *name);
u16 NearFl(u16 panel, const u8 __far *name, u16 end);
#pragma aux SetMask "*_c" __far parm [ax] modify exact [ax]
#pragma aux Sort    "*_c" __far parm [ax] modify exact [ax]
#pragma aux FnFile  "*_c" __far parm [dx] [es di] value [si] modify exact [si]
#pragma aux NearFl  "*_c" __far parm [dx] [es di] [bx] value [ax] modify exact [ax]
u16 BegBX(u16 panel);                                   /* place.c */
u16 SizDX(void);
u16 LnsAX(void);
u16 InvWCB(u16 panel);
u16 FilePan(u16 panel);
void AdrNum(u16 panel, struct Place STACK *place);
void CorPar(u16 panel);
#pragma aux BegBX   "*_c" __far parm [bx] value [bx] modify exact [bx]
#pragma aux SizDX   "*_c" __far value [dx] modify exact [dx]
#pragma aux LnsAX   "*_c" __far value [ax] modify exact [ax]
#pragma aux InvWCB  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux FilePan "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux AdrNum  "*_c" __far parm [ax] [bx] modify exact [ax]
#pragma aux CorPar  "*_c" __far parm [ax] modify exact [ax]
u8 *PtPt10(u16 panel);                                  /* panel.c */
#pragma aux PtPt10  "*_c" __far parm [ax] value [di] modify exact [ax di]
/* The colours of the tree are four numbers in the code, of the assembly's
   or of the C's: the stub has ES on its own segment for them. */
void PutTree(u16 panel, u16 place, u16 size, const u8 __far *colors);
#pragma aux PutTree "*_c" __far parm [ax] [bx] [dx] [es si] modify exact [ax es]  /* tree.c */
void PutInf(u16 panel);                                 /* info.c */
#pragma aux PutInf  "*_c" __far parm [ax] modify exact [ax]
/* Where a command goes on when it is done. A command is entered by a
   jump, from the table of the keys or from a menu, and jumps on; one that
   is C returns one of these, and its stub in the assembly jumps there
   (F1Next in port/asm/VCF1.INC). */
#define TO_FUNC04 0                     /* back to the panels as they are */
#define TO_FNCSEL 1                     /* have what FncAddr names called
                                           for what is selected, or for
                                           what the cursor is on */
#define TO_FNCEXT 2                     /* read the panels again if TmpBuf
                                           has something at 91 */
#define TO_FNCEX1 3                     /* read the panels again */
#define TO_F24_02 4                     /* the current directory is
                                           another: of F67_00 only, which
                                           hands back the panel too */

/* Where the program goes on after a command of the keys. Such a command
   is entered by a jump from the main loop, which is still assembly, and
   jumps back to one of several places of it, or on into another command.
   In C it is a function that gets the panel which the assembly has in DS
   and returns where to go on in the low byte: its stub jumps there by the
   table CmdNx_T of port/asm/VCFUNC.INC. These are the places of that
   table for the commands of enter.c and view.c; keys.c has those of its
   own commands, the first fourteen, by other names. */
#define NEXT_FUNC05   1                 /* wait for the next key, once the
                                           setup has been saved if it has
                                           changed and is saved by itself */
#define NEXT_FUNC04   2                 /* first draw the menu bar */
#define NEXT_INIT50   8                 /* draw the panels again, reading
                                           those that have changed */
#define NEXT_INIT11  14                 /* start over from what F21_30 has
                                           saved in the part of the program
                                           that stays in the memory */
#define NEXT_EXEC    15                 /* and first have that part run
                                           the commands saved there */
#define NEXT_VIEW    16                 /* V00_00: the viewer on the file
                                           ViewFil, or with 1 in the high
                                           word the editor */
#define NEXT_DIRINFO 17                 /* V00_02: the editor in the window
                                           of an Info panel */
#define NEXT_EXIT    18                 /* leave the program */

/* Save the setup, the command line and the panels in the part of the
   program that stays in the memory, and give up the main data, for that
   part to run what the command line holds: returns NEXT_EXEC, or
   NEXT_INIT11 if there is nothing to run, for the command to return in
   its turn. See
   enter.c. */
u32 F21_30(void);
#pragma aux F21_30 "*_c" __far value [dx ax] modify exact [ax dx]
/* The commands of a line of an extension file or a menu file, in
   MenuBuf: F34_30 looks for the next one from `text`, and F34_40 at
   whether the command at `text` is only the name of a file, a program
   for `view` and else a menu, whose path it has put at TmpBuf then. Both
   return a place in the high word, and in the low one 1 if there is no
   command, or if it is such a name. See view.c. */
u32 F34_30(u16 text);
u32 F34_40(u16 text, u16 view);
#pragma aux F34_30 "*_c" __far parm [si] value [dx ax] modify exact [ax dx]
#pragma aux F34_40 "*_c" __far parm [si] [cx] value [dx ax] modify exact [ax dx]
/* Have both panels read again: where a command ends that has changed
   what is on the disk. Returns 1. See dirfunc.c. */
u16 FncEx1(void);
#pragma aux FncEx1 "*_c" __far value [ax] modify exact [ax]

/* Calls what FncAddr names for every file of the directory at TmpBuf, and
   returns 1 if every call did what it was to, 0 if one did not, and 100h
   when it was stopped. */
u16 DirFunc(void);                                      /* dirfunc.c */
#pragma aux DirFunc "*_c" __far value [ax] modify exact [ax]
/* Marks the tree's file of the drive that `path` names, or of the current
   one, as out of date. Returns what DOS answered in the low word, and
   something that is not 0 in the high one for an error. */
u32 DelTr0(const u8 *path);
#pragma aux DelTr0  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]

/* Where a box is on the screen, as DialBox says it: what the assembly
   has in BX, DX and SI then. */
struct Box {
    u16 begin;                          /* the row in the high byte */
    u16 size;                           /* the rows in the high byte */
    u16 saved;                          /* where what it covers is kept */
};

/* The boxes and the dialogs, which the assembly describes: `box`,
   `include`, `table`, `control` and `colors` are offsets in its code
   segment, and `segment` is where what a box shows and a dialog sets is,
   MAIN_SEG for the main data. See each in dialog.c. */
void DialBox(u16 place, u16 box, u16 include, u16 segment,
             struct Box STACK *where);
u16 Dialog(u16 box, u16 include, u16 begin, u16 saved, u16 segment);
void InSt10(u16 place, u16 text, u16 length, u16 color);
u32 MinMnu(u16 control, u16 begin, u16 size, u16 colors, u16 keys, u16 item);
u32 MnMn10(u16 key, u16 place, u16 item, u16 size, u16 table);
u16 MnMn40(u16 colors, u16 place, u16 item, u16 size, u16 table);
#pragma aux DialBox "*_c" __far parm [bx] [ax] [dx] [cx] [di] modify exact [ax]
#pragma aux Dialog  "*_c" __far parm [ax] [dx] [bx] [si] [cx] value [ax] modify exact [ax]
#pragma aux InSt10  "*_c" __far parm [bx] [si] [cx] [ax] modify exact [ax]
#pragma aux MinMnu  "*_c" __far parm [si] [bx] [dx] [di] [ax] [cx] value [dx ax] modify exact [ax dx]
#pragma aux MnMn10  "*_c" __far parm [ax] [bx] [cx] [dx] [di] value [dx ax] modify exact [ax dx]
#pragma aux MnMn40  "*_c" __far parm [ax] [bx] [cx] [dx] [di] value [ax] modify exact [ax]
/* The key bar and the menu bar, the command line and its prompt: see
   each in bars.c. `segment` is where ClsLine reads the colour, MAIN_SEG. */
u16 *ClsLine(u16 place, u16 segment);
void PutKey(u16 bar);
void PutKey0(u16 bar, u16 shift);
void PutMnu(u16 item);
void PtMnu1(u16 item);
void PutPrm(u16 panel);
void DosCom(void);
#pragma aux ClsLine "*_c" __far parm [bx] [ax] value [di] modify exact [ax di]
#pragma aux PutKey  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutKey0 "*_c" __far parm [ax] [dx] modify exact [ax]
#pragma aux PutMnu  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PtMnu1  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutPrm  "*_c" __far parm [ax] modify exact [ax]
#pragma aux DosCom  "*_c" __far modify exact [ax]
/* The entries of an extension file or a menu file: FndItem returns where
   the next one is, the offset in the low word, and 1 in the high one at
   the end of the file; FindExt where in the main data the command for a
   file's extension is, or 0. See extfile.c. */
u32 FndItem(const u8 __far *file);
u16 FindExt(const u8 __far *name);
#pragma aux FndItem "*_c" __far parm [dx si] value [dx ax] modify exact [ax dx]
#pragma aux FindExt "*_c" __far parm [dx si] value [si] modify exact [ax si]
/* Waiting for a key, and a line that is typed into: see each in input.c.
   Input and Input0 return the key, the scan code in the high byte, where
   FCh says that the mouse did something; Input0 has 1 in the high word
   for a key clicked on the key bar. EdLin returns the column of the
   cursor, and 1 in the high word if anything changed. CmpSym is 1 for a
   character that words end at. Search is the speed search among names
   in the segment `segment`. Rnd is the next of the program's random
   numbers. */
u16 Input(u16 bar);
u32 Input0(u16 bar);
u16 Rnd(void);
u32 EdLin(u16 key, u8 *text, u16 cursor, u16 length);
u16 EdSub1(const u8 *line, u16 cursor);
u16 CmpSym(u16 symbol);
u16 Search(u16 segment, u16 at, u16 begin, u16 end, u16 step, u16 key);
#pragma aux Input   "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux Input0  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux Rnd     "*_c" __far value [ax] modify exact [ax]
#pragma aux EdLin   "*_c" __far parm [ax] [si] [dx] [cx] value [dx ax] modify exact [ax dx]
#pragma aux EdSub1  "*_c" __far parm [di] [ax] value [ax] modify exact [ax]
#pragma aux CmpSym  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux Search  "*_c" __far parm [di] [si] [bx] [cx] [dx] [ax] value [si] modify exact [ax si]

/* What the program asks of DOS and the BIOS at every turn: see each in
   system.c. Intr21 and Intr21f call DOS with the registers of `regs` and
   put AX, BX, CX and DX back there, DX where the offset of `name` was;
   they are not 0 if DOS set the carry flag: AX is its error then, or 53h
   for a drive that is not ready. Intr21f has the carry flag set when DOS
   is called, if `carry` is not 0. OpenFil opens a file, `mode` as DOS
   takes it in AL, and returns the handle in the low word, or the error
   there and 1 in the high one. SetPth asks DOS for the current drive and
   directory, into DosPth, and is not 0 if that fails. Fantom asks for
   the disk of a drive that shares a floppy drive with another, 1 for A:,
   and ChgDisk for that of the drive that a path of the main data names:
   not 0 if the user would not put it in. SelDrv makes a drive the
   current one, 0 for A:. */
u32 Timer(void);
void ClrKbd(void);
u16 TstBrk(void);
void Beep(void);
u16 UpCase(u16 letter);
void SetDTA(void);
u16 TstDrv(u16 letter);
void SelDrv(u16 drive);
u16 Intr21(struct DosRegs STACK *regs);
u16 Intr21f(struct DosRegs STACK *regs, u16 carry);
u32 DosTime(void);
u32 OpenFil(const u8 __far *name, u16 mode);
u16 SetPth(void);
u16 Fantom(u16 drive);
u16 ChgDisk(const u8 *path);
#pragma aux Timer   "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux ClrKbd  "*_c" __far modify exact [ax]
#pragma aux TstBrk  "*_c" __far value [ax] modify exact [ax]
#pragma aux Beep    "*_c" __far modify exact [ax]
#pragma aux UpCase  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux SetDTA  "*_c" __far modify exact [ax]
#pragma aux TstDrv  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux SelDrv  "*_c" __far parm [ax] modify exact [ax]
#pragma aux Intr21  "*_c" __far parm [bx] value [ax] modify exact [ax]
#pragma aux Intr21f "*_c" __far parm [bx] [ax] value [ax] modify exact [ax]
#pragma aux DosTime "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux OpenFil "*_c" __far parm [dx bx] [ax] value [dx ax] modify exact [ax dx]
#pragma aux SetPth  "*_c" __far value [ax] modify exact [ax]
#pragma aux Fantom  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux ChgDisk "*_c" __far parm [dx] value [ax] modify exact [ax]
/* The mouse: see each in mouse.c. GetMous returns the buttons in the low
   word and the place in the high one, InWind is not 0 for a place
   outside a window, and DoublMo is 2 to go on, 1 for a second click and 0
   for one that was let go elsewhere. */
void IniMous(void);
void TypMous(u16 pointer);
void SavMous(void);
void CurMous(u16 show);
u32 GetMous(u16 place);
u16 DoublMo(u16 begin, u16 size, u16 cursor);
u16 InWind(u16 where, u16 begin, u16 size);
#pragma aux IniMous "*_c" __far modify exact [ax]
#pragma aux TypMous "*_c" __far parm [ax] modify exact [ax]
#pragma aux SavMous "*_c" __far modify exact [ax]
#pragma aux CurMous "*_c" __far parm [ax] modify exact [ax]
#pragma aux GetMous "*_c" __far parm [cx] value [dx ax] modify exact [ax dx]
#pragma aux DoublMo "*_c" __far parm [bx] [dx] [si] value [ax] modify exact [ax]
#pragma aux InWind  "*_c" __far parm [cx] [bx] [dx] value [ax] modify exact [ax]
/* The screen, its cursor, and windows of the screen buffer: see each in
   window.c. A place has the row in its high byte, and a size the rows.
   `buffer` is an address in the main data. */
struct VideoMode {                      /* for ChkMode: as the BIOS says, */
    u16 ax;                             /* the mode, and above it the
                                           columns */
    u16 bx;                             /* the page, in the high byte */
};
u16 ChkMode(struct VideoMode STACK *mode);
void Cursor(u16 place);
void Cursor1(void);
u16 ChgCur(void);
void CurShp(u16 shape);
u16 GetCur(void);
void HidCur(void);
u32 Int10m(u16 ax, u16 bx, u16 cx, u16 dx);
u16 ScrOffs(void);
u16 SftEGA(u16 place);
void SavWin(u16 begin, u16 size, u16 buffer);
void ResWin(u16 begin, u16 size, u16 buffer);
void ResUser(u16 begin, u16 size);
void Window(u16 begin, u16 size);
void SavScr(void);
#pragma aux ChkMode "*_c" __far parm [bx] value [ax] modify exact [ax]
#pragma aux Cursor  "*_c" __far parm [ax] modify exact [ax]
#pragma aux Cursor1 "*_c" __far modify exact [ax]
#pragma aux ChgCur  "*_c" __far value [ax] modify exact [ax]
#pragma aux CurShp  "*_c" __far parm [cx] modify exact [ax]
#pragma aux GetCur  "*_c" __far value [ax] modify exact [ax]
#pragma aux HidCur  "*_c" __far modify exact [ax]
#pragma aux Int10m  "*_c" __far parm [ax] [bx] [cx] [dx] value [dx cx] modify exact [cx dx]
#pragma aux ScrOffs "*_c" __far value [di] modify exact [ax di]
#pragma aux SftEGA  "*_c" __far parm [bx] value [bx] modify exact [bx]
#pragma aux SavWin  "*_c" __far parm [bx] [dx] [si] modify exact [ax]
#pragma aux ResWin  "*_c" __far parm [bx] [dx] [si] modify exact [ax]
#pragma aux ResUser "*_c" __far parm [bx] [dx] modify exact [ax]
#pragma aux Window  "*_c" __far parm [bx] [dx] modify exact [ax]
#pragma aux SavScr  "*_c" __far modify exact [ax]
/* The frame of a box: see each in box.c. OncZoom takes a box of the
   assembly as DialBox does, in the main data. */
u16 *BoxLine(u16 *screen, u16 cell, u16 ends, u16 margin, u16 width);
u16 *BoxDubl(u16 *screen, u16 color, u16 rows, u16 margin, u16 width);
void MinBox(u16 begin, u16 size, u16 color);
u16 StdLine(u16 place, u16 cell, u16 ends, u16 size);
void OncZoom(u16 place, u16 box, u16 include, struct Box STACK *where);
#pragma aux BoxLine "*_c" __far parm [di] [ax] [bx] [cx] [dx] value [di] modify exact [di]
#pragma aux BoxDubl "*_c" __far parm [di] [ax] [si] [cx] [dx] value [di] modify exact [di]
#pragma aux MinBox  "*_c" __far parm [bx] [dx] [ax] modify exact [ax]
#pragma aux StdLine "*_c" __far parm [bx] [ax] [si] [dx] value [bx] modify exact [bx]
#pragma aux OncZoom "*_c" __far parm [bx] [ax] [dx] [di] modify exact [ax]
/* The screen blanker, which comes back when a key is pressed or the mouse
   moved, and the word to DOS and to what shares the machine that the
   program is idle: see each in blank.c. */
void Blank(void);
void LeaveTm(void);
#pragma aux Blank   "*_c" __far modify exact [ax]
#pragma aux LeaveTm "*_c" __far modify exact [ax]
/* Small ones: see each in misc.c. PgUp and PgDn return the cursor in the
   low word and the first line shown in the high one. */
u16 IsCrLf(u16 word);
u16 InvVar(u8 __far *variable);
u16 ChkSum(const u8 *data, u16 length);
u32 PgUp(u16 cursor, u16 first, u16 rows);
u32 PgDn(u16 cursor, u16 first, u16 rows, u16 lines);
#pragma aux IsCrLf  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux InvVar  "*_c" __far parm [es bx] value [ax] modify exact [ax]
#pragma aux ChkSum  "*_c" __far parm [si] [cx] value [ax] modify exact [ax]
#pragma aux PgUp    "*_c" __far parm [si] [di] [ax] value [dx ax] modify exact [ax dx]
#pragma aux PgDn    "*_c" __far parm [si] [di] [ax] [bx] value [dx ax] modify exact [ax dx]

/* The segment of the main data, as a number. */
#define MAIN_SEG ((u16)((u32)(void __far *)&md >> 16))

/* The panel that a file entry belongs to. */
#define WCB_OF(file) ((struct WCBdef __far *)((u32)(file) & 0xFFFF0000UL))

/* The panel in the segment `panel`: what the assembly has in DS where it
   says WCBseg. */
#define PANEL(panel) ((struct WCBdef __far *)((u32)(panel) << 16))

/* Where in a panel's segment its list of files begins, DskBuf, and where
   in the main data the list of the tree's directories, TreeBuf.TreList.
   The assembly keeps places in both as addresses, not as numbers. */
#define DSKBUF  (sizeof(struct WCBdef) - MinFls * sizeof(struct FILENT))
#define TRELIST ((u16)&((struct DIRTREE *)md.TreeBuf)->TreSum2)

/* The file entry at an address in a panel's segment. */
#define ENTRY(wcb, address) \
    ((struct FILENT __far *)((u8 __far *)(wcb) + (address)))

/* Bits of a file's attributes. The two highest are the program's own, in
   its lists of files. */
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_DIRECTORY 0x10
#define ATTR_SELECTED  0x40

/* Add to the low byte of a word, as the assembly does to a column or a
   count that it keeps in half a register: nothing is carried over. */
#define ADD_LOW(word, more) ((word) & 0xFF00 | (word) + (more) & 0xFF)

#endif
