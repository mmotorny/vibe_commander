/*
 * The start of the program: the setup file read, or the defaults taken
 * where there is none that will do; the banner; the memory for the
 * panels and the room asked for; and what the program finds out about
 * the machine before it draws anything.
 *
 * Translated from VC.ASM.
 *
 * The program runs through this once from Init00, when it is started:
 * Init00, which reads the setup, Init11, which asks for the memory and
 * makes the panels, and IniData, a name that the assembly does not
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
 * program, it has no copy, and for a command it gives back only the
 * room, which GiveMem and TakeMem do here.
 *
 * Main of main.c calls them all.
 */
#include "vc.h"

/* Init00, Init11, IniData, GiveMem and TakeMem are declared in vc.h:
   Main of main.c calls them. */


/* What this reads of other files: in the segment of the tables, the
   names of the setup file and of the main extension file, 13 bytes each
   with what follows them; and the setup as it was last saved or found,
   of setup.c. */
extern const u8 Init_F[];
extern const u8 Ext_F[];
extern u8 IniSave[];


/* Where Init00 has the setup file as it read it, or the defaults of the
   two panels in its place, until Init11 makes the panels of that: in
   MenuBuf, which nothing else uses in between. */
#define IMAGE (md.MenuBuf)

#define FAILED 0xFFFF                   /* ReadOne: nothing was read */

/* The setup that the program has without a setup file: the colours, six
   palettes of each, and what follows them in the main data as far as the
   bytes that are reserved. */
static const u8 Palets[] = {
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
static const u8 DefWCB[] = {
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
   and DOS writes it as far as the dollar sign.

   The banner is two lines, as the original's is: the title, Title_S of
   main_t.c, which names what the program is based on where the original
   had the word of how it was distributed behind its title, Suff_S,
   " freeware"; and the copyright of the original's author, with that of
   this program put behind it. The prompt names this program
   (DEVIATIONS.md, 10). */
static const u8 Copyr_S[] =
    "\r\nCopyright (C) 1991-2000 Vsevolod V. Volkov"
    ", (C) 2026 Maksym Motorny";
static const u8 CrLf_S[]  =
    "\r\n$"
    "\r\nPress ENTER to return \0"
    "\r\nto the Vibe Commander";
#define Retrn_S (CrLf_S + 3)
#define Retr2_S (CrLf_S + 28)
static const u8 NoIni_S[] = "VC.INI is not correct.\r\n";

/* What the environment is searched for, and the mask that the program
   starts with: all of it for the file search, without its backslash for
   Select group. */
static const u8 Comsp[8]   = "COMSPEC=";
static const u8 NcEnv[3]   = "VC=";
static const u8 TempEnv[5] = "TEMP=";
static const u8 Mask_S[]   = "\\*.*";

/* Copy a text with its zero byte. Returns where that is in the copy. */
static u8 *MvzTxt(u8 *to, const u8 *from)
{
    while ((*to = *from++) != 0)
        to++;
    return to;
}

/* Write a text on the screen as a teletype would, on the page `page`:
   its first character whatever it is, and on up to its zero byte. This
   is OutStr of exec.c, which the assembly called there. */
static void OutStr1(const u8 *text, u16 page)
{
    do
        ScreenType(*text++, page, 7);
    while (*text != 0);
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
static u8 *MnPth0(u8 *to, const u8 *env,
                  const u8 *name, u16 length,
                  const u8 *other)
{
    const u8 *from;
    u8 *end;
    u16 at = 0, i;

    for (;;) {                          /* MnPth1 */
        if (env[at] == 0) {             /* MnPth2: no such name */
            from = env + at + 3;
            if (other != 0)
                from = other;
            for (i = 0; i < LenPath; i++)       /* MnPth9 */
                to[i] = from[i];
            end = (u8 *)CutPath(to);
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
static u16 ReadOne(const u8 *path, u8 *end,
                   const u8 *name, u8 *buffer, u16 count)
{
    struct Answer opened, read;
    u16 i;

    for (i = 0; i < 13; i++)
        end[i] = name[i];
    opened = FileOpenBare(path, 0x20);  /* to read; others may read */
    if (opened.failed)
        return FAILED;                  /* RdOne1 */
    read = FileReadBare(opened.value, buffer, count);
    if (FileCloseBare(opened.value).failed || read.failed)
        return FAILED;
    return read.value;
}

/* The program has been started. Read its setup file and its main
   extension file, and make the program ready for its first time round.

   The files are those of the directory that VC= names in the
   environment, or of the program's own. A setup file that is not there
   gives the defaults; one of another length, with a wrong sum or without
   its three letters V gives them too, and is said to be not correct.
   Then the banner. And the program takes what it keeps while commands
   run: Ctrl-C, and its end, which DOS finds in the PSP. The original
   takes DOS's own interrupt too, 21h, which the portable C leaves to
   DOS (see DEVIATIONS.md).

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
    struct Screen screen;
    u8 *image = IMAGE;
    struct WCBdef *first = (struct WCBdef *)(image + 3);
    struct WCBdef *second = (struct WCBdef *)(image + 3 + WCBini);
    u8 *path = md.TmpBuf;
    const u8 *env = ProgramEnvironment();
    u8 *end, *to;
    u16 length, sum, drive, wrong, i;

    end = MnPth0(path, env, NcEnv, sizeof NcEnv, 0);
    length = ReadOne(path, end, Init_F, image,
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
    drive = DriveCurrent();
    first->WinPath[0] = (u8)(drive + 'A');
    first->WinPath[1] = ':';
    first->WinPath[2] = '\\';
    first->WinPath[3] = 0;
    DirCurrentBare(drive + 1, &first->WinPath[3]);
    for (i = 0; i < WCBini; i++)
        ((u8 *)second)[i] = ((u8 *)first)[i];
    first->Visible = 0;

Init04:                                 /* Output Report */
    if (!ChkMode(&screen))
        ScreenSetMode(3);               /* 80 columns, in colour */
    to = MvzTxt(md.ExtMain, Title_S + 2);     /* Init05 */
    to = MvzTxt(to, Copyr_S);
    MvzTxt(to, CrLf_S);
    ConsoleText(md.ExtMain);            /* as far as its $ */
    if (wrong)                          /* Error in INI file */
        OutStr1(NoIni_S, screen.page);

    /* Init09: read main extension file, and end it. */
    to = md.ExtMain;
    length = ReadOne(path, end, Ext_F, to, LenExt - 1);
    if (length != FAILED)
        to += length;
    *to = 0x1A;                         /* Init10 */

    /* Initialize main data. */
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
    TakeEnd();
}

/* The room is the block at `segment`, of `size` paragraphs. */
static void Memory(u16 segment, u16 size)
{
    Room = AT(segment, 0);
    RoomEnd = Room + ((u32)size << 4);
}

/* How long the room is, in paragraphs. The original works that out
   wherever it wants it, of FreMem2 and ViewSeg. */
u16 RoomParas(void)
{
    return (u16)((RoomEnd - Room) >> 4);
}

/* Ask for the memory of the two panels and for the room, and make the
   panels of what Init00 has left of them at IMAGE, without files. Each
   panel has a block of its own, as long as a panel can get, which it
   keeps for as long as the program runs and which is never moved; the
   room is the largest block that there is when the panels have theirs.
   Answers with how many paragraphs there were for the room, or for a
   panel that got none, and whether that is too few for the least that
   the program needs: the assembly has them in BX where it says that
   there is no memory, and the BIOS takes a page of the screen there
   (see AlocEr of exec.c).

   The original asks DOS for one block, the largest that there is, and
   cuts it up itself: its main data at the start, behind it the two
   panels, each as long as its list of files, and the room behind them.
   A panel that grows has the other panel and the room moved up, and one
   that shrinks, down: ReDir of read.c. See DEVIATIONS.md for what
   follows from the difference. */
struct Answer Init11(void)
{
    struct Answer answer;
    struct Block block;
    struct WCBdef *wcb;
    const u8 *from = IMAGE + 3;
    u16 largest, panel, i;

    for (panel = 0; panel < 2; panel++) {
        block = MemTake(PANEL_BLOCK);
        answer.value = block.largest;
        answer.failed = block.failed != 0;
        if (answer.failed)
            return answer;
        wcb = (struct WCBdef *)AT(block.value, 0);
        if (panel == 0)                 /* Init13 */
            LeftPanel = wcb;
        else
            RightPanel = wcb;
        for (i = 0; i < WCBini; i++)
            ((u8 *)wcb)[i] = *from++;
        wcb->CurAdr = DSKBUF;
        wcb->First = DSKBUF;
        wcb->EndBuf = DSKBUF;
    }
    largest = MemTake(0xFFFF).largest;  /* a block of the memory: of all
                                           of it, which is refused, with
                                           how large one can be */
    block = MemTake(largest);
    answer.value = largest;
    answer.failed = ROOM_LEAST > largest;
    if (answer.failed)
        return answer;
    Memory(block.value, largest);
    return answer;
}

/* Give the room back, for the commands that are about to be run. The
   panels keep their blocks. F21_31 of enter.c has left each panel with
   no more files than the original keeps of one.

   The original gives its whole block back to DOS, the main data with
   it, and keeps of each panel what it will make it of again: Init11
   there, and in DEVIATIONS.md what follows from the difference. */
void GiveMem(void)
{
    MemGive(SEG_OF(Room));
}

/* Take the room again, when the commands have been run: the largest
   block that there is now. Answers as Init11 does.

   The original has given its block back to DOS for the commands, and
   asks for the largest that there is now, wherever that is. */
struct Answer TakeMem(void)
{
    struct Answer answer;
    u16 largest;
    struct Block block;

    largest = MemTake(0xFFFF).largest;  /* asked for as Init11 asks */
    block = MemTake(largest);
    answer.value = largest;
    answer.failed = ROOM_LEAST > largest;
    if (answer.failed)
        return answer;
    Memory(block.value, largest);
    return answer;
}

/* If the cursor is below the last line that the program counts, put it
   at the start of that line. Returns the page that is shown, which the
   BIOS is asked for first: the assembly goes on with it in BH. */
static u16 CurNl(void)
{
    u16 page = ScreenState().page;

    if (CursorWhere(page).place >> 8 >= md.Lines)
        Cursor((u16)(u8)(md.Lines - 1) << 8);
    return page;
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
   itself again after a command, and asks DOS for the current drive, for
   DosChg: the drive that the shell is on which it hands its commands to
   with Quick execute commands, a way of running one that the portable C
   has not (see DEVIATIONS.md). Then the display: an EGA or VGA and its
   rows, and where its memory is; the original also finds out whether
   writing to it has to wait for the beam, asks whether TopView or
   DESQview shows a buffer in its place, and draws into that, and whether
   DESQview runs. The version of DOS; and the colour in which DOS writes,
   found by having it write a space.

   Ported as they are:
     - The program handed to a command, by COMMAND.COM, gets its two
       FCBs at 5Ch and 6Ch of the main data, not of the PSP: ES is on
       the main data when the original fills in here the block for
       running a program. That block is made where a program is run
       now, by ProgramRun of sys.h: see Run of dos32.c.
     - A cursor below the last line is put on it before the screen is
       fed a line, so the screen scrolls once. */
void IniData(void)
{
    struct Screen screen;
    struct SysVersion system;
    const u8 *env;
    u16 i, c, at, lines, rows, page, shape, cell;

    HookVct();                          /* Set Error Vectors */

    /* Set Display Segment */
    if (!ChkMode(&screen)) {
        CursorShape(0x2000);            /* no cursor */
        OutStr1(Retrn_S, screen.page);
        if (screen.columns < 80)
            OutStr1(Retr2_S, screen.page);
        else                            /* on the same line */
            OutStr1(Retr2_S + 2, screen.page);
        while ((KeyRead() & 0xFF) != '\r')      /* Init16 */
            ;
        ScreenSetMode(md.ScrMod);
        screen = ScreenState();
        screen.mode &= 0x7F;
    }
    md.ScrMod = (u8)screen.mode;        /* Init17 */
    md.ScrPage = (u8)screen.page;

    /* Correct cursor size. */
    shape = CursorWhere(screen.page).shape;
    if (shape >> 8 > 0x0F)
        shape = 0x0607;
    if (shape == 0x0067)                /* Init18 */
        shape = 0x0607;
    if (md.ScrMod == 7 && shape == 0x0607)      /* Init19 */
        shape = 0x0B0C;
    CursorShape(shape);                 /* Init20 */

    /* Initialize data segment. */
    CountryInfo(&md.Country);           /* Country */
    for (i = 0; i < 128; i++) {         /* Lower case table */
        md.LowTab[i] = (u8)(0x80 + i);
        md.TmpBuf[i] = (u8)CaseMap(0x80 + i);
    }
    for (i = 0; i < 128; i++) {         /* Init22 */
        c = md.TmpBuf[i];
        if (c != 0x80 + i && c >= 0x80)
            md.LowTab[c - 128] = (u8)(0x80 + i);
    }
    md.LoadFl &= 0x7F;
    IniSave[0] = 0;                     /* no setup is known to have been
                                           saved */
    md.DosBuf[0] = 0;
    env = ProgramEnvironment();
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
    HistoryPlace = md.HstBuf;
    md.BarPrev = 0xFF;
    if (ScreenEga())                    /* TstEGA */
        md.TstEGA = 1;
    lines = 25;                         /* Init29: Lines */
    if (md.TstEGA != 0) {
        rows = ScreenFont().last;
        if (rows != 0) {
            rows = (u8)(rows + 1);
            lines = rows;
            if (rows >= MaxLins)
                lines = MaxLins;
        }
    }
    md.Lines = (u8)lines;               /* Init30 */
    ScrOffs();                          /* Init34: where the cells of
                                           the screen are, for the mode
                                           and the page that is shown */
    system = SystemTrueVersion();       /* VerDOS: the version that DOS
                                           has, whatever it says to a
                                           program that asks otherwise */
    if (system.version == 0)
        system = SystemVersion();
    if (system.version == 4 && system.other >> 8 == 0xFF)       /* Init36 */
        system.version = 0x0104;
    md.VerDOS = system.version;         /* Init37 */
    LeftPanel->ChgFlg = 1;              /* WCBseg */
    RightPanel->ChgFlg = 1;
    CurrentPanel = LeftPanel;
    OtherPanel = RightPanel;
    if (md.Active != 0) {
        CurrentPanel = RightPanel;
        OtherPanel = LeftPanel;
    }

    /* Set DOS color. The carriage returns and the line feed are typed
       with no colour, which the BIOS takes only where the screen is
       graphics: the assembly has in BL there what DOS left when it was
       asked for its version. */
    page = CurNl();
    ScreenType('\r', page, 0);
    ScreenType('\n', page, 0);
    page = CurNl();
    cell = ScreenCell(page);            /* the character at the cursor */
    ConsoleChar(' ');                   /* DOS writes a space there */
    ScreenType('\r', page, 0);
    md.CrntCol = (u8)(ScreenCell(page) >> 8);   /* and its colour is
                                                   DOS's */
    ScreenFill(page, cell & 0xFF, cell >> 8, 1);        /* what was there
                                                           before */
}
