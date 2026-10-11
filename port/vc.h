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
 *     Declare them CODE. String literals are not available.
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

/* Routines that are still assembly, for C code to call. Each has an entry
   in the assembly, the routine's name with _a, which is called far and
   calls the routine (see FromC in port/asm/VC.ASM). They take and return
   what the routine's registers hold, so a value in a high byte stays
   there; flags do not come back. */
u16 Color(u16 number);                  /* AH: a colour's number -> AH: it */
u16 UpCase(u16 letter);                 /* AL: a letter -> AL: its capital */
u16 TstHex(u16 digit);                  /* AL: a hex digit as text -> its
                                           value, or above 0FFh if none */
#pragma aux Color  "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux UpCase "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux TstHex "*_a" __far parm [ax] value [ax] modify exact [ax es]

/* The panel that a file entry belongs to. */
#define WCB_OF(file) ((struct WCBdef __far *)((u32)(file) & 0xFFFF0000UL))

/* Bits of a file's attributes. */
#define ATTR_DIRECTORY 0x10

#endif
