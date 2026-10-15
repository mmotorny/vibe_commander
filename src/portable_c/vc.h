/*
 * Volkov Commander in C: what its parts share.
 *
 * The program was translated from assembly a few routines at a time, and
 * the portable C began as a copy of that translation. So the C code still
 * has much of the world that the assembly built, which is not the one a C
 * compiler assumes:
 *
 *   - A place of the memory is kept in numbers of 16 bits wherever the
 *     assembly kept it in registers: a segment, an offset in one. The
 *     program is one of 32 bits, with all of its memory at one address
 *     after another and no segments: the words of the addresses, at the
 *     end of this file, make a pointer of such numbers and the numbers
 *     of a pointer, and say what they take for granted.
 *   - The main data is one struct MainDat, md, and a place in it is
 *     kept as its offset in the structure, as the original has it.
 *   - The tables that the program names by their offsets, boxes and
 *     dialogs with their texts, are in files of their own: see
 *     tables.h.
 *   - Each file panel has a block of the memory to itself, which the
 *     program asks the system for, and is a pointer to it; a place in
 *     its list is kept as an offset from where the block begins.
 *
 * The original has the main data in a block that it asks DOS for and
 * gives back while a command runs; here it is of the program, which
 * stays in the memory whole (see DEVIATIONS.md).
 *
 * Routines keep the names they have in the assembly source, and data keeps
 * its layout: the structures below are those of the original's VC.ASM, in
 * src/asm/, and the only ones that the program has.
 */
#ifndef VC_H
#define VC_H

/* What each compiler says in its own way: NO_RETURN, SEGMENT_BEGIN and
   SEGMENT_END, NONSTRING. */
#include "compiler.h"

/* Numbers of 8, 16 and 32 bits. An int has 32 bits for every compiler
   that reads this, Open Watcom's for the 386 and the gcc and clang of
   the checks, and a long may have 64. The lines below hold each to its
   length. */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/* A number of 32 bits with a sign, the one such type: for a number that
   may be below 0 and is added to a place or to a pointer, a step back.
   One without a sign that has wrapped round for that, FF60h for 160
   back, is right only where what it is added to wraps round with it,
   which a pointer does not. */
typedef int            i32;

/* A byte with a sign made a word. Where the original has `CBW`, a byte
   taken with its sign for a number of 16 bits that then counts without
   one, the code does not write the register. The byte is nearly always
   a small number, a row, a column, a count of lines, for which `CBW`
   was the assembly's way to clear AH: it is taken as it is, into an i32
   where it is reckoned with. Where its sign is meant, it is `(signed
   char)byte`, made an int. No word of 16 bits without a sign stands
   between the byte and its use, and the form

       (u16)(signed char)byte

   is not written. C says FFFEh of it for the byte FEh. Open Watcom,
   with every option but -od, loses the cut wherever the number goes on
   into 32 bits, handed to a parameter of 32 bits, kept in a variable of
   32 bits, added to, compared, divided, shifted or made an index: it
   extends the byte with its sign to all 32 bits, `movsx eax,al`,
   FFFFFFFEh. It does so for any number with a sign that is converted to
   a wider type without one and from there to a still wider type, with
   the cast written or not; the byte made a word is the one such case
   that a program without numbers of 64 bits has. The tree had the form
   on 33 lines, with a `(short)` between, `(u16)(short)(signed
   char)byte`, which it keeps, and has neither since. Every other cast
   that narrows, of 32 or 16 bits to a u16 or a u8, it keeps. The test
   `portable_c-cut` fails for a signed char made a u16 anywhere in the
   tree, which is what keeps the form out, and holds the compiler to
   what it makes of each form: harness/cut.py. */

/* True or false and nothing else: bool, with true and false, as C has
   them since 1999, from the compiler's own header, which is no part of
   a library. A number made one is 1 unless it is 0, and one byte in a
   structure. A typedef of the program's own in their place would stop
   a compiler whose C has bool as a word of the language. */
#include <stdbool.h>

/* Fails to compile when something that the code counts on is not so:
   the name says what. It is the tag of a structure and no typedef, as
   LAID_OUT below is: a typedef more can change the order of two
   variables in the program that Open Watcom makes, which a tag does
   not (see "Notes on the portable C" in CLAUDE.md). */
#define HOLDS(name, truth) struct name { char holds[(truth) ? 1 : -1]; }

HOLDS(u8_has_8_bits, sizeof(u8) == 1 && (u8)-1 == 0xFF);
HOLDS(u16_has_16_bits, sizeof(u16) == 2);
HOLDS(u32_has_32_bits, sizeof(u32) == 4);
HOLDS(i32_has_32_bits_and_a_sign, sizeof(i32) == 4 && (i32)-1 < 0);
HOLDS(a_bool_is_one_byte, sizeof(bool) == 1);

/* The low byte of a number is the first of it in the memory, as it is
   on DOS, and the code counts on that all over. The main data, DOS's
   structures, the screen and the files that the program keeps hold
   words, which it reads and writes as numbers through the structures
   below, and copies as bytes: the setup file is the bytes of the main
   data and of the panels, the help file and the tree's file are read
   into structures, and the character of a cell of the screen is
   written as its first byte. A word in a buffer of bytes is made of
   its bytes, by GET16 and its like below, and does not count on it.
   So nothing compiles for a machine that is not known to have it so. The
   one that Open Watcom compiles for here has, the 386; gcc and clang
   say which it is. Of another compiler nothing is known here: say how
   it tells. */
#if !defined(__386__) \
    && !(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#error The code takes the low byte of a number to be the first: see vc.h
#endif

/* Where in a structure a field is, as a number: offsetof of <stddef.h>,
   which with <stdbool.h> above is what the program has of a compiler's
   headers. */
#include <stddef.h>
#define OFFSET_IN(type, field) ((u16)offsetof(type, field))

/* A word of 16 or 32 bits in a buffer of bytes, read and written as its
   bytes, the low one first, as DOS has it. The program keeps such words
   where the original does, in buffers and in files, at an address that
   may be odd, and reads them so, not through a pointer of the word's
   type cast from the byte's: C does not promise such a read where the
   address is not aligned, and a sanitizer's check of alignment stops at
   one. Open Watcom puts the few instructions in place. */
static inline u16 GET16(const u8 *at)
{
    return (u16)(at[0] | at[1] << 8);
}

static inline u32 GET32(const u8 *at)
{
    return (u32)GET16(at) | (u32)GET16(at + 2) << 16;
}

static inline void PUT16(u8 *at, u16 value)
{
    at[0] = (u8)value;                  /* the low byte, */
    at[1] = (u8)(value >> 8);           /* and the high one */
}

static inline void PUT32(u8 *at, u32 value)
{
    PUT16(at, (u16)value);              /* the low word, */
    PUT16(at + 2, (u16)(value >> 16));  /* and the high one */
}

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
/* Stack size: the original has 100h, and the C translation 1000h. A
   program of 32 bits has four times that: it pushes a word of four
   bytes where one of 16 bits pushes one of two, and keeps what it works
   with in such words. */
#define LenStk  0x4000
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
    u8  LoadFl;                         /* Small mode: of the original.
                                           Here a byte of the setup that
                                           is kept as it is found, but
                                           for its highest bit, which
                                           says for one command that it
                                           is not to be shown */
    u8  UnSlct;                         /* Unselect files when group operation */
    u8  ErrSnd;                         /* Beep when error */
    u8  LftMous;                        /* Left handed mouse */
    u8  ZoomWin;                        /* Zomming windows */
    u8  AltBar;                         /* Alt alone selects menu */
    u8  EscBar;                         /* Esc toggles panels on/off */
    u8  ExecTyp;                        /* Exec command without
                                           COMMAND.COM, Quick execute
                                           commands: of the original.
                                           Here a byte of the setup that
                                           is kept as it is found */
    u8  AutoSiz;                        /* Auto calculate directory sizes */
    u8  HidDirs;                        /* Show hidden subdirs in tree panel */
    u8  QuitAck;                        /* Confirmation on Quit */
    u8  SnowMod;                        /* Snow mode (2=auto): of the
                                           original. Here a byte of the
                                           setup that is kept as it is
                                           found */
    u8  AltView;                        /* Alt-F3 - external viewer */
    u8  AltEdit;                        /* Alt-F4 - external editor */
    u8  ClrBuf;                         /* Clear buffer flag */
    u8  Active;                         /* Active panel */
    u8  Ctrl_O;                         /* Visible status of unactive panel */
    u8  ChkTSR;                         /* TSR manager: of the original.
                                           Here a byte of the setup that
                                           is kept as it is found */
    u8  ChgTree;                        /* Auto rescan tree */
    u8  DriveB;                         /* Show logic drive B: in 1 floppy system */
    u8  DelayTm[2];                     /* Auto change dir delay time */
    u8  StarTab[5];                     /* Star table */
    u8  ResIni[51];                     /* Reserved */
    /* End of main data' part saved in VC.INI */

    u8  LoadRes;                        /* Original LoadFl: as the setup
                                           has it, while a command has
                                           it otherwise */
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
    u16 BatAdr;                         /* Of the original: where in
                                           DosBuf the command to run next
                                           is. Not set or read here: see
                                           NextCommand */
    u16 BatPoin;                        /* Pointer to command, kept
                                           while the shell ran another:
                                           not set here */
    u16 LastMnu;                        /* The last pull down menu item */
    u16 OvlPth;                         /* Pointer to file name in
                                           environment: not set here */
    u16 ComSpec;                        /* Of the original: where the
                                           shell's name is in the
                                           environment. Not set or read
                                           here: see ShellPlace */
    u8  CrnPath[LenPath];               /* Current directory in the tree panel */
    u8  ViewFil[LenStr + 1];            /* Source filename */
    u8  ToFile[LenStr + 1];             /* Target filename */
    u8  FindStr[LenStr + 1];            /* FileFind wildcard */
    u8  SrchStr[LenStr + 1];            /* String for search */
    u8  DosChg[5];                      /* Reserved for change drive or
                                           /C_: here for the second
                                           alone, the count and /C_ in
                                           front of the first command,
                                           its last four bytes */
    u8  DosBuf[LenBat];                 /* Command line */
    /* End of main data' part saved in memory */

    u8  DosLin;                         /* Line number of DOS's command line */
    u8  DosCol;                         /* Current column of DOS's command line */
    u8  BegCol;                         /* Start position of DOS's command line */
    u8  OutCol;                         /* Start position for output com. line */
    u8  ScrPage;                        /* Current screen page number */
    u8  TstEGA;                         /* 1 if EGA or VGA present: the
                                           start asks such a display
                                           for its rows */
    u8  Lines;                          /* Number of lines on screen */
    u8  Snow;                           /* Snow on/off: of the original.
                                           Not set or read here */
    u8  TopView;                        /* TopView output: of the
                                           original. Not set or read
                                           here */
    u8  DesqVie;                        /* Running under DESQVIEW: of
                                           the original. Not set or read
                                           here */
    u8  WinLeav;                        /* Leave time under Windows: of
                                           the original. Not set or read
                                           here */
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
    u16 WCBcrn;                         /* Of the original: the segments
                                           of the current panel, */
    u16 WCBsec;                         /* of the other one, */
    u16 WCB1seg;                        /* of the left one */
    u16 WCB2seg;                        /* and of the right one. Not set
                                           or read here: see
                                           CurrentPanel */
    u16 ViewSeg;                        /* Of the original: the segment
                                           behind the panels. Not set or
                                           read here: see Room */
    u16 MemSeg;                         /* Not set or read here. The
                                           original has a number for the
                                           Info here, FreMem1 */
    u16 FreMem2;                        /* Of the original: the
                                           paragraphs of its block of
                                           memory. Not set or read here:
                                           see RoomEnd */
    u16 Memorys;                        /* The memory that the machine
                                           has, in paragraphs, for the
                                           Info: of the original. Here a
                                           word that nothing sets or
                                           reads, which keeps the place
                                           of what is behind it */
    u16 HstAdr;                         /* Of the original: places of
                                           the main data, in the history, */
    u16 TreAdr;                         /* of the tree's cursor, */
    u16 TreFrst;                        /* of its first line shown */
    u16 TreCrnt;                        /* and of the current directory
                                           in it. Not set or read here:
                                           see HistoryPlace and
                                           TreeCursor */
    u16 ClstSiz;                        /* Of the original: the bytes of
                                           a cluster, */
    u16 Total;                          /* the clusters of the drive */
    u16 FreeDsk;                        /* and those that are free, for
                                           the Info. Not set or read here:
                                           see ClusterBytes */
    u16 InfoLen;
    u16 Rand1;
    u16 Rand2;
    u16 FncAddr;
    u16 MousPos;                        /* Of the original: the line that
                                           a first click was on. Not set
                                           or read here: see ClickLine */
    u32 MousTim;
    u32 AutoTim;
    u32 Screen;                         /* Address of screen */
    u32 ErrVct;                         /* Of the original: who had the
                                           critical error, INT 24h, */
    u32 BrkVct;                         /* and who had Ctrl-Break, INT
                                           1Bh. Not set or read here:
                                           sys_dos.c keeps them, as
                                           addresses of protected mode */
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

/* Window control block: a file panel, in a block of its own. */
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

    /* Of the original: CurAdr, First and EndBuf, places in the panel
       as offsets of 16 bits. Not set or read here: see struct Places. */
    u16 CurAdr16;
    u16 First16;
    u16 EndBuf16;
    /* End of WCB part saved in memory */

    u8  ExtBuf[LenExt];
    u16 EndFul16;                       /* of the original: EndFul. Not
                                           set or read here */
    u8  ChgFlg;
    u8  SizDir;
    /* End of WCB */

    struct FILENT DskBuf[MinFls];
};

/* What a panel's block has in front of the panel, PLACES: how long the
   block is, and the places that the panel keeps of its list, each an
   offset from where the panel begins, as ENTRY takes it: the entry that
   the cursor is on, the first one shown, the end of the list as it is
   shown, which a filter makes shorter, and the end of all that was
   read. The original has the places in the panel, CurAdr, First, EndBuf
   and EndFul, in 16 bits, which is as far as its panel reaches; here a
   panel's block grows as its directory needs, and they are 32 bits.
   The panel keeps the original's words, which nothing sets or reads.
   The places are the last paragraph in front of the panel, and the
   length is in the paragraph before them, so that the panel begins on
   a paragraph as its block does. */
struct Places {
    i32 Bytes;                          /* how long the block is, the
                                           panel's and its list's own
                                           and these 32 */
    i32 Unused[3];
    i32 CurAdr;
    i32 First;
    i32 EndBuf;
    i32 EndFul;
};

#pragma pack(pop)

LAID_OUT(CNTRY, 0x20);
LAID_OUT(DTAs, 0x2C);
LAID_OUT(FILENT, 0x18);
LAID_OUT(MainDat, 0xF48B);
LAID_OUT(WCBdef, 0x1A62);
LAID_OUT(Places, 0x20);

/* The main data, which is the first thing of the segment that DS points
   at: main.c has the object, and says how it comes to be first. Much
   of the program takes it to be at offset 0 there: the tables name a
   place in it by the offset in the structure. */
extern struct MainDat md;

/* The panels, of main.c: the current one and the other one, the left
   one and the right one, each a pointer to the panel's block. They are
   WCBcrn, WCBsec, WCB1seg and WCB2seg of the original, which has the
   segment of each in the main data; a pointer is longer than such a
   word, so they are beside the main data. A panel's block is taken by
   Init11 of start.c and never given back, and grows and shrinks as its
   directory needs while ReDir of read.c reads it, which may move it:
   these four are then made to point where it is (PanelGrow and
   PanelFit of start.c), and nothing else keeps a pointer to a panel or
   into its list across ReDir. The places that a panel keeps of its list,
   CurAdr, First, EndBuf and EndFul, are offsets from where it begins,
   in front of it: see struct Places. */
extern struct WCBdef *CurrentPanel;
extern struct WCBdef *OtherPanel;
extern struct WCBdef *LeftPanel;
extern struct WCBdef *RightPanel;

/* The main data where a panel is asked for. The original comes to
   three routines that go by the panel in DS with DS left on the main
   data, and what they then read of a panel they read of the main data:
   whether it is the left panel, which it is not, in BegBX of place.c,
   for PutPrm of bars.c and for the main loop; its third byte for
   Visible, in M60_02 of commands.c; and its byte 23 for WinPath[3], in
   ReDr00 of read.c. Both bytes are of ColTab, colours of the first
   palette. Nothing else of the main data is read so, and nothing is
   written: whoever hands this to a routine has read what the routine
   reads of a panel. */
#define MAIN_AS_PANEL ((struct WCBdef *)&md)

/* A panel where the main data is asked for: the other way round, and
   one place. Run_00 of usermenu.c comes to ClsLine with DS left on the
   current panel when the commands have no room for a name, and ClsLine
   reads the colour that DOS writes in, CrntCol, through DS: the byte of
   the panel that is as far into it as CrntCol is into the main data,
   AB8h, which is within struct WCBdef. Nothing else of a panel is
   read so, and nothing is written. */
#define PANEL_AS_MAIN(panel) ((const struct MainDat *)(panel))

/* The room, of main.c: the memory that whatever is at work has, the
   text of the viewer or the editor, the list of the file search, a
   copy's buffer, the old list while a directory is read. It is a block
   of its own, behind the blocks of the two panels, and Room is where it
   begins and RoomEnd where it ends: ViewSeg and FreMem2 of the
   original, which has a segment for the first and a count of paragraphs
   for the second, in the main data, and which has the room begin where
   the second panel ends, wherever that is. The room is given back for
   a command and taken again, and may then be elsewhere and of another
   length: nothing keeps a pointer into it across GiveMem, TakeMem or
   Aexec0 of view.c, nor across ReDir of read.c, which makes it shorter
   for a panel that grows and takes it again when it has read the
   directory (PanelGrow and RoomAgain of start.c). RoomBytes, of
   start.c, says how long it is, in bytes. */
extern u8 *Room;
extern u8 *RoomEnd;
i32 RoomBytes(void);

/* Places of the main data that the program keeps, of main.c: each is a
   word of the main data in the original, the place as a number, and a
   pointer here, beside the main data since a pointer is longer than
   the word. The main data does not move, so they hold for as long as
   the program runs.

   TreeCursor is the directory of the tree that the cursor is on,
   TreeFirst the first one shown, and TreeCurrent the current directory
   where PutTree has just written it, or 0 if it is not among the lines
   written: TreAdr, TreFrst and TreCrnt of the original. They are
   entries of the tree's list, TRELIST; the first two are set when a
   panel that shows the tree is set up (Init92 of main.c) and when the
   tree is read (TreeCur of tree.c), and are 0 until then. For a moment
   ChTree of treebox.c has both two entries further on than they are,
   which may be behind the list's end: nothing is read there.

   HistoryPlace is where in the history, HstBuf, Ctrl+E and Ctrl+X
   are: HstAdr. NextCommand is the command of DosBuf that Exec02 of
   exec.c runs next: BatAdr. */
extern struct DIRENT *TreeCursor;
extern struct DIRENT *TreeFirst;
extern struct DIRENT *TreeCurrent;
extern const u8 *HistoryPlace;
extern u8 *NextCommand;

/* Numbers of the main data that are wider here, of main.c: each is a
   word of the main data in the original, which is too narrow for what
   it counts, and is beside the main data since its layout is the
   original's.

   ShellPlace is where the shell's name is in the environment, behind
   "COMSPEC=", counted from the environment's start, or 0 if the
   environment names none: ComSpec. An environment of another system
   may be longer than 64K.

   ClusterBytes is the bytes of a cluster of the drive that the Info
   panel shows, DiskClusters the clusters of the drive and FreeClusters
   those that are free: ClstSiz, Total and FreeDsk, which the Info sets
   when it reads the drive (RdInfo of read.c) and multiplies into the
   bytes of the disk. They are the factors of the size of a disk, and
   are without a sign as that is. The original has the bytes of a
   cluster in 16 bits, where a cluster of 64K is 0, and the Info then
   says that the disk has no bytes: a bug of the original, fixed on the
   user's word of 14 October 2026 (DEVIATIONS.md, 26).

   ClickLine is the line of a list that a first click of the mouse was
   on, for DoublMo of mouse.c to know a second one by: MousPos. */
extern i32 ShellPlace;
extern u32 ClusterBytes;
extern u32 DiskClusters;
extern u32 FreeClusters;
extern i32 ClickLine;

/* A panel's block, of start.c: what it has in front of the panel,
   struct Places, the panel and its list. The list ends where the next
   entry would come within a paragraph and two entries of the block's
   end, PANEL_TAIL, as the original's ends so short of 64K (ReDr00 of
   read.c). A panel's block is the original's 64K and the places at the
   least, PANEL_LEAST, which holds 2,703 entries, and is taken so by
   Init11; ReDir makes it longer while it reads a directory that the
   block does not hold, PanelGrow, and as short as the list is when it
   has read it, but not shorter than that, PanelFit. Neither has a
   ceiling of its own: a panel holds as many entries as there is memory
   for. Each answers with where the panel is then, which may be
   elsewhere, and makes the four pointers to the panels point there.
   PanelGrow answers 0 where there is no more memory, and the panel is
   then where and as long as it was. */
#define PANEL_TAIL    (16 + 2 * (i32)sizeof(struct FILENT))
#define PANEL_LEAST   ((i32)sizeof(struct Places) + 0x10000)
struct WCBdef *PanelGrow(struct WCBdef *panel, i32 kept);
struct WCBdef *PanelFit(struct WCBdef *panel);
void RoomAgain(void);
/* The bytes that the room has at the least, which is what the viewer
   needs, in whole paragraphs as the original counts it: with less the
   program does not start. */
#define MinView     8000                /* Minimum view buffer size */
#define LEAST_ROOM  ((MinView + 15) & ~15)

/* The stack, of stack_dos.c: LenStk bytes in a segment of their own. */
extern u8 Stack[];

/* What the program knows of DOS around it, exec.c. Of its routines
   there, Alloc3 is what the program does first: it looks at the version
   of DOS and keeps no more of the memory than it is.
   TakeEnd has the system put back what SetBrk has changed if DOS ends
   the program. SetBrk
   notes whether DOS looks for Ctrl-C at every call, has it not do so,
   and takes Ctrl-C, so that nothing happens for it. Exec runs the
   commands of DosBuf. AlocEr
   says that there is no memory and leaves, with `bx` for what the
   original has in BX there, and Exit00 leaves with what DOS is to be
   told. */
void Alloc3(void);
void TakeEnd(void);
void SetBrk(void);
void Exec(void);
NO_RETURN void AlocEr(u16 bx);
NO_RETURN void Exit00(u16 code);

/* Texts that the code of more than one file shows: the first three are
   constants of main_t.c and Var_S besides, and the last is one of
   treebox_t.c. They are tables, and a file of tables names them as
   lines of its boxes as well. */
extern const u8 Title_S[];              /* the program's name and version
                                           and what it is based on, behind
                                           two bytes of markup */
extern const u8 NameVer_S[];            /* its name and version alone */
extern const u8 BasedOn_S[];            /* and what it is based on */
extern const u8 Var_S[];                /* a line that is the text at
                                           TmpErr+1 */
extern const u8 Tree_S[];               /* the title of the tree */

/* The numbers of the colours of a panel, for Color. */
#define Back_C  0                       /* what is neither of the others */
#define Cursr_C 1                       /* under the cursor */
#define Slct_C  2                       /* selected */
#define CurSl_C 3                       /* selected, under the cursor */
#define Title_C 4                       /* the titles of the columns */

/* What the program asks of the system that it runs on, sys.h: the
   functions of sys_dos.c by which it has files, directories, drives,
   memory, the time, the keyboard, the screen, the mouse and the timer,
   and what they answer with, struct Answer first. */
#include "sys.h"

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
   to whoever had them, and HookVct takes them again, the first for the
   program's box, FtErr1. Quit puts back what DOS had on the screen before
   the program drew over it, but for the line of the key bar, which is
   blank if there is one. ErrRdy says that the disk of a drive cannot be
   read and asks for another drive: it returns the letter in the low
   byte, or 0 there for none. See each in main.c. */
void RestVct(void);
void HookVct(void);
void Quit(void);
u16 ErrRdy(u16 letter);
/* Read what a panel shows, keeping the cursor of a list of files and
   what is selected in it if `keep`: true if it could not be read. The
   panel may move: `*panel` is where it is afterwards. See read.c. */
bool ReDir(struct WCBdef **panel, bool keep);
/* Get the label of the current drive, a text of at most 11 characters,
   to `volume` in the main data. See read.c. */
struct Label {                          /* what GetVol returns: */
    bool failed;                        /* DOS could not say what the
                                           drive is, or found no label */
    u16 answer;                         /* its error then, and 0 for a
                                           label or a drive of a network:
                                           the original has AX here, and
                                           for a label what DOS left in AH
                                           and the last of 11 characters */
    bool local;                         /* false for a drive of a
                                           network; true for any other,
                                           with a label or without, and
                                           where DOS could not say */
};
struct Label GetVol(u8 *volume);
/* The setup file: see each in setup.c. MakeIni writes into MenuBuf what
   the file holds, and returns how long that is. SaveSet writes it to the
   file, and answers with the bytes written, or with what DOS said was
   wrong if it could not, 53h for a drive that is not ready. */
i32 MakeIni(void);
struct Answer SaveSet(void);

/* The start of the program, start.c. Init00 reads the setup, when the
   program has been loaded, and Init11 then asks for the memory of the
   panels and for the room, all that there is, and makes the panels: it
   answers with how many paragraphs there were, and whether that is too
   few. IniData finds out about the machine, then and after every
   command. GiveMem gives the room back for a command, and TakeMem
   takes it again afterwards, and answers as Init11 does. Main of main.c
   is the program from when DOS has loaded it:
   the system calls it where it enters the program, Start of sys_dos.c,
   and it does not return. */
void Init00(void);
struct Answer Init11(void);
void IniData(void);
void GiveMem(void);
struct Answer TakeMem(void);
NO_RETURN void Main(void);

/* What the setup file holds: three letters V, of each panel what a
   WCBdef has before CurAdr, of the main data what it has before LoadRes,
   and the sum of all these bytes. */
#define WCBini  OFFSET_IN(struct WCBdef, CurAdr16)
#define CmnIni  OFFSET_IN(struct MainDat, LoadRes)
#define INI_LEN (3 + 2 * WCBini + CmnIni + 2)

/* The setup as the main loop last found it, of setup.c: see there. */
extern u8 IniSave[INI_LEN];

/*
 * The setup program.
 *
 * The original has it as a program of its own, VCSETUP, with a copy of
 * the routines that it shares with the main program. Here it is a part
 * of the one program: the files whose names begin with st, and StSetup
 * of stmain.c, which the Options menu calls and which returns when the
 * setup has been left (see DEVIATIONS.md). A routine and an object of
 * the setup program have St in front of the name that they have in its
 * assembly, src/asm/VCSETUP.ASM with VCSTSUB.INC and VCSTHLP.INC, since
 * many of those names are the main program's too; a routine that both
 * programs have word for word is the main program's, and is not
 * written twice.
 *
 * The main data is what the setup program has in its one segment while
 * the setup is open: the setup that is edited is the first CmnIni bytes
 * of it, as it is the main program's, and the routines that both
 * programs have read the colours, the key bar and the clock there. What
 * the setup program has besides is struct StData below, and what it has
 * under another name or of another size is said there too.
 */

/* Not 0 while the setup is open. Three routines that both programs have
   go by it: PutKey0 of bars.c, which has the setup's key bars then,
   Dialog of dialog.c, which draws no menu bar and keeps a typed line
   elsewhere, and Input of input.c, where F1 is the help of the setup.
   Everything else that the setup program's copy of a routine has
   otherwise is the main program's here: each of the three says what. */
extern bool StOpen;

/* What the setup program has of its own, while it is open: in MenuBuf,
   where the main program makes the setup file too, and reads it when it
   starts. A file of the setup has it as `sd`, by a line of its own:
   #define sd (*(struct StData *)md.MenuBuf).

   The first of it is the setup file as it is read and written. The
   original has that as its data from IniData on: the three letters V,
   the two panels, WCB1 and WCB2, the setup that is edited, and the sum.
   Here the setup that is edited is the start of the main data, as it is
   the main program's, and is copied into the file when that is written.

   Of the buffers of the setup program, ScrBuf, WinBuf, TmpBuf and
   TmpErr are the main data's; BoxBuf, which the main program has not, is
   ErrBuf, which is as long; NcPath is the main data's, the directory of
   the program, and has the name of the setup file behind it only where
   stmain.c says so; and UserScr, what DOS had on the screen, is the
   main data's with one row less than the 50 that the setup program
   keeps: StUser of stmain.c has the rows.

   It lies over MenuBuf, which is at an odd place of the main data, so it
   is packed, as the structures above are, and its members are read where
   they are. Gap1 and Gap2 are where a compiler, of a structure that is
   not packed, puts a number of 32 bits at a place that four divides and
   makes the whole as long as four divides, and keep it as it was. */
#pragma pack(push, 1)
struct StData {
    u8  File[INI_LEN + 1];              /* the setup file, and a byte more,
                                           which is read to know a file
                                           that is too long */
    i32 IniStat;                        /* 0 - not found, 1 - Ok, 2 - bad */
    i32 MnItem;                         /* Item of main menu */
    u16 CurType;                        /* Shape of cursor */
    u8  SrcIni[CmnIni];                 /* Source VC.INI */
    u8  BackC;                          /* of the palettes, stcolor.c */
    u8  HistC;
    u8  HstCuC;
    u8  Gap1;
    i32 ColCur;                         /* the item under the cursor, */
    i32 ColFrst;                        /* the first shown, or FFF0h while
                                           none is, */
    i32 ColNum;                         /* and how many there are */
    /* Not of the original: the setup of the program as it was when the
       setup was entered, which the original's main program keeps in its
       own memory meanwhile; and the 50th row of what DOS had on the
       screen. */
    u8  Aside[CmnIni];
    u8  Row50[2 * 80];                  /* its cells, as bytes */
    u8  Gap2[2];
};
#pragma pack(pop)
HOLDS(the_setups_numbers_are_where_they_were,
      offsetof(struct StData, IniStat) % 4 == 0
      && offsetof(struct StData, ColCur) % 4 == 0
      && sizeof(struct StData) % 4 == 0);
HOLDS(the_setups_data_fits_MenuBuf, sizeof(struct StData) <= LenMenu);

/* The setup: entered from the Options menu, and left. It is no command
   that says where the program goes on: that is always where it goes on
   when a command of DOS has ended, which main.c says. See stmain.c. */
void StSetup(void);
/* What DOS had on the screen when the setup was entered, which the setup
   keeps: StUser puts `rows` rows of it into the screen buffer, from the
   row `first` on. See stmain.c. */
void StUser(i32 first, i32 rows);
/* What the items of the setup's menu do, each of which returns when its
   screen has been left: the Configuration, Screen and Mouse, the Panel
   options and the Country info, of stopts.c; the three palettes, of
   stcolor.c. */
void StF02_00(void);
void StF03_00(void);
void StF04_00(void);
void StF05_00(void);
void StF06_00(void);
void StF07_00(void);
void StF09_00(void);
/* The help of the setup, of sthelp.c: its pages read from the help file,
   when the setup is entered, and the page that HlpPage names shown, for
   F1. */
void StRdHelp(void);
void StHelp(void);

/* A line as the original hands it over: the registers that describe
   it. */
struct LineArgs {
    u16 value;                          /* AX or BX: see each routine */
    u16 *screen;                        /* DI: where in the screen buffer */
    const u8 *line;                     /* DS:SI */
    const u8 *colors;                   /* CS:BP: the numbers of four colours */
    const u8 *include;                  /* CS:DX */
};

/* What AdrNum hands back: the original has these in SI, DI, DX and
   AX. */
struct Place {
    i32 cursor;                         /* the number of the cursor's entry */
    i32 first;                          /* of the first one shown */
    i32 last;                           /* of the last one there is */
    i32 lines;                          /* lines of the panel that show
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
u16 *MvsColr(u16 *screen, u16 count, u16 color);
u16 *MovDz0(u16 *screen, const u8 *text);
u8 *ColAddr(u16 number);
u16 ShadCol(u16 color, u16 how);
u16 *Shadow(u16 *cell);
u16 *MvsBw(u16 *screen, const u8 *text, u16 count, u16 color);
u16 *HexByt(u16 *screen, u32 byte);                     /* digits.c */
i32 TstHex(u16 digit);
const u8 *TxtNum(const u8 *text, u16 *number);
void Convert(const u8 *name, u8 *buffer);               /* mask.c */
bool TstMas(const u8 *name, const u8 *mask);
u8 *UnConv(u8 *name, const u8 *buffer);
void NgLine(struct LineArgs *args);                     /* line.c */
void CntLine(struct LineArgs *args);
i32 LnStr(const struct LineArgs *args);
u8 *AddFile(u8 *end);                                   /* string.c */
u8 *MvzLine(u8 *to, const u8 *from, i32 length);
u16 LowCase(u16 letter);
const u8 *CutPath(const u8 *path);
/* The place `at` of a buffer that may be written, where `at` was found
   by something that only reads and answers with a pointer to const, as
   CutPath does: the same place, reckoned from the buffer, so that it is
   the buffer's own pointer. A cast would say the same and drop the
   const. `buffer` is named twice. */
#define PLACE_IN(buffer, at) ((buffer) + ((at) - (buffer)))
/* CutPath of a path that may be written. */
#define CUT_PATH(path) PLACE_IN(path, CutPath(path))
void PathLin(u8 *text);
void LetLin(u8 *text);
void ShrtLin(u8 *text, i32 limit);
void FilZero(u8 *text, i32 length);
i32 StrLen(const u8 *text);
const u8 *FndLet(const u8 *text);
void NormStr(u8 *text);
u8 *MainFil(u8 *to, const u8 *name);
u8 *Number(u8 *buffer, u16 high, u16 low, u16 mode);    /* format.c */
u16 HexDec(i32 number);
/* A date as PutDate() takes it apart. Each field is two characters, the
   first in the low byte, as PUT16() writes them. The assembly's PutDate
   returns this in AX, BX, DX and CX. */
struct DateText {
    u16 century;
    u16 first, second, third;           /* in the country's order */
};
/* A time as PutTime() takes it apart: BX, DX, AX and CH in the assembly. */
struct TimeText {
    u16 hour, minute, second;
    u8  unused;
    u8  suffix;                         /* 'a' or 'p', or a space */
};
void PutDate(u16 date, struct DateText *text);
u8 *BufDate(const struct DateText *text, u8 *buffer);
void PutTime(u16 time, struct TimeText *text);
u8 *BufTime(const struct TimeText *text, u8 *buffer);
u16 *SzDtTm(const struct WCBdef *wcb, const struct FILENT *file,
            u16 *screen, u16 color);
void SetMask(struct WCBdef *panel);                     /* files.c */
bool TstExe(const u8 *name);
void Sort(struct WCBdef *panel);
i32 FnFile(struct WCBdef *panel, const u8 *name);
i32 NearFl(struct WCBdef *panel, const u8 *name, i32 end);
struct Selected {                       /* what is selected in a panel: */
    i32 directories;                    /* how many directories, */
    i32 files;                          /* and how many files */
};
struct Selected TotSel(struct WCBdef *panel);
u16 BegBX(struct WCBdef *panel);                        /* place.c */
i32 NumAdr(struct WCBdef *panel, i32 number);
u16 SizDX(void);
i32 LnsAX(void);
struct WCBdef *InvWCB(struct WCBdef *panel);
bool FilePan(struct WCBdef *panel);
void AdrNum(struct WCBdef *panel, struct Place *place);
void CorPar(struct WCBdef *panel);
struct Shown {                          /* what a panel shows, as offsets
                                           in its list's home, the panel
                                           for files and TRELIST for the
                                           tree (see TREE_OFFSET): */
    i32 cursor;                         /* the entry that the cursor is on, */
    i32 first;                          /* and the first one shown */
};
struct Shown ChgAdr(struct WCBdef *panel, i32 cursor, i32 first);
u8 *PtPt10(struct WCBdef *panel);                       /* panel.c */
void Panel(struct WCBdef *panel);
void PutPath(struct WCBdef *panel);
void PutFls(struct WCBdef *panel);
void SetCur(struct WCBdef *panel, bool set);
u8 *FilDir(u8 *buffer, i32 directories, i32 files);
/* The tree in a panel, tree.c: its colours are four numbers in the code,
   which come as a pointer. */
void PutTree(struct WCBdef *panel, u16 place, u16 size, const u8 *colors);
void PutTre(struct WCBdef *panel);
void TreeCur(void);
void TrCr10(void);
void TrCr20(i32 lines);
void DirSize(struct WCBdef *panel);                     /* read.c */
u16 DiSi10(u16 attributes);
void TstInfo(struct WCBdef *panel);
bool ReadTre(void);                                     /* treebox.c */
bool ChTree(i32 column);
void PutInf(struct WCBdef *panel);                      /* info.c */
/* Where a box is on the screen, as DialBox says it: what the assembly
   has in BX, DX and SI then. */
struct Box {
    u16 begin;                          /* the row in the high byte */
    u16 size;                           /* the rows in the high byte */
    u16 *saved;                         /* where what it covers is kept */
};
/* What the files of the commands call of each other: see each in its
   file. */
u16 C00_00(u16 attributes);                             /* copy.c */
bool F35_61(u16 answer, u16 place);
u16 F38_10(u16 answer);                                 /* dirfunc.c */
void T71_00(u16 function, u16 place, i32 step);         /* edit.c */
u16 FindFil(u16 attributes);                            /* find.c */
void Help00(void);                                      /* help.c */
void F20_02(struct WCBdef *panel);                      /* keys.c */
u16 F32_70(u16 begin, u16 size);                        /* usermenu.c */
void V80_00(const u8 *include, struct Box *where);      /* view.c */
void V81_00(void);

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
   0 for an item that is none (commands_t.c). Each has the name of
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
       table of the keys has them in: see Func12 of main.c. The original
       has two more, which the portable C has not (see DEVIATIONS.md):
       F65_00 behind F64_00, the map of the memory, Alt+F5, and F69_00
       behind F68_00, EGA lines, Alt+F9. */
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
    CMD_F64_00, CMD_M42_00, CMD_F67_00, CMD_F68_00, CMD_M41_00,
    CMD_F75_00,
    /* Those that only the menus have, or have at a label of their own. */
    CMD_M01_00, CMD_M03_00, CMD_M06_00, CMD_M08_00, CMD_F17_01,
    CMD_M16_01, CMD_F62_01, CMD_M21_00, CMD_F76_01, CMD_M53_00,
    CMD_M60_00, CMD_M61_00, CMD_M62_00, CMD_M63_00, CMD_M64_00,
    CMD_M66_00, CMD_M67_00, CMD_M69_00, CMD_M70_00, CMD_M71_00,
    CMD_M72_00,
    /* The one that the original has not: the setup program, which is a
       program of its own there and an item of the Options menu here
       (see DEVIATIONS.md). It is the last, so that no other has another
       number for it: the test of the data is told the number. */
    CMD_SETUP
};

/* Where the program goes on after a command of the keys of the panels,
   of keys.c, enter.c and view.c: the place that such a command returns,
   in struct Onward below. */
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
    NEXT_INIT11,                        /* start over as after a command:
                                           F21_30 has left the panels */
    NEXT_EXEC,                          /* and first have the commands of
                                           DosBuf run */
    NEXT_VIEW,                          /* V00_00: the viewer on the file
                                           ViewFil */
    NEXT_EDIT,                          /* the same with 1 in DL: the
                                           editor on it */
    NEXT_DIRINFO,                       /* V00_02: the editor in the window
                                           of an Info panel */
    NEXT_EXIT                           /* leave the program */
};

/* What a command of the keys of the panels returns: the place, and for
   the three places that are routines of a panel's menu what the routine
   takes, which is 0 with any other place. */
struct Onward {
    u8  place;                          /* where the program goes on: a
                                           NEXT_ */
    u8  item;                           /* for M03_00 five less the kind of
                                           panel, and for M06_00 the place
                                           in the menu bar that is lit
                                           meanwhile */
    struct WCBdef *panel;               /* the panel that the routine of
                                           the menu is for */
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

/* After a command of dirfunc.c, which returns one of these, and after
   the file search, which returns one in struct Searched below. */
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

/* After Copy and Rename, of copy.c: the place that they return, in
   struct Copied below. */
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
   put it there: see main.c for what is read. SI and DI carry nothing
   round the loop but the numbers of two entries of a panel, for the
   mouse's commands, and with them `last`, the number that the original
   has in DX for one of them, F01_05: a panel holds as many entries as
   there is memory for, and the numbers are 32 bits. DX keeps that
   number too, in its 16 bits, for what reads DX as the loop leaves it
   (Func40 of main.c). */
struct Regs {
    u16 ax, bx, cx, dx;
    i32 si, di;                         /* the numbers of two entries */
    u16 bp;
    struct WCBdef *ds;                  /* the current panel */
    i32 last;                           /* and of the last entry */
};

/* The keys of the panels, keys.c: `panel` is the current panel. F00_60,
   F00_05 and F01_05 are where the mouse comes in, with the numbers of the
   cursor's entry, of the first one shown and of the last one there is,
   and with its buttons; F76_01 and F17_01 where the menus do, F17_01 with
   the panel of the menu. The keys of the cursor leave BX and DX in `r`
   as the original has them when it goes back to the loop: F00_50, which
   moves the cursor and which the speed search calls too, returns what it
   leaves in the two. */
struct Left {
    u16 bx;                             /* BX, and */
    u16 dx;                             /* DX: see F00_50 for what each
                                           holds */
};
struct Left F00_50(struct WCBdef *panel, i32 cursor, i32 first);
struct Onward F00_60(struct WCBdef *panel, i32 cursor, i32 first, u16 buttons,
                     struct Regs *r);
struct Onward F00_00(struct WCBdef *panel, struct Regs *r);
struct Onward F00_05(struct WCBdef *panel, i32 cursor, i32 first, u16 buttons,
                     struct Regs *r);
struct Onward F01_00(struct WCBdef *panel, struct Regs *r);
struct Onward F01_05(struct WCBdef *panel, i32 cursor, i32 first, i32 last,
                     u16 buttons, struct Regs *r);
struct Onward F02_00(struct WCBdef *panel, struct Regs *r);
struct Onward F03_00(struct WCBdef *panel, struct Regs *r);
struct Onward F04_00(struct WCBdef *panel, struct Regs *r);
struct Onward F05_00(struct WCBdef *panel, struct Regs *r);
struct Onward F06_00(struct WCBdef *panel, struct Regs *r);
struct Onward F07_00(struct WCBdef *panel, struct Regs *r);
struct Onward F08_00(struct WCBdef *panel, struct Regs *r);
struct Onward F76_00(struct WCBdef *panel);
struct Onward F76_01(struct WCBdef *panel);
struct Onward F09_00(struct WCBdef *panel);
struct Onward F10_00(struct WCBdef *panel);
struct Onward F78_00(struct WCBdef *panel);
struct Onward F11_00(struct WCBdef *panel);
struct Onward F12_00(struct WCBdef *panel);
struct Onward F13_00(void);
struct Onward F14_00(void);
struct Onward F15_00(struct WCBdef *panel);
struct Onward F16_00(void);
struct Onward F17_01(struct WCBdef *panel);
struct Onward F79_00(struct WCBdef *panel);
struct Onward F18_00(struct WCBdef *panel);
struct Onward F20_00(struct WCBdef *panel);

/* Enter, the directory changed and the history, enter.c. F21_10 is Enter
   on the entry under the cursor, F21_25 a command of the command line
   run, F21_31 the program started over to run what DosBuf holds;
   F24_01 goes to the directory whose
   path is at `path` in the main data.
   F24_02 has the panels show the directory that DOS is in when they
   are drawn next, `panel` being the current one: it is no command, and
   what calls it goes on at Init50, which draws them. */
struct Onward F21_00(struct WCBdef *panel);
struct Onward F21_10(struct WCBdef *panel);
struct Onward F21_25(void);
struct Onward F21_31(void);
struct Onward F22_00(struct WCBdef *panel);
struct Onward F23_00(struct WCBdef *panel);
struct Onward F24_00(struct WCBdef *panel);
struct Onward F24_01(struct WCBdef *panel, const u8 *path);
void F24_02(struct WCBdef *panel);
struct Onward F25_00(void);
struct Onward F26_00(void);
struct Onward F27_00(struct WCBdef *panel);
struct Onward F30_00(struct WCBdef *panel);
struct Onward F75_00(struct WCBdef *panel);

/* The viewer and the editor, view.c and edit.c: the commands, and what
   they go on in. V00_02 opens the file ViewFil and says in `open` what it
   found, 0 if there is nothing to show, 1 for the editor and 2 for the
   viewer; T00_00 is the editor, which returns how it was left. V00_10 is
   the viewer; V13_00 closes the file and goes back to the panels, as
   V13_01 does without closing one. See each there. */
struct Open {
    u16 handle;                         /* 0FFFFh for a new file */
    struct Box box;                     /* where the box "Reading the
                                           file" is */
};
enum {                                  /* how the editor was left: */
    EDIT_VIEW,                          /* the file is to be viewed
                                           instead */
    EDIT_CLOSE,                         /* it is to be closed, and nothing
                                           more */
    EDIT_DONE                           /* it has been edited and is
                                           closed */
};
struct EditEnd {
    u8  how;                            /* an EDIT_ above */
    bool written;                       /* with EDIT_DONE whether a file
                                           was written: what V13_01
                                           takes */
};
struct Onward F43_00(void);
struct Onward F44_00(void);
struct Onward F63_00(struct WCBdef *panel);
struct Onward F64_00(struct WCBdef *panel);
struct Onward F33_00(struct WCBdef *panel);
struct Onward F34_00(struct WCBdef *panel);
struct Onward F34_10(struct WCBdef *panel);
i32 V00_02(i32 menu, bool edit, bool whole, struct Open *open);
void V00_10(u16 handle);
void V13_00(void);
void V13_01(bool changed);
struct EditEnd T00_00(u16 handle, u16 begin, u16 size);
/* Two that the viewer and the editor share, and the file search the
   second: T70_00 of edit.c clears a line of the window in the screen
   buffer, and V82_00 of view.c looks for the text of a search in `count`
   bytes at `at` of the text `text`. */
struct Cleared {
    u16 *screen;                        /* where the line is in the screen
                                           buffer */
    u16 place;                          /* its row and column on the
                                           screen */
};
struct Sought {
    u16 found;                          /* 1 if the text was found, capitals
                                           and small letters alike, 0 if
                                           the bytes were, 100h if neither */
    i32 end;                            /* where the search ended: behind
                                           what was found */
};
struct Cleared T70_00(i32 line);
struct Sought V82_00(const u8 *text, i32 at, i32 count);

/* The commands of the menus, commands.c: `panel` is the current panel,
   or the panel of the Left or the Right menu, and `item` what the
   original has in CX, from the menu bar the menu in the low byte and the
   item in the high one. M01_01, M03_00, M06_00 and M06_10 are where the
   keys of the panels and the box of the drives go on. */
i32 M01_00(struct WCBdef *panel, u16 item);
i32 M01_01(struct WCBdef *panel, u16 item);
i32 M03_00(struct WCBdef *panel, u16 item);
i32 M06_00(struct WCBdef *panel, u16 item);
i32 M06_10(struct WCBdef *panel, u16 item);
i32 M08_00(struct WCBdef *panel, u16 item);
i32 M16_00(struct WCBdef *panel);
i32 M16_01(struct WCBdef *panel);
i32 M16_10(struct WCBdef *panel);
i32 M21_00(void);
i32 M41_00(void);
i32 M42_00(struct WCBdef *panel);
i32 M49_00(struct WCBdef *panel);
i32 M60_00(void);
i32 M61_00(void);
i32 M53_00(u16 item);
i32 M62_00(u16 item);
i32 M63_00(u16 item);
i32 M64_00(u16 item);
i32 M66_00(struct WCBdef *panel);
i32 M67_00(struct WCBdef *panel);
i32 M69_00(struct WCBdef *panel);
i32 M70_00(struct WCBdef *panel);
i32 M71_00(struct WCBdef *panel);
i32 M72_00(struct WCBdef *panel);
i32 M75_00(struct WCBdef *panel);

/* Directories made, files deleted, attributes and the label, dirfunc.c;
   and where a command ends that goes through files: FncSel for those
   that are selected, FncMas for those of a mask, with the title of its
   boxes, and FncExt, which like FncSel returns true if the panels are to
   be read again. */
i32 F37_00(struct WCBdef *panel);
i32 F47_00(void);
i32 F38_00(struct WCBdef *panel);
i32 F48_00(void);
i32 F58_00(struct WCBdef *panel);
i32 M30_00(struct WCBdef *panel);
i32 F54_00(void);
i32 FncMas(const u8 *include);
bool FncSel(void);
bool FncExt(void);
/* What the dialog of attributes has Dialog call when it is answered,
   with the answer: it reads the date and the time that were typed. */
struct Checked {                        /* what M30_60 returns: */
    u16 answer;                         /* the answer for the dialog to go
                                           by */
    bool refused;                       /* whether a line is no date or no
                                           time: the dialog goes on */
};
struct Checked M30_60(u16 answer);
/* What Panel options of the setup has Dialog call in the same way: it
   reads the delay of the tree that was typed. See stopts.c. */
struct Checked StF04_10(u16 answer);

/* Copy and Rename, copy.c: `command` is 1 to rename, and `directories`
   and `files` are what the original has in BX and DX at the jump. Where
   the command does not count what is selected, it goes by those two as
   they come: see F35_50 there. */
struct Copied {                         /* what they return: */
    i32 place;                          /* where the program goes on: a
                                           GO_ */
    const u8 *title;                    /* with GO_MASKED the title of the
                                           boxes */
};
struct Copied F35_01(struct WCBdef *panel, i32 command, i32 directories,
                     i32 files);
struct Copied F45_01(struct WCBdef *panel, i32 command, i32 directories,
                     i32 files);

/* The file search, find.c; the command line, cmdline.c: a key for it,
   Esc, the selected files and the path of a panel put into it, the
   history in its box, and the speed search; the user menu, usermenu.c,
   1 for the main one; the box of the drives, drive.c; and the menu bar,
   menu.c. See each there. */
struct Searched {                       /* what the file search returns: */
    i32 place;                          /* where the program goes on: a
                                           TO_ */
    struct WCBdef *panel;               /* with TO_F24_02 the panel that is
                                           to show the directory which DOS
                                           is in now */
};
struct SpeedKey {                       /* what the speed search returns: */
    u16 key;                            /* the key that ended it, to be
                                           acted on: 0 for Esc */
    u16 box;                            /* where its box was */
};
struct Picked {                         /* what the menu bar returns: */
    u8  menu;                           /* the menu, 1 to 5 */
    u8  item;                           /* the item, from 1 with the lines
                                           across the menu: 0 on the bar
                                           alone, and FFh in a menu with no
                                           item under the cursor */
    u8  held;                           /* what the assembly has in DL when
                                           it jumps to the command */
    u8  table;                          /* which of the tables of
                                           commands_t.c has the command of
                                           the item, from 1: 0 if none was
                                           chosen */
};
struct Searched F67_00(void);
void F73_00(u16 key);
bool F74_00(void);
bool F80_00(struct WCBdef *panel);
void F82_01(struct WCBdef *panel, u16 key);
bool F68_00(void);
struct SpeedKey F77_00(u16 key, struct WCBdef *panel);
bool F32_00(bool main);
bool F62_01(struct WCBdef *panel, i32 item);
struct Picked F39_00(bool again, struct WCBdef *panel);

/* Make the program ready for what the command line holds to be run: the
   interrupts given back, the screen as DOS left it. Returns the place
   NEXT_EXEC, or NEXT_INIT11 if there is nothing to run, for the command
   to return in its turn. See enter.c. */
struct Onward F21_30(void);
/* The commands of a line of an extension file or a menu file, in
   MenuBuf: F34_30 looks for the next one from `text`, and F34_40 at
   whether the command at `text` is only the name of a file, a program
   for `view` and else a menu, whose path it has put at TmpBuf then. See
   view.c. */
struct Next {                           /* what F34_30 found: */
    bool none;                          /* whether there is no command */
    u8 *at;                             /* where the command begins: or,
                                           with none, the line that is no
                                           command, or the place behind
                                           the end of the file */
};
struct Named {                          /* what F34_40 found: */
    bool file;                          /* whether the command is only
                                           the name of a file */
    u8 *at;                             /* if it is not, the place before
                                           the command */
};
struct Next F34_30(const u8 *text);
struct Named F34_40(u8 *text, bool view);
/* Have both panels read again: where a command ends that has changed
   what is on the disk. Returns true. See dirfunc.c. */
bool FncEx1(void);

/* Calls what FncAddr names for every file of the directory at TmpBuf, and
   returns 1 if every call did what it was to, 0 if one did not, and 100h
   when it was stopped. */
u16 DirFunc(void);                                      /* dirfunc.c */
/* Marks the tree's file of the drive that `path` names, or of the current
   one, as out of date. Returns what DOS answered, and whether that is an
   error. */
struct Answer DelTr0(const u8 *path);

/* Takes the mark off `count` bytes that were read from the help file,
   in the main data; a count of 0 is one of 65536. The help of the setup,
   sthelp.c, calls it too: the setup program has the routine word for
   word. See help.c. */
void DeCode(u8 *at, u16 count);

/* The boxes and the dialogs, which the assembly describes, each handed
   on by a pointer to it. A box is handed on as what every box begins
   with and a dialog as its header, the two structures of tables.h: a
   file of code declares one as that structure, `extern const struct
   BOXTOP Name;` and `extern const struct BOXHD Name;`, and hands on its
   address, so a box is refused where a dialog is asked for, and a text
   for either. `include` is a text, or 0 for none; `table` and `control`
   are a line of buttons and `color` numbers of colours, bytes all. And
   `data` is what a box shows of and a dialog sets in, at the offsets
   that their tables name: &md for the main data, or a panel. See each
   in dialog.c. */
struct BOXTOP;
struct BOXHD;
void DialBox(u16 place, const struct BOXTOP *head, const u8 *include,
             const void *data, struct Box *where);
u16 Dialog(const struct BOXHD *head, const u8 *include, u16 begin, u16 size,
           void *data);
void InSt10(u16 place, const u8 *text, i32 length, u16 color);
struct Buttons {                        /* what MinMnu returns: */
    u16 key;                            /* the key that is for the caller */
    u16 chosen;                         /* the button chosen, in the low
                                           byte; above it the key bar as it
                                           came, or 0 for the letter of a
                                           button */
};
struct Taken {                          /* what MnMn10 returns: */
    u16 key;                            /* the key */
    i32 button;                         /* the button chosen */
    bool taken;                         /* the buttons have dealt with the
                                           key */
};
struct Buttons MinMnu(const u8 *control, u16 begin, u16 size,
                      const u8 *color, u16 keys, u16 item);
struct Taken MnMn10(u16 key, u16 place, u16 item, u16 size,
                    const u8 *table);
u16 MnMn40(u16 colors, u16 place, u16 item, u16 size, const u8 *table);
/* The key bar and the menu bar, the command line and its prompt: see
   each in bars.c. `data` is where ClsLine reads the colour, &md. */
u16 *ClsLine(u16 place, const struct MainDat *data);
void PutKey(i32 bar);
void PutKey0(i32 bar, i32 shift);
void PutMnu(i32 item);
void PtMnu1(i32 item);
void PutPrm(struct WCBdef *panel);
void DosCom(void);
/* The entries of an extension file or a menu file, in MenuBuf: FndItem
   returns where the next one is, and whether that is the end of the
   file; FindExt where the command for a file's extension is, or 0. See
   extfile.c. Run_00 makes the commands of a line that begin at `text`
   into DosBuf, and is true if there is one to run: see usermenu.c. */
struct Entry {
    const u8 *at;                       /* where it is */
    bool end;                           /* whether the file ends there */
};
struct Entry FndItem(const u8 *file);
u8 *FindExt(const u8 *name);
bool Run_00(u8 *text, bool view);
/* Waiting for a key, and a line that is typed into: see each in input.c.
   Input and Input0 return the key, the scan code in the high byte, where
   FCh says that the mouse did something; Input0 says besides whether
   the key was clicked on the key bar. EdLin returns the column of the
   cursor, and whether anything changed. CmpSym is true for a
   character that words end at. Search is the speed search among names
   in the segment `segment`. Rnd is the next of the program's random
   numbers. */
struct Key {
    u16 key;                            /* the key, the scan code in the
                                           high byte */
    bool clicked;                       /* true if the mouse clicked it
                                           on the key bar */
};
struct Edited {
    i32 cursor;                         /* the column of the cursor */
    bool changed;                       /* true if the line or the cursor
                                           has changed */
};
u16 Input(i32 bar);
struct Key Input0(i32 bar);
u16 Rnd(void);
struct Edited EdLin(u16 key, u8 *text, i32 cursor, i32 length);
i32 EdSub1(const u8 *line, i32 cursor);
bool CmpSym(u16 symbol);
i32 Search(const u8 *list, i32 at, i32 begin, i32 end, i32 step,
           u16 key);

/* What the program makes of the system at every turn: see each in
   system.c. OpenFil opens a file, `mode` as DOS takes it in AL, and
   answers with the handle, or with the error. SetPth asks for the
   current drive and directory, into DosPth, and is true if that fails.
   Fantom asks for the disk of a drive that shares a floppy drive with
   another, 1 for A:, and ChgDisk for that of the drive that a path of
   the main data names: true if the user would not put it in. */
u32 Timer(void);
void ClrKbd(void);
bool TstBrk(void);
void Beep(void);
u16 UpCase(u16 letter);
void SetDTA(void);
bool TstDrv(u16 letter);
struct Answer OpenFil(const u8 *name, u16 mode);
bool SetPth(void);
bool Fantom(i32 drive);
bool ChgDisk(const u8 *path);
/* The mouse: see each in mouse.c. GetMous says which buttons are held
   and where the pointer is, InWind is not 0 for a place
   outside a window, and DoublMo is 2 to go on, 1 for a second click and 0
   for one that was let go elsewhere. */
struct Mouse {
    u16 buttons;                        /* those that are held: the left
                                           one 1, the right one 2, the
                                           middle one 4 */
    u16 place;                          /* the cell that the pointer is
                                           on, the row in the high byte */
};
void IniMous(void);
void TypMous(u16 pointer);
void SavMous(void);
void CurMous(bool show);
struct Mouse GetMous(u16 place);
i32 DoublMo(u16 begin, u16 size, i32 cursor);
bool InWind(u16 where, u16 begin, u16 size);
/* The screen, its cursor, and windows of the screen buffer: see each in
   window.c. A place has the row in its high byte, and a size the rows.
   `buffer` is an address in the main data. */
bool ChkMode(struct Screen *screen);
void Cursor(u16 place);
void Cursor1(void);
u16 ChgCur(void);
void CurShp(u16 shape);
u16 GetCur(void);
void HidCur(void);
void Scroll(bool down, u16 color, u16 upper, u16 lower);
u16 ScrOffs(void);
u16 SftEGA(u16 place);
void SavWin(u16 begin, u16 size, u16 *buffer);
void ResWin(u16 begin, u16 size, const u16 *buffer);
void ResUser(u16 begin, u16 size);
void Window(u16 begin, u16 size);
void SavScr(void);
/* The frame of a box: see each in box.c. OncZoom takes a box of the
   assembly as DialBox does, in the main data. */
u16 *BoxLine(u16 *screen, u16 cell, u16 ends, i32 margin, i32 width);
u16 *BoxDubl(u16 *screen, u16 color, i32 rows, i32 margin, i32 width);
void MinBox(u16 begin, u16 size, u16 color);
u16 StdLine(u16 place, u16 cell, u16 ends, u16 size);
void OncZoom(u16 place, const struct BOXTOP *box, const u8 *include,
             struct Box *where);
/* The screen blanker, which comes back when a key is pressed or the mouse
   moved, and the word to DOS that the program is idle: see each in
   blank.c. */
void Blank(void);
void LeaveTm(void);
/* Small ones: see each in misc.c. PgUp and PgDn return where a list is
   after a page up or down. */
bool IsCrLf(u16 word);
bool InvVar(u8 *variable);
u16 ChkSum(const u8 *data, u16 length);
struct Paged {
    i32 cursor;                         /* the line that the cursor is on */
    i32 first;                          /* the first line shown */
};
struct Paged PgUp(i32 cursor, i32 first, i32 rows);
struct Paged PgDn(i32 cursor, i32 first, i32 rows, i32 lines);

/* The words of the addresses. The program keeps a few places of the
   memory in numbers where the original kept them in registers or in
   words: an offset in a panel, an entry of the tree's list, a place of
   the main data. A segment is no such number any more: a panel, a text,
   a block and a table are pointers. These words make a pointer of such
   a number and the number of a pointer, and the code of the program is
   to do it through them and nowhere else: a cast between a pointer and
   a number that is written out in a file is a place that has still to
   go through one of them, and harness/hostcheck.py counts it.

   Each counts from where its block, list or structure begins, and says
   at it what it takes for granted. None of them rests on where the
   linker or the system has put anything, as the program kept the
   memory of the original inside its own until the piece F10 of
   TODO.md, "The portable C in a flat memory: the plan": a megabyte
   counted in paragraphs from 64K below the main data, in which a
   segment and an offset were made a pointer. */

/* The number in a word of a table that holds one where it could name
   something, NUMBER of tables.h: the pointer's low 16 bits, by way of a
   number that is as long as a pointer, size_t of the header that
   OFFSET_IN includes. A word that names something is read as the
   pointer that it is. */
#define NUMBER_OF(names) ((u16)(size_t)(names))

/* A pointer into the main data is a pointer. It takes nothing for
   granted, and does not look whether the pointer is into the main
   data. */
#define NEAR(pointer) ((u8 *)(pointer))

/* A place of the main data as a number: how far it is behind where the
   main data begins; and the pointer to a place that is kept as a
   number, which is cut to 16 bits first. They take for granted that the
   place is in the main data, which is shorter than 64K. A pointer to
   anything else, a table, a local variable, a panel, gives a number
   that means nothing, with no word said. */
#define MD_PLACE(pointer) ((u16)((const u8 *)(pointer) - (const u8 *)&md))
#define MD_POINTER(place) ((u8 *)&md + (u16)(place))

/* Where in the main data a field is, as a number: in a table, the word
   that the assembly has for the name of a field of MainDat; in the code,
   a place of the main data that a field gives, as the original leaves
   in a register or hands DOS. A field of a field is written with a dot,
   MD_AT(DTA.FilAttr). */
#define MD_AT(field) OFFSET_IN(struct MainDat, field)

/* The main data as bytes. Where the program reads or copies the main
   data across its fields, as the original reads its main data as one
   block, in front of a field, behind its end or from one field into the
   next, it reads these bytes at the place of the field, MD_AT, and the
   number of bytes from there: C says nothing of what lies next to a
   field, and of the bytes of the structure it does. A field that is read
   within its bounds is read as the field. */
#define MD_BYTES ((u8 *)&md)

/* Where in a panel's block its list of files begins, DskBuf, an offset
   in the panel; and the list of the tree's directories in the main
   data, TreeBuf.TreList, a pointer to its first entry, the root. The
   assembly keeps places in both as addresses, not as numbers. The list
   begins inside the tree's header, where its last field is, and goes on
   behind it: it is the bytes of the main data from there. */
#define DSKBUF  ((i32)(sizeof(struct WCBdef) - MinFls * sizeof(struct FILENT)))
#define TRELIST \
    ((struct DIRENT *)(MD_BYTES + MD_AT(TreeBuf) \
                       + offsetof(struct DIRTREE, TreSum2)))

/* An entry of the tree's list as an offset in the list, in bytes, and
   the entry at such an offset. What moves a cursor through a panel's
   files or through the tree alike, F00_50 of keys.c, the speed search,
   ChgAdr and NumAdr of place.c, the tree's box, knows an entry of
   either list by a number of 16 bits: a file by its offset in the
   panel, which is 16 bits by the panel's layout, and a directory by
   this, where the original has its address in the main data. The two
   differ by where the list begins, in every such number alike, and
   what the program does with them is differences and comparisons
   within the list; the list is in the main data, far shorter than
   64K. The offset is cut to 16 bits, as ENTRY cuts a panel's. */
#define TREE_OFFSET(entry) \
    ((u16)((const u8 *)(entry) - (const u8 *)TRELIST))
#define TREE_ENTRY(offset) \
    ((struct DIRENT *)((u8 *)TRELIST + (u16)(offset)))

/* The file entry at an address in a panel, an offset from where the
   panel begins: `wcb` is the panel at every use. The original's offset
   was 16 bits, and one that went below 0 wrapped round in its segment;
   here a panel is longer than 64K, and an address is a place in it,
   never below 0. */
#define ENTRY(wcb, address) \
    ((struct FILENT *)((u8 *)(wcb) + (address)))

/* The places of a panel's list, struct Places, which are in front of
   the panel in its block. */
#define PLACES(wcb) ((struct Places *)(wcb) - 1)

/* Bits of a file's attributes. The two highest are the program's own, in
   its lists of files. */
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_DIRECTORY 0x10
#define ATTR_SELECTED  0x40

/* Add to the low byte of a word, as the assembly does to a column or a
   count that it keeps in half a register: nothing is carried over. */
#define ADD_LOW(word, more) (((word) & 0xFF00) | (((word) + (more)) & 0xFF))

#endif
