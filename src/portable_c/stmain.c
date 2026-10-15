/*
 * The setup: entered from the Options menu and left again, its setup
 * file read and written, its menu, everything set to what the program
 * has without a setup file, and the question whether to save.
 *
 * Translated from VCSETUP.ASM.
 *
 * The original has the setup as a program of its own, VCSETUP, which a
 * user runs from the command line of the main program or from DOS. It
 * reads the setup file, VC.INI, and edits what it has read: the setup of
 * a main program that is running is not looked at, and gets what was
 * saved only when the setup program has been left, by a call of DOS that
 * the main program answers. Here the setup is a part of the one program,
 * an item of its Options menu, and StSetup below is all of it:
 *
 *   - The screen is made what it is when a command is run: what DOS has
 *     shown, without the panels and the bars, and a line of the screen
 *     gone by. The setup program then is on the screen as the original
 *     is when it has been run from the program's command line, but for
 *     the line of that command (see DEVIATIONS.md).
 *   - What the program has of its setup, the first CmnIni bytes of the
 *     main data, is set aside, and the setup file is read into their
 *     place: the main data is what the setup program has in its one
 *     segment, and the routines that both programs have, the boxes, the
 *     dialogs and the bars, go by what is edited, as in the original.
 *   - Left without saving, the program has its setup back as it was.
 *     Left with it, the file is written and the program keeps what was
 *     edited, but for what the original's running copy keeps as it has
 *     it: StExit says which.
 *   - Either way the program goes on as when a command has ended: the
 *     screen is looked at anew and the panels are read again.
 *
 * Of the code with which the setup program starts and ends, only what
 * the one program has not done already is here. It has its directory,
 * the country, the screen and the two interrupts, and DOS does not look
 * for Ctrl-C at every call. The banner, the word on the version of DOS
 * and on the memory are of a program of its own.
 *
 * The setup file is asked for as the rest of the program asks for its
 * files, with the program's errors (see sys.h), where the original's
 * setup program calls DOS itself: a drive that is not ready has the
 * program's box. That is declared too.
 *
 * What the items of the menu do is of other files, one for each kind:
 * stopts.c for the options and stcolor.c for the palettes; and the
 * help is of sthelp.c.
 */
#include "vc.h"

/* What the setup program has of its own: struct StData of vc.h. */
#define sd (*(struct StData *)md.MenuBuf)

/* The parts of the setup file, where it is read to and written from:
   the two panels, each as far as a WCBdef is in the file, the setup that
   is edited, and the sum of all the bytes before it. */
#define WCB1   ((struct WCBdef *)(sd.File + 3))
#define WCB2   ((struct WCBdef *)(sd.File + 3 + WCBini))
#define COMMON (sd.File + 3 + 2 * WCBini)
#define SUM    (*(u16 *)(sd.File + INI_LEN - 2))

#define Hist_C  23                      /* the colours of the menu: a line, */
#define HstCu_C 24                      /* and the one under the cursor */

#define KEY_MOUSE 0xFC                  /* the scan code that Input has
                                           for the mouse */

#define ERR_NO_FILE 2
#define ARCHIVE     0x20                /* the attribute of a file that
                                           has changed */

/* Routines of the rest of the program that this file calls: all are of
   vc.h, and those that the setup program has too are the main program's
   word for word, or go by StOpen where the setup program's copy does
   something else that shows: PutKey0, Dialog and Input.

   Quit puts into the screen buffer what DOS had on the screen, and
   ClsLine empties a row of it. SavMous notes where the mouse pointer is,
   for the main program to put it back, and CurMous shows or hides it.
   RestVct gives the two interrupts that the program has taken back to
   whoever had them.

   DialBox draws a box and says where it is, Dialog asks a question and
   answers with the button in the low byte, 0 for the first and FFh for
   Esc; PutKey shows a key bar, Input waits for a key. What the file asks
   of the system it asks by the functions of sys.h. */

/* F32_70 of usermenu.c, which vc.h has, is what the mouse does in a
   list that is in a box. The setup program has it for its menu under a
   name of its own, Main50, and word for word: see StMain00. */

/* The tables of stmain_t.c that the code names. */
extern const u8 StPalets[];             /* the setup without a file */
extern const struct BOXHD StIniEr_B;    /* the setup file is missing or
                                           wrong, */
extern const u8 StNoFnd_S[];            /* with one of these for its */
extern const u8 StNoIni_S[];            /* second line */
extern const struct BOXTOP StMain_P;    /* the menu */
extern const struct BOXHD StDeflt_B;    /* "reset all options?" */
extern const struct BOXHD StSvSt_B;     /* "save the current setup?" */

/* Not 0 while the setup is open: see vc.h. */
bool StOpen;

/* The panel that the program has without a setup file: files in brief,
   shown, hidden ones too, no filter, unsorted, no mask, and the root of
   the drive A:, whose letter StDefault makes that of the current drive.
   The original copies as much from here as the file has of a panel,
   WCBini bytes, which is 64 more than this is: see StDefault. */
static const u8 StDefWCB[24] = {
    0,                                  /* WinTyp */
    0,                                  /* BrfFul */
    1,                                  /* Visible */
    1,                                  /* Hidden */
    0,                                  /* MasTyp */
    8,                                  /* SortTyp */
    0,                                  /* Ctrl_L */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      /* DskMas */
    'A', ':', '\\', 0                   /* WinPath */
};

/* The name of the setup file. The original puts 13 bytes from here
   behind the directory of the program: the name, and what follows it
   there, which is its zero byte, the name of the environment's variable
   and the start of a text that nothing reads. */
static const u8 StInit_F[13] NONSTRING = "VC.INI\0VC=\0So";

/* The page of the setup's help for each item of the menu. */
static const u8 StMainHlp[9] = { 1, 2, 3, 4, 4, 4, 5, 6, 7 };

/* What is said, through DOS, when the setup file could not be written:
   the line ends as DOS's call for writing a text has it, with a dollar
   sign, and has no zero byte. */
static const u8 StStErr_S[49] NONSTRING =
    "There was an error while saving the setup file\r\n$";

/*------------- What DOS had on the screen -------------*/

/* A row of what DOS had on the screen when the setup was entered:
   UserScr of the setup program, which has 50 rows. The main data's has
   the 49 that the main program keeps, so the last of 50 is with the
   setup's own data. */
static u16 *StUserScr(i32 row)
{
    if (row < MaxLins - 1)
        return md.UserScr + row * 80;
    return sd.Row50;
}

/* Keep what the screen buffer has, all the rows of the screen, as what
   DOS had on the screen. */
static void StKeep(void)
{
    const u16 *from = md.ScrBuf;
    u16 *to;
    i32 row, i;

    for (row = 0; row < md.Lines; row++) {
        to = StUserScr(row);
        for (i = 0; i < 80; i++)
            *to++ = *from++;
    }
}

/* Put into the screen buffer what DOS had on the screen: `rows` rows of
   it, from the row `first` on. */
void StUser(i32 first, i32 rows)
{
    u16 *to = BasAdr((u16)(first << 8));
    const u16 *from;
    i32 i;

    for (; rows != 0; rows--) {
        from = StUserScr(first++);
        for (i = 0; i < 80; i++)
            *to++ = *from++;
    }
}

/*--------------------- Default init --------------------*/

/* Make the setup what the program has without a setup file: every
   colour, every option, and both panels, which show the root of the
   current drive, the first of them switched off.

   The original copies the file's 88 bytes of a panel from DefWCB, which
   has 24: the other 64 are what stands behind it in the setup program,
   the box that says the setup file is missing, with the addresses that
   it names, and the start of a dialog that only the binary has that was
   shipped. They land behind the zero byte that ends the path of each
   panel, where nothing reads them, and are in the file when that is
   saved. Here those 64 bytes are zeros (see DEVIATIONS.md). */
static void StDefault(void)
{
    const u8 *from = StPalets;
    u8 *first = (u8 *)WCB1;
    u8 *second = (u8 *)WCB2;
    i32 i;

    for (i = 0; i < CmnIni; i++)
        ((u8 *)&md)[i] = from[i];
    for (i = 0; i < (i32)sizeof StDefWCB; i++)
        first[i] = StDefWCB[i];
    for (; i < WCBini; i++)
        first[i] = 0;
    WCB1->WinPath[0] += (u8)DriveCurrent();
    for (i = 0; i < WCBini; i++)
        second[i] = first[i];
    WCB1->Visible = 0;
}

/*------------------ Read configuration -----------------*/

/* Put the name of the setup file behind the directory of the program,
   in NcPath, which is the path of the file then. Returns where the name
   begins: a zero byte there makes NcPath the directory again, which it
   has to be for whatever else looks for a file of the program.

   The path may be longer than NcPath is, by the 13 bytes at the most
   that the original has room for behind it. Here it runs on into
   TempDir, which the program makes anew when the setup has been left,
   with NcPath (IniData of start.c). */
static u8 *StFile(void)
{
    u8 *end = md.NcPath;
    i32 i;

    while (*end != 0)
        end++;
    for (i = 0; i < (i32)sizeof StInit_F; i++)
        end[i] = StInit_F[i];
    return end;
}

/* Read the setup file, whose path is at NcPath: the setup that is edited
   into the main data, and the two panels into the setup's own. IniStat
   says what was found: 1 for a file of the right length that begins
   with VVV and has the right sum; 0 for none, and for one that could not
   be opened, read or closed; 2 for any other. Without a good file the
   setup is what the program has without one: StDefault. */
static void StInit15(void)
{
    struct Answer opened, closed;
    struct Moved read;
    const u8 *file = sd.File;
    i32 i;

    sd.IniStat = 0;
    opened = FileOpen(md.NcPath, 0x40); /* to read, with nobody kept from
                                           the file meanwhile */
    if (!opened.failed) {
        read = FileRead(opened.value, sd.File, INI_LEN + 1);
        closed = FileClose(opened.value);
        if (!closed.failed && !read.failed) {
            sd.IniStat = 1;
            if (read.count == INI_LEN
                && file[0] == 'V' && file[1] == 'V' && file[2] == 'V'
                && ChkSum(file, INI_LEN - 2) == SUM) {
                for (i = 0; i < CmnIni; i++)
                    ((u8 *)&md)[i] = COMMON[i];
                return;
            }
            sd.IniStat = 2;             /* Init17: bad data in VC.INI */
        }
    }
    StDefault();                        /* Init18 */
}

/*---------------------- Main Menu ----------------------*/

/* Give the line of the item `item` of the menu the colour `color`, a
   number of the palette, and put it on the screen: `begin` and `size`
   are where the menu's box is and how large. */
static void StMain40(u16 color, i32 item, u16 begin, u16 size)
{
    u16 place = begin + 0x209;
    i32 width = (u8)(size - 16);

    place = (u16)((u8)((place >> 8) + item) << 8 | (place & 0xFF));
    MvsColr(BasAdr(place), (u16)width, Color(color));
    Window(place, (u16)(0x100 | width));
}

/* Set default configuration: ask, and with Yes make the setup what the
   program has without a setup file. */
static void StF08_00(void)
{
    if ((u8)Dialog(&StDeflt_B, 0, 0, 0, &md) == 0)
        StDefault();
}

/* A key or the mouse at the menu, Main02 of the assembly, until one of
   them moves the cursor or chooses an item: answers the item that the
   cursor is to go to, or CHOSEN where the item in `item` is chosen, as
   at Main32. A key or the mouse that chooses Exit, Main31 of the
   assembly, and F2 to F9, Main20, make `item` that item first. The
   menu's box is `where`, and its last item, Exit, is `last`. */
#define CHOSEN (-1)

static i32 StMain02(i32 *item, i32 last, const struct Box *where)
{
    u16 key, mouse;

    for (;;) {                          /* Main02 */
        key = KEY_MOUSE << 8;           /* the mouse, as Input answers it */
        if (GetMous(0).buttons == 0) {
            md.MousEnt = 0;
            key = Input(0);
        }
        if (key >> 8 == KEY_MOUSE) {    /* Main30: Mouse */
            mouse = F32_70(where->begin, where->size);  /* Main50 */
            if (mouse >> 8 < 1)
                continue;
            if (mouse >> 8 == 1)        /* a line: its number */
                return mouse & 0xFF;
            if (mouse >> 8 != 0x1C)     /* not the second click on a line: */
                *item = 0x40;           /* Main31, Exit */
            return CHOSEN;
        }
        if (key >> 8 == 0)
            continue;
        switch (key & 0xFF) {
        case 0x0D:                      /* Enter */
            return CHOSEN;
        case 0x1B:                      /* Esc: Main31, Exit */
            *item = 0x40;
            return CHOSEN;
        }
        switch (key) {
        case 0x4800:                    /* Main10: Up */
            return *item == 0 ? last : *item - 1;
        case 0x5000:                    /* Main11: Down */
            return *item >= last ? 0 : *item + 1;
        case 0x4700:                    /* Main12: Home, */
        case 0x4900:                    /* PgUp */
            return 0;
        case 0x4F00:                    /* Main13: End, */
        case 0x5100:                    /* PgDn */
            return last;
        case 0x4400:                    /* F10: Main31, Exit */
            *item = 0x40;
            return CHOSEN;
        case 0x3C00:                    /* Main20: F2, Configuration */
        case 0x3D00:                    /* F3, Screen and Mouse */
        case 0x3E00:                    /* F4, Panel options */
        case 0x3F00:                    /* F5, Set Black & White palette */
        case 0x4000:                    /* F6, Set Color palette */
        case 0x4100:                    /* F7, Set Laptop palette */
        case 0x4200:                    /* F8, Set default configuration */
        case 0x4300:                    /* F9, Country info */
            *item = (key >> 8) - 0x3C;
            return CHOSEN;
        }
    }
}

/* The menu of the setup: Main00 of the original. Up and Down move the
   cursor, round at the ends; Home and PgUp go to the first item, End and
   PgDn to the last; Enter is the item under the cursor, and F2 to F9
   are an item each. What an item does returns here, and the menu is
   drawn again with the cursor on that item. Returns when the setup is
   left: by Esc or F10, or by the last item, Exit.

   The original has two tables here that Case goes through: Main_T, the
   keys, each with where the code goes on, and Menu_T, the items by their
   numbers from 1, each with its code. An entry of Main_T is a key with
   its scan code, or, for Enter and Esc, a character with any; Case
   takes no key without a scan code.

   The mouse is the main program's routine for a list in a box, F32_70
   of usermenu.c, which the setup program has word for word as Main50:
   a click on a line puts the cursor there, with either button, a second
   click on the line enters an item, and the middle button, or the left
   one let go outside the box, is Exit.

   Ported as it is: the item that a second click enters is the one under
   the cursor, not the one that was clicked. They are the same unless a
   key has moved the cursor between the two clicks. */
static void StMain00(void)
{
    struct Box where;
    i32 item, to, last;

    for (;;) {                          /* Main00 */
        PutKey(0);
        DialBox(0x528, &StMain_P, 0, &md, &where);
        Window(where.begin, where.size);
        item = sd.MnItem;
        last = ((where.size >> 8) - 6) & 0xFF;  /* the item Exit */
        md.MousTim |= 0xFFFF0000UL;
        md.MousEnt = 0;
        for (;;) {                      /* Main01: Set cursor */
            StMain40(HstCu_C, item, where.begin, where.size);
            md.HlpPage = StMainHlp[item];
            do
                to = StMain02(&item, last, &where);
            while (to == item);         /* Main14: where it is, Main02 */
            if (to == CHOSEN)
                break;
            StMain40(Hist_C, item, where.begin, where.size);
            item = to;
        }
        /* Main32: Enter */
        ResWin(where.begin, where.size, where.saved);
        Window(where.begin, where.size);
        sd.MnItem = item;
        switch (item) {
        case 0: StF02_00(); break;
        case 1: StF03_00(); break;
        case 2: StF04_00(); break;
        case 3: StF05_00(); break;
        case 4: StF06_00(); break;
        case 5: StF07_00(); break;
        case 6: StF08_00(); break;
        case 7: StF09_00(); break;
        default: return;
        }
    }
}

/*------------------------ Exit -------------------------*/

/* Leave the setup: Exit of the original. If anything of the setup has
   changed since it was read, ask whether to save it. Then what DOS had
   on the screen is put back, the mouse pointer is hidden, and the cursor
   has its shape again, at the start of the row that it was on. With
   Save, the setup file is written, with the two panels as they were
   read or as StDefault made them; the file keeps the attributes that it
   has and is marked as changed, and one that is not there is made.

   Then the program gets its setup. With Save it keeps what was edited,
   but for four bytes: which panel is the current one, whether the other
   is switched off, and the byte of TSR manager are as they were when the
   setup was entered, and the byte of Memory allocation stays so for
   LoadFl, with the setup file's in LoadRes, which is what LoadFl becomes
   when the setup has been left, as after a command. That is what the
   original's setup program does to a main program that is running, by a
   call of DOS that the main program answers with the address of its
   setup (INT 21h with AX 2B02h, CX 'VC' and DX 'OM'): the original's
   main program answers only if its setup has TSR manager on, and here
   the program takes the setup whatever that is. Without Save the program
   has its setup back as it was.

   Neither of the two bytes is a setting here: the setup shows no
   Memory allocation and no TSR manager, and nothing goes by either
   byte but the highest bit of LoadFl, which is no part of the setup
   (see vc.h). Both are in the file as they were read from it.

   Ported as they are:
     - Cancel in the box leaves the setup as Save does, without saving:
       nothing goes back to the menu.
     - A file that could not be written is said in a line that DOS
       writes, where the cursor is, and the program gets the setup all
       the same. */
static void StExit(void)
{
    const struct MainDat *aside = (const struct MainDat *)sd.Aside;
    struct FileAttr had;
    struct Answer opened, closed;
    struct Moved wrote;
    i32 i;
    bool save = false, failed = true;
    u8 edited;

    for (i = 0; i < CmnIni; i++)        /* Compare VC.INI with source */
        if (sd.SrcIni[i] != ((u8 *)&md)[i])
            break;
    if (i != CmnIni
        && (u8)Dialog(&StSvSt_B, 0, 0, 0, &md) == 0)
        save = true;
    StUser(0, md.Lines);                /* Exit1: Restore screen */
    Window(0, (u16)(md.Lines << 8 | 80));
    CurMous(0);                         /* Hide mouse */
    CursorShape(sd.CurType);            /* Restore cursor */
    Cursor((u16)(md.DosLin << 8));
    if (!save) {
        for (i = 0; i < CmnIni; i++)
            ((u8 *)&md)[i] = sd.Aside[i];
        return;
    }

    sd.File[0] = 'V';                   /* Exit2: Save VC.INI */
    sd.File[1] = 'V';
    sd.File[2] = 'V';
    for (i = 0; i < CmnIni; i++)
        COMMON[i] = ((u8 *)&md)[i];
    SUM = ChkSum(sd.File, INI_LEN - 2);
    StFile();
    had = FileAttributes(md.NcPath);
    /* A file that is not there has no attributes: the original goes on
       with CX as it is, 0 from adding up the sum. */
    if (!had.failed || had.value == ERR_NO_FILE) {
        opened = FileCreate(md.NcPath, had.attributes | ARCHIVE);  /* Exit4 */
        if (!opened.failed) {
            wrote = FileWrite(opened.value, sd.File, INI_LEN);
            closed = FileClose(opened.value);
            failed = closed.failed || wrote.failed
                     || wrote.count != INI_LEN;
        }
    }
    if (failed)                         /* Exit5 */
        ConsoleText(StStErr_S);

    edited = md.LoadFl;                 /* Exit6 */
    md.Active = aside->Active;
    md.Ctrl_O = aside->Ctrl_O;
    md.ChkTSR = aside->ChkTSR;
    md.LoadFl = aside->LoadFl;
    md.LoadRes = edited;
}

/*-------------------- The setup ---------------------*/

/* The setup, from the Options menu: all of it, until it has been left.
   When this returns, the screen is what DOS had shown, and the program
   has to go on as when a command has ended (see the top of this
   file).

   First the program is made ready as for a command, F21_30 and F21_31
   of enter.c, with three things otherwise: the two interrupts stay the
   program's, the row of the command line is emptied where a command
   is written into it with its prompt, and nothing is run. The cursor
   goes to the next row, as when the command is ended with Enter.

   Then what the setup program does when it starts, from where it reads
   its files: the pages of its help, StRdHelp of sthelp.c; the setup
   file, with the box that says so if it is missing or wrong, Ok to go
   on with what the program has without a file and Exit, Esc or F10 to
   leave; the copy that tells in the end whether anything has changed;
   the cursor's row and shape; the screen kept as DOS's; and the mouse
   set up anew, which puts its pointer in the middle of the screen. The
   main program puts it back when the setup has been left, where SavMous
   noted it.

   Ported as it is: with Ok in that box the original is to take the
   setup for changed, so that what the program has without a file is
   offered for saving when the setup is left, by filling SrcIni with
   FFh. It fills as many bytes as CX holds, which is 0 there: nothing is
   stored, and a setup that is left as it was made is not saved. */
void StSetup(void)
{
    struct CursorAt cursor;
    u8 *name;
    const u8 *reason;
    u16 page, found, answer;
    i32 i;

    md.LoadRes = md.LoadFl & 0x7F;      /* F21_30 */
    SavMous();                          /* F21_31 */
    Quit();
    ClsLine((u16)(md.DosLin << 8), &md);
    Window(0, (u16)(md.Lines << 8 | 80));
    Cursor((u16)(md.DosLin << 8));
    CurMous(0);
    page = ScreenState().page;
    ScreenType('\r', page, 0);
    ScreenType('\n', page, 0);

    for (i = 0; i < CmnIni; i++)
        sd.Aside[i] = ((u8 *)&md)[i];

    StRdHelp();                         /* the pages of the help */
    name = StFile();                    /* Init15 */
    StInit15();
    for (i = 0; i < CmnIni; i++)        /* Init20: Save source VC.INI */
        sd.SrcIni[i] = ((u8 *)&md)[i];
    md.Quote = 0;
    sd.MnItem = 0;
    md.BarPrev = 0xFF;
    cursor = CursorWhere(page);         /* Save cursor position */
    md.DosLin = (u8)(cursor.place >> 8);
    sd.CurType = cursor.shape;
    HidCur();
    SavScr();                           /* Save screen */
    StKeep();
    found = MouseFind();                /* Init mouse */
    if (found == 0x21)
        found = MouseReset();
    md.Mouse = found == 0xFFFF;        /* Init33 */
    if (md.Mouse != 0) {
        TypMous(0);
        if (md.Lines != 25)
            MouseRows(0, (u16)((md.Lines << 3) - 1));
        CurMous(1);                     /* Init34 */
    }

    StOpen = true;
    answer = 0;
    if (sd.IniStat != 1) {              /* Error */
        if (sd.IniStat < 1)
            reason = StNoFnd_S;
        else
            reason = StNoIni_S;
        answer = Dialog(&StIniEr_B, reason, 0, 0, &md);
        /* Err2, with Ok: nothing is stored in SrcIni. */
    }
    *name = 0;                          /* NcPath is the directory again */
    if ((u8)answer == 0)
        StMain00();
    StExit();
    StOpen = false;

    RestVct();                          /* as F21_31 has it for a command:
                                           the program takes the two
                                           interrupts again when it goes
                                           on */
    md.LoadFl = md.LoadRes;             /* Exec10 of exec.c */
}
