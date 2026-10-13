/*
 * Volkov Commander in C: what its parts share.
 *
 * The program was translated from assembly a few routines at a time, with
 * both halves running as one program all the while. So the C code lives
 * in the world the assembly built, which is not the one a C compiler
 * assumes:
 *
 *   - DS points at the main data segment, a block that the program
 *     allocates when it starts. It holds one struct MainDat and nothing
 *     else, so C code has no static data of its own.
 *   - Constants sit in the code segment, as they do in the assembly.
 *     Declare them CODE. String literals are not available, but to give
 *     such a constant its contents. The tables that the program names
 *     by their offsets, boxes and dialogs with their texts, are in files
 *     of their own: see tables.h.
 *   - The stack is in a third segment. A pointer to a local variable must
 *     be declared STACK.
 *   - Each file panel has a segment to itself, reached by far pointers.
 *
 * Routines keep the names they have in the assembly source, and data keeps
 * its layout: the structures below are those of the original's VC.ASM, in
 * src/asm/, and the only ones that the C translation has.
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

/* The main data segment, which DS points at: a block of the memory that
   the program asks DOS for, and so no object of the program's but a
   layout at offset 0 of DS. To the compiler it is an object like any
   other, of which it takes near addresses and makes far ones with DS.
   To the linker it has to be at offset 0 of something, and is named as
   what the program has there: Psp of resident.c, the room of the PSP.
   That rests on this compiler and its linker, which takes the offset
   of an object in the group that the object is of. A layout at the
   number 0 will not do, (*(struct MainDat __near *)0): the compiler
   makes a far pointer of an address that is a number with the segment
   0, not with DS, where a text of the main data is handed to a
   function that takes a far one. */
extern struct MainDat __near md;
#pragma aux md "Psp"

/* What the part of the program that stays in the memory while a command
   runs keeps there (resident.c), in the order of the original: the
   interrupts as they were when a program was last run, with what the
   BIOS keeps at 40h:A8h behind them; where INT 22h and INT 27h went
   before the program took them; where that part calls the rest of the
   program, a far address; the programs that have stayed in the memory,
   each by the segment of its record; the name of the program run last;
   and what is kept of the two panels and of the main data while a
   command runs: of a panel what a WCBdef has before ExtBuf and of each
   file of its list the first 13 bytes, of the main data what it has
   before DosLin. The setup file is read across the three, so they stand
   one behind the other.

   resident.c has the structure itself, and DS on it. The rest of the
   program reaches it through far pointers: the segment is that of the
   stack, and STAYS is the place of a member, of which a file takes the
   offset as of any object that it names. */
#define MaxProg 18                      /* Maximal resident quantaty */
#define STAYS_WCB \
    ((u16)(u32)&((struct WCBdef __far *)0)->ExtBuf + 13 * MinFls)
#define STAYS_CMN ((u16)&((struct MainDat *)0)->DosLin)

#pragma pack(push, 1)
struct Stays {
    u32 IntTab[256];
    u32 SavePtr;
    u32 Vect22;
    u32 Vect27;
    u32 TrnAdr;
    u16 Counter;
    u16 PrgList[MaxProg];
    u8  NameBuf[12];
    u8  WCB1sav[STAYS_WCB];
    u8  WCB2sav[STAYS_WCB];
    u8  ComnIni[STAYS_CMN];
};
#pragma pack(pop)

#ifndef RESIDENT
extern struct Stays __far Stays;
#pragma aux Stays "*"
#define STAYS(member) (&Stays.member)
#endif

/* Texts that the code of more than one file shows: the first two are
   constants of main_t.c, the third is one of treebox_t.c. They are in
   the group of the constants, and code reads them as far objects, so
   naming one costs a relocation. A file of constants, which is one that
   has tables.h and this by that, names them as lines of its boxes, by
   near words: there they are near. */
#ifdef TABLES_H
#define TEXT_FAR
#else
#define TEXT_FAR __far
#endif
extern const u8 TEXT_FAR Title_S[];     /* the program's name and version,
                                           behind two bytes of markup */
extern const u8 TEXT_FAR Var_S[];       /* a line that is the text at
                                           TmpErr+1 */
extern const u8 TEXT_FAR Tree_S[];      /* the title of the tree */

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

/* What goes through files, DirFunc, FncSel and FncMas, calls a routine
   for each of them: the one that FncAddr names. The assembly has the
   address of the routine there. The C has its number, one of these, in
   Fnc_T of dirfunc.c, a table of the functions: the word is still a
   word, and the main data is laid out as it was. FncCall calls the
   function, for a file with these attributes at TmpBuf: the high byte of
   what it returns is not 0 to stop. */
enum {
    FNC_C00_00,                         /* copy, rename or move: copy.c */
    FNC_DISI10,                         /* add up the sizes: read.c */
    FNC_FINDFIL,                        /* the file search: find.c */
    FNC_F38_40,                         /* delete: dirfunc.c */
    FNC_M30_50                          /* set attributes: dirfunc.c */
};
u16 FncCall(u16 attributes);
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
/* The start of the program, start.c. Init00 reads the setup, when the
   program has been loaded, and Init11 asks DOS for the main data, when it
   has, and after every command: `stays` is the segment of the part of
   the program that stays in the memory while a command runs, and `block`
   that of the block of memory that the rest is in. Init11 returns the
   segment of the main data, or 0 if there is too little memory, and then
   in the high word how many paragraphs there were. Neither has DS on the
   main data. IniData, which has, makes the main data of what that part
   has kept, and finds out about the machine. TrnMain of main.c is what
   the part that stays calls, with something that is not 0 when the
   program has just been loaded: it calls these and the main loop, and
   returns what that part is to do, in the low word: 100h to run the
   commands of DosBuf, 200h to say that there is no memory and leave,
   with the paragraphs in the high word, and less to leave, with that
   for DOS. See Go of resident.c. */
void Init00(u16 stays);
u32 Init11(u16 stays, u16 block);
void IniData(void);
u32 TrnMain(u16 first);
#pragma aux TrnMain "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
/* Read what a panel shows, keeping the cursor of a list of files and
   what is selected in it if `keep` is not 0: not 0 if it could not be
   read. See read.c. */
u16 ReDir(u16 panel, u16 keep);
/* The setup file: see each in setup.c. MakeIni writes into MenuBuf what
   the file holds, and returns how long that is. SaveSet writes it to the
   file: 1 in the high word if it could not, with what DOS answered in
   the low one, or 53h there for a drive that is not ready. */
u16 MakeIni(void);
u32 SaveSet(void);

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
   the second copy end otherwise, and resident.c calls those. That is all
   that the #pragma aux of each says here, the name that the linker has
   for it: two copies of one function cannot have one name. */
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
#pragma aux Int21d  DOS_NAME
#pragma aux Int21f  DOS_NAME
#pragma aux Int21e  DOS_NAME
#pragma aux Int10   DOS_NAME
#pragma aux TopView DOS_NAME
#pragma aux TopScr  DOS_NAME
#pragma aux Int16   DOS_NAME
#pragma aux Int2F   DOS_NAME
#pragma aux Int33   DOS_NAME
#pragma aux MousAt  DOS_NAME
#pragma aux CaseMap DOS_NAME
#pragma aux InPort  DOS_NAME
#pragma aux IntOff  DOS_NAME
#pragma aux IntOn   DOS_NAME
#pragma aux Int28   DOS_NAME
#pragma aux DvPause DOS_NAME

/* The registers as an interrupt function of the compiler saves them on
   the stack and puts them back: its first parameter. */
struct Saved {
    u16 gs, fs, es, ds, di, si, bp, sp, bx, dx, cx, ax;
};

/* A line as the original hands it over: the registers that describe
   it. */
struct LineArgs {
    u16 value;                          /* AX or BX: see each routine */
    u16 *screen;                        /* DI: where in the screen buffer */
    const u8 __far *line;               /* DS:SI */
    const u8 __far *colors;             /* CS:BP: the numbers of four colours */
    const u8 __far *include;            /* CS:DX */
};

/* What AdrNum hands back: the original has these in SI, DI, DX and
   AX. */
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

/* Routines for C code in other files than theirs to call. Each is
   called as the compiler calls any function: the C translation says of
   each, in a #pragma aux, in which registers the assembly called it, and
   the portable C does not. */
u16 Color(u16 number);                  /* screen.c */
u16 *BasAdr(u16 place);
u16 __far *MvsColr(u16 __far *screen, u16 count, u16 color);
u16 __far *MovDz0(u16 __far *screen, const u8 __far *text);
u16 __far *HexByt(u16 __far *screen, u16 byte);         /* digits.c */
u16 TstHex(u16 digit);
void Convert(const u8 __far *name, u8 *buffer);         /* mask.c */
u16 TstMas(const u8 __far *name, const u8 *mask);
void NgLine(struct LineArgs STACK *args);               /* line.c */
void CntLine(struct LineArgs STACK *args);
u8 __far *AddFile(u8 __far *end);                       /* string.c */
u8 __far *MvzLine(u8 __far *to, const u8 __far *from, u16 length);
u16 LowCase(u16 letter);
const u8 __far *CutPath(const u8 __far *path);
void PathLin(u8 __far *text);
void LetLin(u8 __far *text);
void ShrtLin(u8 __far *text, u16 limit);
u8 *Number(u8 *buffer, u16 high, u16 low, u16 mode);    /* format.c */
u16 *SzDtTm(const struct FILENT __far *file, u16 *screen, u16 color);
void SetMask(u16 panel);                                /* files.c */
void Sort(u16 panel);
u16 FnFile(u16 panel, const u8 __far *name);
u16 NearFl(u16 panel, const u8 __far *name, u16 end);
u16 BegBX(u16 panel);                                   /* place.c */
u16 SizDX(void);
u16 LnsAX(void);
u16 InvWCB(u16 panel);
u16 FilePan(u16 panel);
void AdrNum(u16 panel, struct Place STACK *place);
void CorPar(u16 panel);
u8 *PtPt10(u16 panel);                                  /* panel.c */
/* The tree in a panel, tree.c: its colours are four numbers in the code,
   which come as a far pointer. */
void PutTree(u16 panel, u16 place, u16 size, const u8 __far *colors);
void PutInf(u16 panel);                                 /* info.c */
/* Where a box is on the screen, as DialBox says it: what the assembly
   has in BX, DX and SI then. */
struct Box {
    u16 begin;                          /* the row in the high byte */
    u16 size;                           /* the rows in the high byte */
    u16 saved;                          /* where what it covers is kept */
};

/*
 * The commands.
 *
 * A command is what a key or an item of a menu does. In the original it
 * is no routine: the main loop jumps to it by the table of the keys and
 * the menu bar by the tables of the menus, with the current panel in DS,
 * and it ends by jumping back to one of several places of the loop, or on
 * into another command. Here a command is a function that returns where
 * the program goes on, and the loop, MainLoop of main.c, calls it from a
 * `switch` and goes on from what it returns. A function takes of what the
 * original has in its registers at the jump only what the command reads,
 * the panel as a rule.
 *
 * The commands are of four families, each with its own places to go on
 * at, which is how the original's files have them.
 */

/* The commands, by number: what the main loop finds for a key or for
   the mouse, and what the tables of the menus have for their items, with
   0 for an item that is none (src/c/commands_t.c). Each has the name of
   the label that the original has for it, which is how the test of the
   data holds the menus' tables against the original's: it reads the
   numbers here (harness/datacheck.py). Command of main.c has what each
   is. */
enum {
    CMD_F73_00 = 1,                     /* the key goes to the command
                                           line */
    CMD_F77_00,                         /* the speed search */
    CMD_F11_00,                         /* the panels off or on */
    CMD_F39_00,                         /* the menu bar */
    CMD_F20_00,                         /* the other panel */
    CMD_F34_10,                         /* the editor on dirinfo */
    CMD_F00_60,                         /* the cursor to an entry */
    CMD_F00_05,                         /* a line up */
    CMD_F01_05,                         /* a line down */
    CMD_F21_10,                         /* Enter on the entry */
    CMD_F21_31,                         /* run the command in DosBuf */
    /* The other commands of the keys, in the order that the original's
       table of the keys has them in: see Func12 of main.c. */
    CMD_F00_00, CMD_F01_00, CMD_F02_00, CMD_F03_00, CMD_F04_00,
    CMD_F05_00, CMD_F06_00, CMD_F07_00, CMD_F08_00, CMD_F09_00,
    CMD_F10_00, CMD_F76_00, CMD_F21_00, CMD_F27_00, CMD_F74_00,
    CMD_F78_00, CMD_F12_00, CMD_F13_00, CMD_F14_00, CMD_F15_00,
    CMD_F16_00, CMD_F17_00, CMD_M16_00, CMD_M16_10, CMD_M30_00,
    CMD_M49_00, CMD_F18_00, CMD_F79_00, CMD_F22_00, CMD_F23_00,
    CMD_F24_00, CMD_F25_00, CMD_F26_00, CMD_F80_00, CMD_F81_00,
    CMD_F82_00, CMD_F30_00, CMD_F32_00, CMD_F33_00, CMD_F34_00,
    CMD_F35_00, CMD_F36_00, CMD_F37_00, CMD_F38_00, CMD_F40_00,
    CMD_F42_00, CMD_F43_00, CMD_F44_00, CMD_F45_00, CMD_F46_00,
    CMD_F47_00, CMD_F48_00, CMD_M75_00, CMD_F54_00, CMD_F55_00,
    CMD_F56_00, CMD_F58_00, CMD_F61_00, CMD_F62_00, CMD_F63_00,
    CMD_F64_00, CMD_F65_00, CMD_M42_00, CMD_F67_00, CMD_F68_00,
    CMD_F69_00, CMD_M41_00, CMD_F75_00,
    /* Those that only the menus have, or have at a label of their own. */
    CMD_M01_00, CMD_M03_00, CMD_M06_00, CMD_M08_00, CMD_F17_01,
    CMD_M16_01, CMD_F62_01, CMD_M21_00, CMD_F76_01, CMD_M53_00,
    CMD_M60_00, CMD_M61_00, CMD_M62_00, CMD_M63_00, CMD_M64_00,
    CMD_M66_00, CMD_M67_00, CMD_M69_00, CMD_M70_00, CMD_M71_00,
    CMD_M72_00, CMD_M73_00
};

/* Where the program goes on after a command of the keys of the panels,
   of keys.c, enter.c and view.c, in the low byte of what it returns. The
   routines of a panel's menu take a panel, in the high word, and a byte,
   the high byte of the low word: for M03_00 five less the kind of panel,
   and for M06_00 the place in the menu bar that is lit meanwhile. */
enum {
    NEXT_FUNC07,                        /* wait for the next key */
    NEXT_FUNC05,                        /* first save the setup, if it has
                                           changed and is saved by itself */
    NEXT_FUNC04,                        /* first draw the menu bar */
    NEXT_FUNC01,                        /* first draw the current panel
                                           and put the screen buffer on
                                           the screen */
    NEXT_INPUT,                         /* F73_00: the key goes to the
                                           command line */
    NEXT_EARLIER,                       /* F25_00: the command before in
                                           the history */
    NEXT_LATER,                         /* F26_00: the one after */
    NEXT_INIT40,                        /* find the command line again
                                           and draw everything */
    NEXT_INIT50,                        /* draw the panels again, reading
                                           those that have changed */
    NEXT_M01_01,                        /* the menu's routine that makes a
                                           panel one of files, in the order
                                           and the form it has */
    NEXT_M03_00,                        /* that makes one the tree or the
                                           Info */
    NEXT_M06_00,                        /* that switches one off or on */
    NEXT_POP_FUNC04,                    /* FUNC04 and FUNC05, where the
                                           original takes off its stack the
                                           panel that it has put there for
                                           the menu's routines */
    NEXT_POP_FUNC05,
    NEXT_INIT11,                        /* start over from what F21_30 has
                                           saved in the part of the program
                                           that stays in the memory */
    NEXT_EXEC,                          /* and first have that part run
                                           the commands saved there */
    NEXT_VIEW,                          /* V00_00: the viewer on the file
                                           ViewFil, or with 1 in the high
                                           word the editor */
    NEXT_DIRINFO,                       /* V00_02: the editor in the window
                                           of an Info panel */
    NEXT_EXIT                           /* leave the program */
};

/* After a command of the menus, of commands.c. */
enum {
    MNU_FUNC04,                         /* back to the panels as they are:
                                           the menu bar is drawn first */
    MNU_INIT50,                         /* draw the panels again, reading
                                           those that have changed */
    MNU_F24_01,                         /* have DOS go to the directory at
                                           TmpBuf, and read the panels */
    MNU_V00_01                          /* edit the file ViewFil, with the
                                           menu marked that the command
                                           came from */
};

/* After a command of dirfunc.c, and after the file search. */
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

/* After Copy and Rename, of copy.c, in the low word of what they return.
   With GO_MASKED the high word is the title of the boxes. */
#define GO_KEYS     0                   /* Func04: the next key */
#define GO_PANELS   1                   /* Init50: the panels drawn anew */
#define GO_SELECTED 2                   /* FncSel: C00_00 for every file
                                           that is selected */
#define GO_MASKED   3                   /* FncMas: for those of a mask */
#define GO_DONE     4                   /* FncExt: the panels drawn anew
                                           if something was written */

/* The registers that go round the main loop: what a command finds in
   them, as the loop and the command before have left them, and what it
   leaves there in its turn. A command is handed what it reads of them.
   One that leaves something which another may read gets them all, to
   put it there: see main.c for what is read. */
struct Regs {
    u16 ax, bx, cx, dx, si, di, bp;
    u16 ds;                             /* the current panel */
};

/* The keys of the panels, keys.c: `panel` is the current panel. F00_60,
   F00_05 and F01_05 are where the mouse comes in, with the numbers of the
   cursor's entry, of the first one shown and of the last one there is,
   and with its buttons; F76_01 and F17_01 where the menus do, F17_01 with
   the panel of the menu. The keys of the cursor leave BX and DX in `r`
   as the original has them when it goes back to the loop: F00_50, which
   moves the cursor and which the speed search calls too, returns what it
   leaves in the two, BX in the low word. */
u32 F00_50(u16 panel, u16 cursor, u16 first);
u32 F00_60(u16 panel, u16 cursor, u16 first, u16 buttons,
           struct Regs STACK *r);
u32 F00_00(u16 panel, struct Regs STACK *r);
u32 F00_05(u16 panel, u16 cursor, u16 first, u16 buttons,
           struct Regs STACK *r);
u32 F01_00(u16 panel, struct Regs STACK *r);
u32 F01_05(u16 panel, u16 cursor, u16 first, u16 last, u16 buttons,
           struct Regs STACK *r);
u32 F02_00(u16 panel, struct Regs STACK *r);
u32 F03_00(u16 panel, struct Regs STACK *r);
u32 F04_00(u16 panel, struct Regs STACK *r);
u32 F05_00(u16 panel, struct Regs STACK *r);
u32 F06_00(u16 panel, struct Regs STACK *r);
u32 F07_00(u16 panel, struct Regs STACK *r);
u32 F08_00(u16 panel, struct Regs STACK *r);
u32 F76_00(u16 panel);
u32 F76_01(u16 panel);
u32 F09_00(u16 panel);
u32 F10_00(u16 panel);
u32 F78_00(u16 panel);
u32 F11_00(u16 panel);
u32 F12_00(u16 panel);
u32 F13_00(void);
u32 F14_00(void);
u32 F15_00(u16 panel);
u32 F16_00(void);
u32 F17_01(u16 panel);
u32 F79_00(u16 panel);
u32 F18_00(u16 panel);
u32 F20_00(u16 panel);

/* Enter, the directory changed and the history, enter.c. F21_10 is Enter
   on the entry under the cursor, F21_25 a command of the command line
   run, F21_31 the program started over to run what DosBuf holds; F24_01
   goes to the directory whose path is at `path` in the main data, and
   F24_02 has the current panel show the directory that DOS is in. */
u32 F21_00(u16 panel);
u32 F21_10(u16 panel);
u32 F21_25(void);
u32 F21_31(void);
u32 F22_00(u16 panel);
u32 F23_00(u16 panel);
u32 F24_00(u16 panel);
u32 F24_01(u16 panel, u16 path);
u32 F24_02(u16 panel);
u32 F25_00(void);
u32 F26_00(void);
u32 F27_00(u16 panel);
u32 F30_00(u16 panel);
u32 F75_00(u16 panel);

/* The viewer and the editor, view.c and edit.c: the commands, and what
   they go on in. V00_02 opens the file ViewFil and says in `open` what it
   found, 0 if there is nothing to show, 1 for the editor and 2 for the
   viewer; T00_00 is the editor, which returns 0 if the file is to be
   viewed instead, 1 if it is to be closed and nothing more, and 2 when
   it has been edited and is closed, with what V13_01 takes in the high
   byte: not 0 if a file was written. V00_10 is the viewer; V13_00 closes
   the file and goes back to the panels, as V13_01 does without closing
   one. See each there. */
struct Open {
    u16 handle;                         /* 0FFFFh for a new file */
    struct Box box;                     /* where the box "Reading the
                                           file" is */
};
u32 F43_00(void);
u32 F44_00(void);
u32 F63_00(u16 panel);
u32 F64_00(u16 panel);
u32 F33_00(u16 panel);
u32 F34_00(u16 panel);
u32 F34_10(u16 panel);
u16 V00_02(u16 menu, u16 edit, u16 whole, struct Open STACK *open);
void V00_10(u16 handle);
void V13_00(void);
void V13_01(u16 changed);
u16 T00_00(u16 handle, u16 begin, u16 size, u16 saved);

/* The commands of the menus, commands.c: `panel` is the current panel,
   or the panel of the Left or the Right menu, and `item` what the
   original has in CX, from the menu bar the menu in the low byte and the
   item in the high one. M01_01, M03_00, M06_00 and M06_10 are where the
   keys of the panels and the box of the drives go on. */
u16 M01_00(u16 panel, u16 item);
u16 M01_01(u16 panel, u16 item);
u16 M03_00(u16 panel, u16 item);
u16 M06_00(u16 panel, u16 item);
u16 M06_10(u16 panel, u16 item);
u16 M08_00(u16 panel, u16 item);
u16 M16_00(u16 panel);
u16 M16_01(u16 panel);
u16 M16_10(u16 panel);
u16 M21_00(void);
u16 M41_00(void);
u16 M42_00(u16 panel);
u16 F69_00(u16 item);
u16 M49_00(u16 panel);
u16 M60_00(void);
u16 M61_00(void);
u16 M53_00(u16 item);
u16 M62_00(u16 item);
u16 M63_00(u16 item);
u16 M64_00(u16 item);
u16 M66_00(u16 panel);
u16 M67_00(u16 panel);
u16 M69_00(u16 panel);
u16 M70_00(u16 panel);
u16 M71_00(u16 panel);
u16 M72_00(u16 panel);
u16 M73_00(u16 panel);
u16 M75_00(u16 panel);

/* Directories made, files deleted, attributes and the label, dirfunc.c;
   and where a command ends that goes through files: FncSel for those
   that are selected, FncMas for those of a mask, with the title of its
   boxes, and FncExt, which like FncSel returns 1 if the panels are to be
   read again. */
u16 F37_00(u16 panel);
u16 F47_00(void);
u16 F38_00(u16 panel);
u16 F48_00(void);
u16 F58_00(u16 panel);
u16 M30_00(u16 panel);
u16 F54_00(void);
u16 FncMas(u16 include);
u16 FncSel(void);
u16 FncExt(void);

/* Copy and Rename, copy.c: `command` is 1 to rename, and `directories`
   and `files` are what the original has in BX and DX at the jump. Where
   the command does not count what is selected, it goes by those two as
   they come: see F35_50 there. */
u32 F35_01(u16 panel, u16 command, u16 directories, u16 files);
u32 F45_01(u16 panel, u16 command, u16 directories, u16 files);

/* The file search, find.c, which returns a place of dirfunc.c's and in
   the high word, with TO_F24_02, the panel; the map of the memory,
   memory.c; the command line, cmdline.c: a key for it, Esc, the selected
   files and the path of a panel put into it, the history in its box, and
   the speed search, which returns the key that ended it and in the high
   word where its box was; the user menu, usermenu.c, 1 for the main one;
   the box of the drives, drive.c; and the menu bar, menu.c. See each
   there. */
u32 F67_00(void);
void F65_00(void);
void F73_00(u16 key);
u16 F74_00(void);
u16 F80_00(u16 panel);
void F82_01(u16 panel, u16 key);
u16 F68_00(void);
u32 F77_00(u16 key, u16 panel);
u16 F32_00(u16 main);
u16 F62_01(u16 panel, u16 item);
u32 F39_00(u16 again, u16 panel);

/* Save the setup, the command line and the panels in the part of the
   program that stays in the memory, and give up the main data, for that
   part to run what the command line holds: returns NEXT_EXEC, or
   NEXT_INIT11 if there is nothing to run, for the command to return in
   its turn. See
   enter.c. */
u32 F21_30(void);
/* The commands of a line of an extension file or a menu file, in
   MenuBuf: F34_30 looks for the next one from `text`, and F34_40 at
   whether the command at `text` is only the name of a file, a program
   for `view` and else a menu, whose path it has put at TmpBuf then. Both
   return a place in the high word, and in the low one 1 if there is no
   command, or if it is such a name. See view.c. */
u32 F34_30(u16 text);
u32 F34_40(u16 text, u16 view);
/* Have both panels read again: where a command ends that has changed
   what is on the disk. Returns 1. See dirfunc.c. */
u16 FncEx1(void);

/* Calls what FncAddr names for every file of the directory at TmpBuf, and
   returns 1 if every call did what it was to, 0 if one did not, and 100h
   when it was stopped. */
u16 DirFunc(void);                                      /* dirfunc.c */
/* Marks the tree's file of the drive that `path` names, or of the current
   one, as out of date. Returns what DOS answered in the low word, and
   something that is not 0 in the high one for an error. */
u32 DelTr0(const u8 *path);

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
/* The key bar and the menu bar, the command line and its prompt: see
   each in bars.c. `segment` is where ClsLine reads the colour, MAIN_SEG. */
u16 *ClsLine(u16 place, u16 segment);
void PutKey(u16 bar);
void PutKey0(u16 bar, u16 shift);
void PutMnu(u16 item);
void PtMnu1(u16 item);
void PutPrm(u16 panel);
void DosCom(void);
/* The entries of an extension file or a menu file: FndItem returns where
   the next one is, the offset in the low word, and 1 in the high one at
   the end of the file; FindExt where in the main data the command for a
   file's extension is, or 0. See extfile.c. */
u32 FndItem(const u8 __far *file);
u16 FindExt(const u8 __far *name);
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
/* The frame of a box: see each in box.c. OncZoom takes a box of the
   assembly as DialBox does, in the main data. */
u16 *BoxLine(u16 *screen, u16 cell, u16 ends, u16 margin, u16 width);
u16 *BoxDubl(u16 *screen, u16 color, u16 rows, u16 margin, u16 width);
void MinBox(u16 begin, u16 size, u16 color);
u16 StdLine(u16 place, u16 cell, u16 ends, u16 size);
void OncZoom(u16 place, u16 box, u16 include, struct Box STACK *where);
/* The screen blanker, which comes back when a key is pressed or the mouse
   moved, and the word to DOS and to what shares the machine that the
   program is idle: see each in blank.c. */
void Blank(void);
void LeaveTm(void);
/* Small ones: see each in misc.c. PgUp and PgDn return the cursor in the
   low word and the first line shown in the high one. AsmSeg is the
   segment of the program's constants, the texts and tables of the files
   whose names end in _t, where what is known by its offset is read;
   ResSeg that of the part of the program that stays in the memory while
   a command runs, where the stack is; and MemRun that of the part that
   goes then, which this code is of. None of the three needs DS. */
u16 AsmSeg(void);
u16 ResSeg(void);
u16 MemRun(void);
u16 IsCrLf(u16 word);
u16 InvVar(u8 __far *variable);
u16 ChkSum(const u8 *data, u16 length);
u32 PgUp(u16 cursor, u16 first, u16 rows);
u32 PgDn(u16 cursor, u16 first, u16 rows, u16 lines);

/* The segment of the main data, as a number: this compiler makes a far
   pointer of a near one with DS, and that is not read anywhere else. */
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
