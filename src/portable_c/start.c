/*
 * The start of the program: the setup file read, or the defaults taken
 * where there is none that will do; the banner; the memory for the main
 * data asked of DOS, and the main data made of what the part of the
 * program that stays in the memory has kept; and what the program finds
 * out about the machine before it draws anything.
 *
 * Translated from VC.ASM.
 *
 * The program runs through this once from Init00, when it is started, and
 * from Init11 every time it comes back from a command, with all but the
 * part that stays loaded anew. Up to Init11 there is no main data: the
 * setup and the main extension file are read to where a command would
 * leave them, ComnIni and the two panels in the part that stays. Init00
 * and Init11 therefore get that part as a segment and touch nothing but
 * through far pointers; DS is of no use to them. Init11 asks DOS for the
 * main data and hands back its segment, and from there on DS is on it:
 * IniData, a name that the assembly does not have, is the rest, as far
 * as the main loop (see main.c).
 *
 * TrnMain of main.c calls the three, with the segment of that part, and
 * puts DS on the main data when Init11 has returned its segment.
 */
#include "vc.h"

/* Init00, Init11 and IniData are declared in vc.h: TrnMain of main.c
   calls them. */

/* AsmSeg, of vc.h, returns the segment of the program's constants, and
   ResSeg the segment of the part of the program that stays in the memory
   while a command runs: its PSP is there, and its stack. Neither needs
   DS. */

/* What this reads, writes or names of other files. In the group of the
   constants: the names of the setup file and of the main extension file,
   13 bytes each with what follows them, and the word where FatErr finds
   the main data. In the part that stays, of struct Stays there: what is
   kept while a command runs, ComnIni of the main data and the two
   panels; how many programs have stayed in the memory since; and where
   the interrupts 22h and 27h went before the program took them. And of
   that part besides: where the interrupt 21h went; where the linker's
   relocation table is, in paragraphs from the PSP; and the routines that
   the program has the three interrupts go to, with the one that takes
   Ctrl-Break. */
#define ComnIni STAYS(ComnIni)
#define WCB1sav STAYS(WCB1sav)
#define WCB2sav STAYS(WCB2sav)
#define Counter STAYS(Counter)
#define Vect22  STAYS(Vect22)
#define Vect27  STAYS(Vect27)
extern const u8 __far Init_F[];
extern const u8 __far Ext_F[];
extern const u8 __far SecDat[];
extern const u8 __far Vect21[];
extern const u8 __far RelTab[];
extern const u8 __far TrnPrW[];
extern const u8 __far New21[];
extern const u8 __far Exit01[];
extern const u8 __far New27[];
extern const u8 __far SetBrk[];
#pragma aux Vect21 "*"
#pragma aux RelTab "*"
#pragma aux TrnPrW "*"
#pragma aux New21 "*"
#pragma aux Exit01 "*"
#pragma aux New27 "*"
#pragma aux SetBrk "*"

/* A function of the part that stays, resident.c, which is called
   far and loads its own DS: SetBrk keeps every register but AX and DX. */
typedef void __far StaysFn(void);
#pragma aux StaysFn modify exact [ax dx]

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define AT(segment, offset) ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))

/* The PSP: where memory that DOS has ends, where the program goes on
   when it ends, and the environment. */
#define PSP_MEMORY    0x02
#define PSP_TERMINATE 0x0A
#define PSP_ENVIRON   0x2C

#define MinView 8000                    /* Minimum view buffer size */

/* What the setup file holds: three letters V, of each panel what a
   WCBdef has before CurAdr, of the main data what it has before LoadRes,
   and the sum of all these bytes. A panel is kept, while a command runs,
   up to ExtBuf, and the main data up to DosLin. */
#define WCBini  ((u16)(u32)&((struct WCBdef __far *)0)->CurAdr)
#define WCBsave ((u16)(u32)&((struct WCBdef __far *)0)->ExtBuf)
#define CmnIni  ((u16)&((struct MainDat *)0)->LoadRes)
#define CmnSave ((u16)&((struct MainDat *)0)->DosLin)
#define INI_LEN (3 + 2 * WCBini + CmnIni + 2)

#define FAILED 0xFFFF                   /* ReadOne: nothing was read */

/* The setup that the program has without a setup file: the colours, six
   palettes of each, and what follows them in the main data as far as the
   bytes that are reserved. */
static const u8 CODE Palets[] = {
    0x07, 0x07, 0x1B, 0x07, 0x07, 0x07, /*  0 - Main windows */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  1 - Cursor */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /*  2 - Selected files */
    0x7F, 0x09, 0x3E, 0x09, 0x70, 0x70, /*  3 - Cursor for selected files */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /*  4 - "Name   Size ..." */
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, /*  5 - Key bar numbers */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  6 - Key bar titles */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  7 - Menu bar */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /*  8 - Menu bar cursor */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  9 - Menu box border */
    0x70, 0x70, 0x3F, 0x70, 0x70, 0x70, /* 10 - Menu box text */
    0x7F, 0x70, 0x3E, 0x70, 0x70, 0x70, /* 11 - Menu box high letters */
    0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /* 12 - Menu box text cursor */
    0x0F, 0x0F, 0x0E, 0x0F, 0x0F, 0x0F, /* 13 - Menu box high letters cursor */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 14 - Menu box dark lines */
    0x70, 0x70, 0x33, 0x70, 0x70, 0x70, /* 15 - Menu box dark lines '-' */
    0x07, 0x07, 0x70, 0x07, 0x07, 0x07, /* 16 - Function box */
    0x0F, 0x0F, 0x7E, 0x0F, 0x0F, 0x0F, /* 17 - Function box bright */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 18 - Function box cursor */
    0x07, 0x07, 0x3F, 0x07, 0x07, 0x07, /* 19 - Configuration window */
    0x70, 0x70, 0x0F, 0x70, 0x70, 0x70, /* 20 - Configuration cursor */
    0x0F, 0x0F, 0x3E, 0x0F, 0x0F, 0x0F, /* 21 - Configuration window bright */
    0x7F, 0x09, 0x0E, 0x09, 0x70, 0x70, /* 22 - Configuration cursor bright */
    0x70, 0x70, 0x3F, 0x70, 0x70, 0x70, /* 23 - History window */
    0x07, 0x07, 0x0F, 0x07, 0x07, 0x07, /* 24 - History cursor */
    0x07, 0x07, 0x13, 0x07, 0x07, 0x07, /* 25 - Menu help border */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 26 - Menu help window */
    0x07, 0x07, 0x30, 0x07, 0x07, 0x07, /* 27 - Help window */
    0x0F, 0x0F, 0x3F, 0x0F, 0x0F, 0x0F, /* 28 - Help window bold */
    0x70, 0x70, 0x0F, 0x70, 0x70, 0x70, /* 29 - Help reversive */
    0x0F, 0x09, 0x3E, 0x09, 0x0F, 0x0F, /* 30 - Help underline */
    0x07, 0x07, 0x4F, 0x07, 0x07, 0x07, /* 31 - Error windows */
    0x70, 0x70, 0x70, 0x70, 0x70, 0x70, /* 32 - Error window cursor */
    0x0F, 0x0F, 0x4E, 0x0F, 0x0F, 0x0F, /* 33 - Error window bright */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 34 - Clock */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /* 35 - Arrows in view/edit */
    0x07, 0x07, 0x0B, 0x07, 0x07, 0x07, /* 36 - Screen blank */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /* 37 - Screen blank light stars */
    0x02, 0x02, 0x01, 0x02, 0x00, 0x00, /* 38 - Shadow flag */
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, /* 39 - Blink / High background /
                                                None */
    1,                                  /* FulScr */
    0,                                  /* MenuVs */
    1,                                  /* KeyBar */
    1,                                  /* Prompt */
    1,                                  /* InsDwn */
    0,                                  /* AutoMnu */
    1,                                  /* MiniSta */
    0,                                  /* Clock */
    1,                                  /* AutoCD */
    1,                                  /* AutoSav */
    1,                                  /* ColTyp */
    5,                                  /* BlDelay */
    1,                                  /* LoadFl */
    1,                                  /* UnSlct */
    1,                                  /* ErrSnd */
    0,                                  /* LftMous */
    1,                                  /* ZoomWin */
    1,                                  /* AltBar */
    1,                                  /* EscBar */
    0,                                  /* ExecTyp */
    0,                                  /* AutoSiz */
    1,                                  /* HidDirs */
    1,                                  /* QuitAck */
    2,                                  /* SnowMod */
    1,                                  /* AltView */
    1,                                  /* AltEdit */
    1,                                  /* ClrBuf */
    1,                                  /* Active */
    0,                                  /* Ctrl_O */
    1,                                  /* ChkTSR */
    1,                                  /* ChgTree */
    1,                                  /* DriveB */
    60, 0,                              /* DelayTm: 0.6 seconds */
    0xFA, 0xF9, 0x07, 0x04, 0x0F        /* StarTab: the five characters
                                           that are stars */
};

/* The table goes as far as the bytes that are reserved. */
typedef char Palets_size[sizeof Palets
                         == (u16)&((struct MainDat *)0)->ResIni ? 1 : -1];

/* The panel that the program has without a setup file, as far as its
   mask, which is empty: files in brief, shown, hidden ones too, no
   filter, unsorted. */
static const u8 CODE DefWCB[] = {
    0,                                  /* WinTyp */
    0,                                  /* BrfFul */
    1,                                  /* Visible */
    1,                                  /* Hidden */
    0,                                  /* MasTyp */
    8,                                  /* SortTyp */
    0,                                  /* Ctrl_L */
    0                                   /* DskMas */
};

/* The banner, the two texts that ask for Enter where the screen is not
   one the program can draw on, and what is said of a setup file that is
   not one. The end of the banner, CrLf_S, has no zero byte: it is copied
   as far as the next one, with the text that stands behind it, Retrn_S,
   and DOS writes it as far as the dollar sign. */
static const u8 CODE Suff_S[]  = " freeware";
static const u8 CODE Copyr_S[] =
    "\r\nCopyright (C) 1991-2000 Vsevolod V. Volkov";
static const u8 CODE CrLf_S[]  =
    "\r\n$"
    "\r\nPress ENTER to return \0"
    "\r\nto the Volkov Commander";
#define Retrn_S (CrLf_S + 3)
#define Retr2_S (CrLf_S + 28)
static const u8 CODE NoIni_S[] = "VC.INI is not correct.\r\n";

/* What the environment is searched for, and the mask that the program
   starts with: all of it for the file search, without its backslash for
   Select group. */
static const u8 CODE Comsp[8]   = "COMSPEC=";
static const u8 CODE NcEnv[3]   = "VC=";
static const u8 CODE TempEnv[5] = "TEMP=";
static const u8 CODE Mask_S[]   = "\\*.*";

/* Copy a text with its zero byte. Returns where that is in the copy. */
static u8 __far *MvzTxt(u8 __far *to, const u8 __far *from)
{
    while ((*to = *from++) != 0)
        to++;
    return to;
}

/* Write a text on the screen as a teletype would, on the page `page`:
   its first character whatever it is, and on up to its zero byte. This
   is OutStr of the part that stays, which the assembly called there. */
static void OutStr1(const u8 __far *text, u16 page)
{
    struct BiosRegs regs;

    regs.cx = 0;
    regs.dx = 0;
    do {
        regs.ax = 0x0E00 | *text++;
        regs.bx = page << 8 | 7;
        Int10((struct BiosRegs STACK *)&regs);
    } while (*text != 0);
}

/* Write to `to` the directory that the environment `env` names with
   `name`, of `length` characters, VC= or TEMP=, with a backslash at its
   end. Without the name there it is the directory of the program's file,
   whose path stands behind the environment, or the text at `other` if
   that is not 0, cut off behind its last backslash. Returns where the
   directory ends in `to`.

   The assembly took the text of NcPath for TempDir, and knew TempDir by
   its offset alone, whatever the segment: MainPth at its first call
   writes into the part that stays, at 0B87h in the original, and TempDir
   is at 0B99h of the main data. */
static u8 __far *MnPth0(u8 __far *to, const u8 __far *env,
                        const u8 __far *name, u16 length,
                        const u8 __far *other)
{
    const u8 __far *from;
    u8 __far *end;
    u16 at = 0, i;

    for (;;) {                          /* MnPth1 */
        if (env[at] == 0) {             /* MnPth2: no such name */
            from = env + at + 3;
            if (other != 0)
                from = other;
            for (i = 0; i < LenPath; i++)       /* MnPth9 */
                to[i] = from[i];
            end = (u8 __far *)CutPath(to);
            break;
        }
        for (i = 0; i < length && env[at + i] == name[i]; i++)
            ;
        if (i == length) {              /* MnPth3 */
            end = MvzLine(to, env + at + length, LenPath - 2);
            end = AddFile(end);
            break;
        }
        while (env[at++] != 0)
            ;
    }
    *end = 0;                           /* MnPth4 */
    return end;
}

/* Read a file of the program's directory. The path of the directory
   begins at `path` and ends at `end`, and the 13 bytes at `name` are put
   behind it. Up to `count` bytes of the file are read to `buffer`.
   Returns how many were read, or FAILED if the file could not be opened,
   read or closed. */
static u16 ReadOne(const u8 __far *path, u8 __far *end,
                   const u8 __far *name, u8 __far *buffer, u16 count)
{
    struct DosRegs regs;
    u16 i, failed, length;

    for (i = 0; i < 13; i++)
        end[i] = name[i];
    regs.ax = 0x3D20;                   /* open, to read; others may read */
    regs.bx = 0;
    regs.cx = 0;
    regs.name = path;
    regs.si = 0;
    regs.di = 0;
    if (Int21d((struct DosRegs STACK *)&regs))
        return FAILED;                  /* RdOne1 */
    regs.bx = regs.ax;
    regs.cx = count;
    regs.name = buffer;
    regs.ax = 0x3F00 | regs.ax & 0xFF;
    failed = Int21d((struct DosRegs STACK *)&regs);
    length = regs.ax;
    regs.ax = 0x3E00 | regs.ax & 0xFF;
    if (Int21d((struct DosRegs STACK *)&regs) || failed)
        return FAILED;
    return length;
}

/* Make a panel of what is kept of one while a command runs, at `from`:
   the start of its segment, and of each file its name and its
   attributes, 13 bytes. The rest of what a list says of a file is not
   there: the list is read again. */
static void Expand(u16 panel, const u8 __far *from)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 __far *to = (u8 __far *)wcb;
    struct FILENT __far *file;
    u16 count, i;

    for (i = 0; i < WCBsave; i++)
        *to++ = *from++;
    count = (u16)(wcb->EndBuf - DSKBUF) / sizeof(struct FILENT);
    for (file = wcb->DskBuf; count != 0; count--) {     /* Expnd1 */
        for (i = 0; i < 12; i++)
            file->NameF[i] = *from++;
        file->NameF[12] = 0;
        file->AttrF = *from++;
        file++;
    }
}

/* The program has been started. Read its setup file and its main
   extension file into the part that stays in the memory, at `stays`, and
   make that part ready for the program's first time round: as a command
   would have left it.

   The files are those of the directory that VC= names in the
   environment, or of the program's own. A setup file that is not there
   gives the defaults; one of another length, with a wrong sum or without
   its three letters V gives them too, and is said to be not correct.
   Then the banner. And the program takes the interrupts that it keeps
   while commands run: its end, which DOS finds in the PSP, Ctrl-Break,
   and, if it is to know of programs that stay in the memory, DOS's
   own, 21h and 27h.

   Ported as they are: the setup file is read to where its last part,
   the main data's, lands in ComnIni, so its sum, and one byte more of a
   file that is too long, are in LoadRes, ScrMod and Mouse there until
   they are set. And the text of the banner is written into the buffer
   of the extension file before that is read.

   Not as the original has it: it takes the defaults of a panel up to
   WinPath's end, 80 bytes beyond the 8 that it has for them, and so has
   the start of its table of the keys, twenty keys with the addresses of
   their commands, in the mask and behind the path of both panels,
   behind the zero byte that ends each. Here those 80 bytes are zeros:
   the table is a switch now and has no bytes. Nothing reads them where
   they are, and MkIni1 clears them before the first key, when it holds
   the setup against the one that was saved. */
void Init00(u16 stays)
{
    struct DosRegs regs;
    struct BiosRegs bios;
    struct VideoMode mode;
    struct MainDat __far *saved =
        (struct MainDat __far *)AT(stays, OFFSET(ComnIni));
    struct WCBdef __far *first =
        (struct WCBdef __far *)AT(stays, OFFSET(WCB1sav));
    struct WCBdef __far *second =
        (struct WCBdef __far *)AT(stays, OFFSET(WCB2sav));
    u8 __far *path = AT(stays, OFFSET(WCB1sav) + WCBsave);
    u8 __far *image = AT(stays, OFFSET(ComnIni) - (3 + 2 * WCBini));
    const u8 __far *env = AT(WORD_AT(stays, PSP_ENVIRON), 0);
    u8 __far *end, __far *to;
    u16 __far *vector;
    /* The segment of the table of vectors. Written as a constant, the
       compiler takes a far pointer into segment 0 for a near one. */
    volatile u16 table = 0;
    u16 code = AsmSeg();
    u16 length, sum, drive, wrong, i;

    end = MnPth0(path, env, NcEnv, sizeof NcEnv, 0);
    length = ReadOne(path, end, AT(code, OFFSET(Init_F)), image,
                     INI_LEN + 1);
    wrong = 0;
    if (length == INI_LEN) {
        sum = 0;
        for (i = 0; i < INI_LEN - 2; i++)       /* Init01 */
            sum += image[i];
        if (*(u16 __far *)(image + INI_LEN - 2) == sum
            && image[0] == 'V' && image[1] == 'V' && image[2] == 'V') {
            for (i = 0; i < WCBini; i++) {
                ((u8 __far *)first)[i] = image[3 + i];
                ((u8 __far *)second)[i] = image[3 + WCBini + i];
            }
            goto Init04;
        }
    }
    if (length != FAILED)
        wrong = 1;                      /* Init02: a file, but not one of
                                           the program's */

    /* Init03: the defaults, with both panels on the current directory
       and the first one switched off. */
    for (i = 0; i < sizeof Palets; i++)
        ((u8 __far *)saved)[i] = Palets[i];
    for (; i < CmnIni; i++)
        ((u8 __far *)saved)[i] = 0;
    for (i = 0; i < sizeof DefWCB; i++)
        ((u8 __far *)first)[i] = DefWCB[i];
    for (; i < WCBini; i++)
        ((u8 __far *)first)[i] = 0;
    regs.ax = 0x1900;                   /* the current drive */
    regs.bx = wrong;
    regs.cx = 0;
    regs.name = AT(stays, 0);
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    drive = regs.ax & 0xFF;
    first->WinPath[0] = (u8)(drive + 'A');
    first->WinPath[1] = ':';
    first->WinPath[2] = '\\';
    first->WinPath[3] = 0;
    regs.ax = 0x4700;                   /* its current directory */
    regs.name = AT(stays, drive + 1);
    regs.si = OFFSET(WCB1sav)
              + (u16)(u32)&((struct WCBdef __far *)0)->WinPath[3];
    Int21d((struct DosRegs STACK *)&regs);
    for (i = 0; i < WCBini; i++)
        ((u8 __far *)second)[i] = ((u8 __far *)first)[i];
    first->Visible = 0;

Init04:                                 /* Output Report */
    mode.ax = 0;
    mode.bx = wrong;
    if (!ChkMode((struct VideoMode STACK *)&mode)) {
        bios.ax = 3;                    /* 80 columns, in colour */
        bios.bx = mode.bx;
        bios.cx = 0;
        bios.dx = 0;
        Int10((struct BiosRegs STACK *)&bios);
    }
    to = MvzTxt(saved->ExtMain, AT(code, OFFSET(Title_S) + 2));  /* Init05 */
    to = MvzTxt(to, Suff_S);
    to = MvzTxt(to, Copyr_S);
    MvzTxt(to, CrLf_S);
    regs.ax = 0x0900;                   /* a text, as far as its $ */
    regs.bx = mode.bx;
    regs.cx = 0;
    regs.name = saved->ExtMain;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    if (wrong)                          /* Error in INI file */
        OutStr1(NoIni_S, mode.bx >> 8);

    /* Init09: read main extension file, and end it. */
    to = saved->ExtMain;
    length = ReadOne(path, end, AT(code, OFFSET(Ext_F)), to, LenExt - 1);
    if (length != FAILED)
        to += length;
    *to = 0x1A;                         /* Init10 */

    /* Initialize main data. */
    first->CurAdr = DSKBUF;
    second->CurAdr = DSKBUF;
    first->First = DSKBUF;
    second->First = DSKBUF;
    first->EndBuf = DSKBUF;
    second->EndBuf = DSKBUF;
    WORD_AT(stays, OFFSET(Counter)) = 0;
    saved->CrnPath[0] = 0;
    saved->ViewFil[0] = 0;
    saved->ToFile[0] = 0;
    saved->SrchStr[0] = 0;
    *(u16 __far *)saved->HstBuf = 0;
    saved->LastMnu = 1;
    saved->MouseX = 0xFFFF;
    saved->MouseY = 0xFFFF;
    saved->LoadRes = 0xFF;
    saved->MenuFl = saved->AutoMnu;
    for (i = 0; i < 4; i++)
        saved->SelMas[i] = Mask_S[i + 1];
    for (i = 0; i < 5; i++)
        saved->FindStr[i] = Mask_S[i];
    ((StaysFn *)((u32)stays << 16 | OFFSET(SetBrk)))();
    vector = (u16 __far *)AT(stays, OFFSET(Vect22));
    vector[0] = WORD_AT(stays, PSP_TERMINATE);
    WORD_AT(stays, PSP_TERMINATE) = OFFSET(Exit01);
    vector[1] = WORD_AT(stays, PSP_TERMINATE + 2);
    WORD_AT(stays, PSP_TERMINATE + 2) = stays;
    if (saved->ChkTSR == 0)
        return;
    regs.ax = 0x3521;                   /* where an interrupt goes */
    regs.bx = OFFSET(ComnIni);
    regs.cx = 0;
    regs.name = AT(stays, OFFSET(SetBrk));
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    vector = (u16 __far *)AT(stays, OFFSET(Vect21));
    vector[0] = regs.bx;
    vector[1] = regs.es;
    IntOff();
    WORD_AT(table, 4 * 0x21) = OFFSET(New21);
    WORD_AT(table, 4 * 0x21 + 2) = stays;
    IntOn();
    regs.ax = 0x3527;
    Int21d((struct DosRegs STACK *)&regs);
    vector = (u16 __far *)AT(stays, OFFSET(Vect27));
    vector[0] = regs.bx;
    vector[1] = regs.es;
    regs.ax = 0x2500 | regs.ax & 0xFF;  /* have one go somewhere */
    regs.name = AT(stays, OFFSET(New27));
    Int21d((struct DosRegs STACK *)&regs);
}

/* Ask DOS for the memory of the main data, the two panels and the
   viewer, all that there is, and make the main data and the panels of
   what the part that stays has kept of them, at `stays`. `block` is the
   block of the memory that the code is in, which is first cut down to
   the code: the linker's relocation table, behind it, has been used.
   Returns the segment of the main data, or 0 if there is not enough
   memory for the least that the program needs, and then, in the high
   word, how many paragraphs there were: the assembly has them in BX
   when the part that stays says that there is no memory, where the BIOS
   takes a page of the screen (see AlocEr of resident.c).

   FreMem2 is how much there was to have, in paragraphs. The original
   has a second number, FreMem1: that or, if it is more, how large the
   block of the code could have been made, which is what a command gets
   where the code is given up for it. It asks DOS for that first, by
   making the block of the code as large as it can be; the portable C
   does not ask. */
u32 Init11(u16 stays, u16 block)
{
    struct DosRegs regs;
    struct MainDat __far *data;
    const u8 __far *from;
    u16 code = AsmSeg();
    u16 main_size = ((sizeof(struct MainDat) - 1) >> 4) + 1;
    u16 panel_size = ((sizeof(struct WCBdef) - 1) >> 4) + 1;
    u16 largest, segment, i;

    regs.ax = 0x4A00;                   /* Init12: make a block another
                                           size */
    regs.bx = WORD_AT(stays, OFFSET(RelTab))
        - WORD_AT(stays, OFFSET(TrnPrW));
    regs.cx = 0;
    regs.name = AT(stays, 0);
    regs.si = 0;
    regs.di = 0;
    regs.es = block;
    Int21e((struct DosRegs STACK *)&regs);
    regs.ax = 0x4800 | regs.ax & 0xFF;  /* a block of the memory: of all
                                           of it, which DOS refuses, and
                                           says how large one can be */
    regs.bx = 0xFFFF;
    Int21d((struct DosRegs STACK *)&regs);
    largest = regs.bx;
    regs.ax = 0x4800 | regs.ax & 0xFF;
    Int21d((struct DosRegs STACK *)&regs);
    segment = regs.ax;
    if ((u16)(((MinView - 1) >> 4) + 1 + main_size + 2 * panel_size)
        > largest)
        return (u32)largest << 16;
    data = (struct MainDat __far *)AT(segment, 0);
    data->FreMem2 = largest;            /* Init13 */
    data->WCB1seg = segment + main_size;
    data->WCB2seg = data->WCB1seg + panel_size;
    data->ViewSeg = data->WCB2seg + panel_size;
    WORD_AT(code, OFFSET(SecDat)) = segment;

    /* Restore data segment. */
    from = AT(stays, OFFSET(ComnIni));
    for (i = 0; i < CmnSave; i++)
        ((u8 __far *)data)[i] = from[i];
    Expand(data->WCB1seg, AT(stays, OFFSET(WCB1sav)));
    Expand(data->WCB2seg, AT(stays, OFFSET(WCB2sav)));
    return segment;
}

/* Where the cursor is, into `regs`; and if it is below the last line
   that the program counts, put it at the start of that line. */
static void CurNl(struct BiosRegs STACK *regs)
{
    regs->ax = 0x0F00 | regs->ax & 0xFF;
    Int10(regs);
    regs->ax = 0x0300 | regs->ax & 0xFF;
    Int10(regs);
    if (regs->dx >> 8 >= md.Lines)
        Cursor((u16)(u8)(md.Lines - 1) << 8);
}

/* The main data is there, with what was kept of it. Fill in the rest:
   what the machine and DOS say of themselves, and what begins anew
   every time the program has come back.

   The critical error of DOS and Ctrl-Break are taken. A screen that is
   not text of 80 columns is one that a command has left: the program
   asks for Enter and sets the mode it had. The cursor gets a shape if
   it has none or one that a BIOS is known to report wrongly. The
   country, and with it a table of the small letters beyond ASCII, made
   by asking DOS for the capital of each. The environment is searched
   for COMSPEC=, and for VC= and TEMP=; the original also notes where
   the path of the program's own file is behind it, OvlPth, to load
   itself again after a command. Then the display: an EGA or VGA and its rows,
   whether writing to it has to wait for the beam, where its memory is,
   and whether TopView or DESQview shows a buffer in its place; the
   version of DOS; and the colour in which DOS writes, found by having
   it write a space.

   Ported as they are:
     - The program handed to a command, by COMMAND.COM, gets its two
       FCBs at 5Ch and 6Ch of the main data, not of the PSP: ES is on
       the main data when the block for running a program is filled
       in.
     - For snow the status of the display is read until its lowest bit
       is set seven times running, with interrupts off, in 32,767 tries.
       That is timed by its instructions, and the compiled ones are
       others: on the display it was written for, a CGA, the bit is set
       for all of the time that the beam returns to the top, which is
       longer than seven readings take either way.
     - TopView's buffer is taken at the offset of the display's page,
       whatever offset TopView answers with.
     - A cursor below the last line is put on it before the screen is
       fed a line, so the screen scrolls once. */
void IniData(void)
{
    struct DosRegs regs;
    struct BiosRegs bios;
    struct VideoMode mode;
    u16 stays = ResSeg();
    const u8 __far *env;
    u16 __far *shown;
    u16 i, c, key, at, lines, rows, snow, tries, count, segment, version,
        other, cell;

    bios.ax = HookVct();                /* Set Error Vectors */

    mode.ax = bios.ax;                  /* Set Display Segment */
    mode.bx = md.FreMem2;
    bios.cx = 0;
    bios.dx = 0;
    if (ChkMode((struct VideoMode STACK *)&mode)) {
        bios.ax = mode.ax;
        bios.bx = mode.bx;
    } else {
        bios.ax = 0x0100 | mode.ax & 0xFF;      /* no cursor */
        bios.bx = mode.bx & 0xFF00 | mode.ax >> 8;
        bios.cx = 0x2000;
        Int10((struct BiosRegs STACK *)&bios);
        OutStr1(Retrn_S, bios.bx >> 8);
        if ((bios.bx & 0xFF) < 80)
            OutStr1(Retr2_S, bios.bx >> 8);
        else                            /* on the same line */
            OutStr1(Retr2_S + 2, bios.bx >> 8);
        do                              /* Init16 */
            key = Int16(0).key;
        while ((key & 0xFF) != '\r');
        bios.ax = md.ScrMod;
        Int10((struct BiosRegs STACK *)&bios);
        bios.ax = 0x0F00 | bios.ax & 0xFF;
        Int10((struct BiosRegs STACK *)&bios);
        bios.ax &= 0xFF7F;
    }
    md.ScrMod = (u8)bios.ax;            /* Init17 */
    md.ScrPage = (u8)(bios.bx >> 8);

    /* Correct cursor size. */
    bios.ax = 0x0300 | bios.ax & 0xFF;
    Int10((struct BiosRegs STACK *)&bios);
    if (bios.cx >> 8 > 0x0F)
        bios.cx = 0x0607;
    if (bios.cx == 0x0067)              /* Init18 */
        bios.cx = 0x0607;
    if (md.ScrMod == 7 && bios.cx == 0x0607)    /* Init19 */
        bios.cx = 0x0B0C;
    bios.ax = 0x0100 | bios.ax & 0xFF;  /* Init20 */
    Int10((struct BiosRegs STACK *)&bios);

    /* Initialize data segment. */
    regs.ax = 0x3800;                   /* Country */
    regs.bx = bios.bx;
    regs.cx = bios.cx;
    regs.name = (const u8 *)&md.Country;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    for (i = 0; i < 128; i++) {         /* Lower case table */
        md.LowTab[i] = (u8)(0x80 + i);
        md.TmpBuf[i] = (u8)CaseMap(regs.ax & 0xFF00 | 0x80 + i);
    }
    for (i = 0; i < 128; i++) {         /* Init22 */
        c = md.TmpBuf[i];
        if (c != 0x80 + i && c >= 0x80)
            md.LowTab[c - 128] = (u8)(0x80 + i);
    }
    md.LoadFl &= 0x7F;
    md.Memorys = WORD_AT(stays, PSP_MEMORY);
    *AT(stays, OFFSET(WCB1sav)) = 0;    /* IniSave: no setup is known to
                                           have been saved */
    md.DosBuf[0] = 0;
    md.EPB.EnvSeg = 0;
    md.EPB.FCB1 = (u32)MAIN_SEG << 16 | 0x5C;
    md.EPB.FCB2 = (u32)MAIN_SEG << 16 | 0x6C;
    env = AT(WORD_AT(stays, PSP_ENVIRON), 0);
    at = 0;                             /* ComSpec */
    for (;;) {                          /* Init24 */
        if (env[at] == 0) {
            at = 0;                     /* Init25 */
            break;
        }
        for (i = 0; i < sizeof Comsp && env[at + i] == Comsp[i]; i++)
            ;
        at += i;
        if (i == sizeof Comsp)
            break;
        while (env[at++] != 0)
            ;
    }
    md.ComSpec = at;                    /* Init26 */
    MnPth0(md.NcPath, env, NcEnv, sizeof NcEnv, 0);
    PathLin(md.NcPath);
    MnPth0(md.TempDir, env, TempEnv, sizeof TempEnv, md.NcPath);
    md.DosCol = 0;
    md.BegCol = 0;
    md.OutCol = 0;
    md.Quote = 0;
    md.SftErr = 0;
    md.BlocEr = 0;
    md.TstEGA = 0;
    md.WinMous = 0;
    md.WinLeav = 0;
    md.HstAdr = (u16)md.HstBuf;
    md.BarPrev = 0xFF;
    regs.ax = 0x1900;                   /* DosChg */
    Int21d((struct DosRegs STACK *)&regs);
    md.DosChg[0] = (u8)regs.ax;
    bios.ax = 0x1200 | regs.ax & 0xFF;  /* TstEGA */
    bios.bx = 0xFF10;
    Int10((struct BiosRegs STACK *)&bios);
    if (bios.bx != 0xFF10)
        md.TstEGA = 1;
    lines = 25;                         /* Init29: Lines */
    bios.dx &= 0xFF00;
    if (md.TstEGA != 0) {
        bios.bx &= 0x00FF;
        bios.ax = 0x1130;
        Int10((struct BiosRegs STACK *)&bios);
        rows = bios.dx & 0xFF;
        if (rows != 0) {
            rows = (u8)(rows + 1);
            lines = rows;
            if (rows >= MaxLins)
                lines = MaxLins;
        }
    }
    md.Lines = (u8)lines;               /* Init30 */
    snow = 1;                           /* Snow */
    if (md.ScrMod != 7 && md.SnowMod != 1
        && (md.SnowMod < 1 || md.TstEGA == 0)) {
        tries = 0x8000;
        for (;;) {                      /* Init31 */
            IntOn();
            if (--tries == 0)
                break;
            IntOff();
            for (count = 7; count != 0; count--)    /* Init32 */
                if (!(InPort(0x3DA) & 1))
                    break;
            if (count == 0) {
                snow = 0;
                break;
            }
        }
    }
    IntOn();                            /* Init33 */
    md.Snow = (u8)snow;
    segment = 0xB800;                   /* Screen Segment */
    if (md.ScrMod == 7)
        segment = 0xB000;
    shown = TopScr((u16 __far *)AT(segment, ScrOffs()));   /* Init34 */
    md.TopView = 0;
    if ((u16)((u32)shown >> 16) != segment) {
        md.TopView = 1;
        md.Snow = 1;
    }
    *((u16 *)&md.Screen + 1) = (u16)((u32)shown >> 16);    /* Init35 */
    regs.ax = 0x2B01;                   /* Check DESQview: a date that
                                           DOS refuses */
    regs.bx = segment;
    regs.cx = 0x4445;                   /* 'DE' */
    regs.name = (const u8 __far *)&md + 0x5351;     /* 'SQ' */
    Int21d((struct DosRegs STACK *)&regs);
    md.DesqVie = (u8)(regs.ax + 1);
    regs.ax = 0x3306;                   /* VerDOS: the version that DOS
                                           has, whatever it says to a
                                           program that asks otherwise */
    regs.bx = 0;
    Int21d((struct DosRegs STACK *)&regs);
    version = regs.bx;
    other = regs.ax;
    if (version == 0) {
        regs.ax = 0x3000;
        regs.bx = other;
        Int21d((struct DosRegs STACK *)&regs);
        version = regs.ax;
        other = regs.bx;
    }
    if (version == 4 && other >> 8 == 0xFF)     /* Init36 */
        version = 0x0104;
    md.VerDOS = version;                /* Init37 */
    PANEL(md.WCB1seg)->ChgFlg = 1;      /* WCBseg */
    PANEL(md.WCB2seg)->ChgFlg = 1;
    md.WCBcrn = md.WCB1seg;
    md.WCBsec = md.WCB2seg;
    if (md.Active != 0) {
        md.WCBcrn = md.WCB2seg;
        md.WCBsec = md.WCB1seg;
    }

    /* Set DOS color. */
    CurNl((struct BiosRegs STACK *)&bios);
    bios.ax = 0x0E0D;
    Int10((struct BiosRegs STACK *)&bios);
    bios.ax = 0x0E0A;
    Int10((struct BiosRegs STACK *)&bios);
    CurNl((struct BiosRegs STACK *)&bios);
    bios.ax = 0x0800 | bios.ax & 0xFF;  /* the character at the cursor */
    Int10((struct BiosRegs STACK *)&bios);
    cell = bios.ax;
    regs.ax = 0x0600 | cell & 0xFF;     /* DOS writes a space there */
    regs.bx = bios.bx;
    regs.cx = bios.cx;
    bios.dx = bios.dx & 0xFF00 | ' ';
    regs.name = (const u8 __far *)&md + bios.dx;
    Int21d((struct DosRegs STACK *)&regs);
    bios.ax = 0x0E0D;
    Int10((struct BiosRegs STACK *)&bios);
    bios.ax = 0x0800 | bios.ax & 0xFF;  /* and its colour is DOS's */
    Int10((struct BiosRegs STACK *)&bios);
    md.CrntCol = (u8)(bios.ax >> 8);
    bios.ax = 0x0900 | cell & 0xFF;     /* what was there before */
    bios.bx = bios.bx & 0xFF00 | cell >> 8;
    bios.cx = 1;
    Int10((struct BiosRegs STACK *)&bios);
}
