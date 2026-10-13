/*
 * The start of the program: the setup file read, or the defaults taken
 * where there is none that will do; the banner; the memory for the
 * panels and the viewer asked of DOS; and what the program finds out
 * about the machine before it draws anything.
 *
 * Translated from VC.ASM.
 *
 * The program runs through this once from Init00, when it is started:
 * Init00, which reads the setup, Init11, which asks DOS for the memory
 * and makes the panels, and IniData, a name that the assembly does not
 * have, which is the rest, as far as the main loop (see main.c). After
 * every command it comes back to IniData: a command may have left the
 * screen, the country and the directories anything.
 *
 * The original comes back to Init11 instead. It gives its main data
 * back to DOS while a command runs, and keeps a copy of the setup, of
 * the command line and of what the panels show in the part of it that
 * stays in the memory: Init11 asks for the memory again and makes the
 * main data and the panels of the copy. So its Init00 reads the setup
 * to where that copy is, with no main data yet. The portable C stays in
 * the memory whole (see DEVIATIONS.md): its main data is of the
 * program, it has no copy, and for a command it gives DOS back only
 * the memory behind the panels, which GiveMem and TakeMem do here.
 *
 * Main of main.c calls them all.
 */
#include "vc.h"

/* Init00, Init11, IniData, GiveMem and TakeMem are declared in vc.h:
   Main of main.c calls them. */

/* AsmSeg, of vc.h, returns the segment of the program's tables. */

/* What this reads of other files: in the segment of the tables, the
   names of the setup file and of the main extension file, 13 bytes each
   with what follows them; and the setup as it was last saved or found,
   of setup.c. */
extern const u8 __far Init_F[];
extern const u8 __far Ext_F[];
extern u8 IniSave[];

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define AT(segment, offset) ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))

/* The PSP: where memory that DOS has ends, and the environment. */
#define PSP_MEMORY    0x02
#define PSP_ENVIRON   0x2C

#define MinView 8000                    /* Minimum view buffer size */

/* How many paragraphs the viewer takes at the least. */
#define VIEW_PARAS  (((MinView - 1) >> 4) + 1)

/* Where Init00 has the setup file as it read it, or the defaults of the
   two panels in its place, until Init11 makes the panels of that: in
   MenuBuf, which nothing else uses in between. */
#define IMAGE (md.MenuBuf)

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
                         == OFFSET_IN(struct MainDat, ResIni) ? 1 : -1];

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
   is OutStr of resident.c, which the assembly called there. */
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
   writes into the part of the original that stays in the memory, at
   0B87h, and TempDir is at 0B99h of the main data. */
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

/* The program has been started. Read its setup file and its main
   extension file, and make the program ready for its first time round.

   The files are those of the directory that VC= names in the
   environment, or of the program's own. A setup file that is not there
   gives the defaults; one of another length, with a wrong sum or without
   its three letters V gives them too, and is said to be not correct.
   Then the banner. And the program takes the interrupts that it keeps
   while commands run: its end, which DOS finds in the PSP, Ctrl-Break,
   and, if it is to know of programs that stay in the memory, DOS's
   own, 21h and 27h.

   The panels are not there yet, since the original asks DOS for their
   memory after the banner, and says after it that there is none. What
   the file has of them, or their defaults, is left at IMAGE for Init11.

   Ported as it is: the text of the banner is written into the buffer
   of the extension file before that is read.

   Not as the original has it: it takes the defaults of a panel up to
   WinPath's end, 80 bytes beyond the 8 that it has for them, and so has
   the start of its table of the keys, twenty keys with the addresses of
   their commands, in the mask and behind the path of both panels,
   behind the zero byte that ends each. Here those 80 bytes are zeros:
   the table is a switch now and has no bytes. Nothing reads them where
   they are, and MkIni1 clears them before the first key, when it holds
   the setup against the one that was saved. And it reads the file to
   where its last part lands in its copy of the main data, so that the
   file's sum, and one byte more of a file that is too long, are in
   LoadRes, ScrMod and Mouse until those are set; here the file is read
   to IMAGE, and the three are as the program was loaded. */
void Init00(void)
{
    struct DosRegs regs;
    struct BiosRegs bios;
    struct VideoMode mode;
    u8 *image = IMAGE;
    struct WCBdef *first = (struct WCBdef *)(image + 3);
    struct WCBdef *second = (struct WCBdef *)(image + 3 + WCBini);
    u8 *path = md.TmpBuf;
    const u8 __far *env = AT(WORD_AT(PspSeg, PSP_ENVIRON), 0);
    u8 __far *end, __far *to;
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
        if (*(u16 *)(image + INI_LEN - 2) == sum
            && image[0] == 'V' && image[1] == 'V' && image[2] == 'V') {
            for (i = 0; i < CmnIni; i++)
                ((u8 *)&md)[i] = image[3 + 2 * WCBini + i];
            goto Init04;
        }
    }
    if (length != FAILED)
        wrong = 1;                      /* Init02: a file, but not one of
                                           the program's */

    /* Init03: the defaults, with both panels on the current directory
       and the first one switched off. */
    for (i = 0; i < sizeof Palets; i++)
        ((u8 *)&md)[i] = Palets[i];
    for (; i < CmnIni; i++)
        ((u8 *)&md)[i] = 0;
    for (i = 0; i < sizeof DefWCB; i++)
        ((u8 *)first)[i] = DefWCB[i];
    for (; i < WCBini; i++)
        ((u8 *)first)[i] = 0;
    regs.ax = 0x1900;                   /* the current drive */
    regs.bx = wrong;
    regs.cx = 0;
    regs.name = (const u8 __far *)&md;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    drive = regs.ax & 0xFF;
    first->WinPath[0] = (u8)(drive + 'A');
    first->WinPath[1] = ':';
    first->WinPath[2] = '\\';
    first->WinPath[3] = 0;
    regs.ax = 0x4700;                   /* its current directory */
    regs.name = AT(MAIN_SEG, drive + 1);
    regs.si = (u16)&first->WinPath[3];
    Int21d((struct DosRegs STACK *)&regs);
    for (i = 0; i < WCBini; i++)
        ((u8 *)second)[i] = ((u8 *)first)[i];
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
    to = MvzTxt(md.ExtMain, AT(code, OFFSET(Title_S) + 2));     /* Init05 */
    to = MvzTxt(to, Suff_S);
    to = MvzTxt(to, Copyr_S);
    MvzTxt(to, CrLf_S);
    regs.ax = 0x0900;                   /* a text, as far as its $ */
    regs.bx = mode.bx;
    regs.cx = 0;
    regs.name = md.ExtMain;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    if (wrong)                          /* Error in INI file */
        OutStr1(NoIni_S, mode.bx >> 8);

    /* Init09: read main extension file, and end it. */
    to = md.ExtMain;
    length = ReadOne(path, end, AT(code, OFFSET(Ext_F)), to, LenExt - 1);
    if (length != FAILED)
        to += length;
    *to = 0x1A;                         /* Init10 */

    /* Initialize main data. */
    Counter = 0;
    md.CrnPath[0] = 0;
    md.ViewFil[0] = 0;
    md.ToFile[0] = 0;
    md.SrchStr[0] = 0;
    *(u16 *)md.HstBuf = 0;
    md.LastMnu = 1;
    md.MouseX = 0xFFFF;
    md.MouseY = 0xFFFF;
    md.LoadRes = 0xFF;
    md.MenuFl = md.AutoMnu;
    for (i = 0; i < 4; i++)
        md.SelMas[i] = Mask_S[i + 1];
    for (i = 0; i < 5; i++)
        md.FindStr[i] = Mask_S[i];
    SetBrk();
    TakeDos();
}

/* What DOS has given the program for its panels and the viewer: the
   block at `segment`, of `size` paragraphs. */
static void Memory(u16 segment, u16 size)
{
    md.MemSeg = segment;
    md.FreMem2 = segment + size - MAIN_SEG;
}

/* Ask DOS for the memory of the two panels and the viewer, all that
   there is, and make the panels of what Init00 has left of them at
   IMAGE: each with room for the fewest files, one behind the other,
   without files, and the viewer's memory behind them. Answers with how
   many paragraphs there were, and whether that is too few for the least
   that the program needs: the assembly has them in BX where it says
   that there is no memory, and the BIOS takes a page of the screen
   there (see AlocEr of resident.c).

   The original asks for its main data here too, which is the start of
   the same block: FreMem2 is the paragraphs of that block there, and
   here as many as it is from the main data to the end of the block. */
struct Answer Init11(void)
{
    struct DosRegs regs;
    struct Answer answer;
    struct WCBdef __far *wcb;
    const u8 *from = IMAGE + 3;
    u16 largest, segment, panel, i;

    regs.ax = 0x4800;                   /* a block of the memory: of all
                                           of it, which DOS refuses, and
                                           says how large one can be */
    regs.bx = 0xFFFF;
    regs.cx = 0;
    regs.name = (const u8 __far *)&md;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    largest = regs.bx;
    regs.ax = 0x4800 | regs.ax & 0xFF;
    Int21d((struct DosRegs STACK *)&regs);
    segment = regs.ax;
    answer.value = largest;
    answer.failed = (u16)(VIEW_PARAS + 2 * PANEL_PARAS) > largest;
    if (answer.failed)
        return answer;
    Memory(segment, largest);           /* Init13 */
    md.WCB1seg = segment;
    md.WCB2seg = md.WCB1seg + PANEL_PARAS;
    md.ViewSeg = md.WCB2seg + PANEL_PARAS;
    for (panel = md.WCB1seg; panel != md.ViewSeg; panel += PANEL_PARAS) {
        wcb = PANEL(panel);
        for (i = 0; i < WCBini; i++)
            ((u8 __far *)wcb)[i] = *from++;
        wcb->CurAdr = DSKBUF;
        wcb->First = DSKBUF;
        wcb->EndBuf = DSKBUF;
    }
    return answer;
}

/* Give DOS the memory behind the panels back, for the commands that are
   about to be run. F21_31 of enter.c has left each panel with no more
   files than one has room for at the least: the panel that is behind
   the other is put right behind it, as the two are when the program
   starts, and the block is made as long as the two.

   The original gives the whole block back, the main data with it, and
   keeps of each panel what it will make it of again: Init11 there, and
   in DEVIATIONS.md what follows from the difference. */
void GiveMem(void)
{
    struct DosRegs regs;
    u16 first = md.WCB1seg, second = md.WCB2seg;
    u16 behind, i;
    const u8 __far *from;
    u8 __far *to;

    if (second < first) {
        first = md.WCB2seg;
        second = md.WCB1seg;
    }
    behind = first + PANEL_PARAS;
    if (second != behind) {
        from = (const u8 __far *)PANEL(second);
        to = (u8 __far *)PANEL(behind);
        for (i = 0; i < sizeof(struct WCBdef); i++)
            to[i] = from[i];
        if (second == md.WCB1seg)
            md.WCB1seg = behind;
        else
            md.WCB2seg = behind;
    }
    md.ViewSeg = behind + PANEL_PARAS;
    regs.ax = 0x4A00;                   /* make a block another size */
    regs.bx = md.ViewSeg - md.MemSeg;
    regs.cx = regs.si = regs.di = 0;
    regs.name = (const u8 __far *)&md;
    regs.es = md.MemSeg;
    Int21e((struct DosRegs STACK *)&regs);
}

/* Take all the memory that there is again, when the commands have been
   run: answers as Init11 does.

   The original has given its block back to DOS for the commands, and
   asks for the largest that there is now, wherever that is: where its
   own was, as a rule, and behind a program that a command has left in
   the memory if that is where the most is. This does the same, to have
   its memory where the original has it. The block of the panels is
   given back, and the largest that DOS has is asked for: the same block
   again, longer, or another, to which the panels are then moved. They
   are read where they were, in memory that is DOS's for that moment and
   that nothing has been given since. */
struct Answer TakeMem(void)
{
    struct DosRegs regs;
    struct Answer answer;
    u16 kept = md.ViewSeg - md.MemSeg;
    u16 largest, segment, i;
    const u8 __far *from;
    u8 __far *to;

    regs.ax = 0x4900;                   /* give a block back */
    regs.bx = regs.cx = regs.si = regs.di = 0;
    regs.name = (const u8 __far *)&md;
    regs.es = md.MemSeg;
    Int21e((struct DosRegs STACK *)&regs);
    regs.ax = 0x4800;                   /* the largest block, asked for
                                           as Init11 asks */
    regs.bx = 0xFFFF;
    Int21d((struct DosRegs STACK *)&regs);
    largest = regs.bx;
    regs.ax = 0x4800 | regs.ax & 0xFF;
    Int21d((struct DosRegs STACK *)&regs);
    segment = regs.ax;
    answer.value = largest;
    answer.failed = (u16)(VIEW_PARAS + kept) > largest;
    if (answer.failed)
        return answer;
    if (segment != md.MemSeg) {
        /* From the start: where the two overlap, the new block begins
           in front of the old one. */
        from = AT(md.MemSeg, 0);
        to = AT(segment, 0);
        for (i = kept << 4; i != 0; i--)
            *to++ = *from++;
        md.WCB1seg += segment - md.MemSeg;
        md.WCB2seg += segment - md.MemSeg;
        md.ViewSeg += segment - md.MemSeg;
    }
    Memory(segment, largest);
    return answer;
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

/* The setup is there, and the panels. Fill in the rest of the main
   data: what the machine and DOS say of themselves, and what begins anew
   every time the program has come back from a command. The original
   has made its main data anew then, of what it kept of it, the part
   that the main data has before DosLin; here that part is as it was,
   and so is the rest, where the original has whatever the memory holds
   until this and the main loop set it.

   The critical error of DOS and Ctrl-Break are taken. A screen that is
   not text of 80 columns is one that a command has left: the program
   asks for Enter and sets the mode it had. The cursor gets a shape if
   it has none or one that a BIOS is known to report wrongly. The
   country, and with it a table of the small letters beyond ASCII, made
   by asking DOS for the capital of each. The environment is searched
   for COMSPEC=, and for VC= and TEMP=; the original also notes where
   the path of the program's own file is behind it, OvlPth, to load
   itself again after a command. Then the display: an EGA or VGA and its
   rows, and where its memory is; the original also finds out whether
   writing to it has to wait for the beam, asks whether TopView or
   DESQview shows a buffer in its place, and draws into that, and whether
   DESQview runs. The version of DOS; and the colour in which DOS writes,
   found by having it write a space.

   Ported as they are:
     - The program handed to a command, by COMMAND.COM, gets its two
       FCBs at 5Ch and 6Ch of the main data, not of the PSP: ES is on
       the main data when the block for running a program is filled
       in.
     - A cursor below the last line is put on it before the screen is
       fed a line, so the screen scrolls once. */
void IniData(void)
{
    struct DosRegs regs;
    struct BiosRegs bios;
    struct VideoMode mode;
    const u8 __far *env;
    u16 i, c, key, at, lines, rows, segment, version, other, cell;

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
    md.Memorys = WORD_AT(PspSeg, PSP_MEMORY);
    IniSave[0] = 0;                     /* no setup is known to have been
                                           saved */
    md.DosBuf[0] = 0;
    md.EPB.EnvSeg = 0;
    md.EPB.FCB1 = (u32)MAIN_SEG << 16 | 0x5C;
    md.EPB.FCB2 = (u32)MAIN_SEG << 16 | 0x6C;
    env = AT(WORD_AT(PspSeg, PSP_ENVIRON), 0);
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
    segment = 0xB800;                   /* Screen Segment */
    if (md.ScrMod == 7)
        segment = 0xB000;
    ScrOffs();                          /* Init34: Screen is the memory
                                           of the display, at the page
                                           that is shown */
    *((u16 *)&md.Screen + 1) = segment;
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
