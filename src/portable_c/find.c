/*
 * Find file: the search of a directory and all below it for files by
 * their names and by what they contain, the list of what was found, and
 * the keys that are taken while the search runs.
 *
 * Translated from VCFIND.INC.
 *
 * The command, F67_00, asks what to look for, sets up what is kept here,
 * has FindFil search the directory at TmpBuf, on one drive or on every
 * fixed one, and lets FndMove take the keys once the search is over.
 * FindFil is also what DirFunc, which goes through the directories
 * below, calls for each of them: the search goes as deep as the
 * directories do.
 *
 * A file that was found is a line of 65 characters, as the list shows
 * it, and so is the directory before the first file found in it. The
 * lines are kept in the memory from ViewSeg on, one behind the other;
 * what follows the last one serves to read a file into that is searched
 * for a text.
 */
#include "vc.h"

#define LenFnf (LenStr + 1)             /* the length of a line */

/* What the search keeps: FndData, which lies in TmpBuf from its byte 100
   on. The path that is searched is at the start of TmpBuf. */
#pragma pack(push, 1)
struct FndData {
    u16 FndCur;                         /* Current line */
    u16 FndFrst;                        /* First output line */
    u16 FndNumb;                        /* Total number of lines */
    u16 FndSize;                        /* Max. number of files */
    u16 FndBg;                          /* Base of the window */
    u16 FndSz;                          /* Size of the window */
    u16 FndFls;                         /* Number of found files */
    u8  FndMode;                        /* 0-auto scroll; 1-allow move cursor */
    u8  FndDir;                         /* Not first file in subdirectory */
    u8  FndMenu;                        /* Menu item */
    u8  FndDrv;                         /* Drive letter */
};
#pragma pack(pop)

LAID_OUT(FndData, 18);

#define FND ((struct FndData *)(md.TmpBuf + 100))

/* Bits of FndMode and of FndDir. */
#define MODE_CURSOR  1                  /* a key has moved the cursor: the
                                           list no longer follows its end */
#define MODE_ANSWER  2                  /* the menu was answered while the
                                           search ran */
#define DIR_LISTED   1                  /* the directory has its line */
#define DIR_BEGUN    2                  /* DOS has been asked for its first
                                           file */

#define Conf_C  19                      /* the colour of the box */
#define CnfRe_C 20                      /* and of the line under the cursor */

#define KEY_MOUSE 0xFC                  /* the high byte of a key that is
                                           the mouse */

u16 FindFil(u16 attributes);
static u16 FndMove(u16 key, u16 mode, u16 place, u16 size);
static void FndCurs(u16 color, u16 line, u16 first, u16 place, u16 size);

/* FindFil is what FncAddr names for the search. F67_00, the command, is
   declared in vc.h: it returns where the program goes on, one of the TO_
   there, and in the high word, for TO_F24_02, the panel whose directory
   is another now. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   GetMous returns the buttons of the mouse and where it is, the row in
   the high byte; without a mouse that is `place` as it came. ClrKbd empties the keyboard buffer, if the setup
   says so, and TypMous sets the character that the mouse pointer is.
   InWind is not 0 if a place is outside a window. DoublMo returns 2 for a
   click that may be the first of two, 1 for the second of two on the line
   `line`, and 0 for a second one that was let go elsewhere.

   PgUp and PgDn move the cursor of a list a page, `rows` lines: they
   return the cursor and the first line shown. Int10m calls the BIOS for
   the screen, and returns the corners of the window as that leaves them.
   Int16 calls the BIOS for the keyboard: `none` of what it returns is 1
   if the zero flag was set. Help00 is the help,
   which is C, as is DirFunc, of vc.h.

   HidCur puts the cursor below the screen. SavWin keeps what the
   screen buffer has where a window is to be, ResWin puts it back, and
   MinBox draws a box with a shadow there, in the colour in the high byte
   of `color`. StdLine writes a line of such a box at `place`, of the
   width that the low byte of `size` has for the box: the cell `cell`
   between the characters of `ends`; it returns the place a row below.

   Int21d calls DOS as Intr21 does, with the registers of `regs` and
   their answer there, but not through Intr21: DOS's box for a drive
   that is not ready is not looked for, and an error is not asked about.
   Int2F calls the multiplex interrupt, and returns AX and BX. */
void Help00(void);                                      /* help.c */

/* C routines of other files that vc.h does not have. */
struct DateText {                                       /* format.c */
    u16 century;
    u16 first, second, third;
};
struct TimeText {
    u16 hour, minute, second;
    u8  unused;
    u8  suffix;
};
void PutDate(u16 date, struct DateText STACK *text);
u8 *BufDate(const struct DateText STACK *text, u8 *buffer);
void PutTime(u16 time, struct TimeText STACK *text);
u8 *BufTime(const struct TimeText STACK *text, u8 *buffer);
void FilZero(u8 __far *text, u16 length);               /* string.c */
void NormStr(u8 __far *text);
u8 __far *UnConv(u8 __far *name, const u8 __far *buffer);   /* mask.c */
void V81_00(void);                                      /* view.c */
u32 V82_00(u16 segment, u16 at, u16 count);

/* Objects that are named by their offsets, of find_t.c but for
   Mask_S, which is of another file: Mask_S is \*.*, and four bytes are
   copied from it or from behind it; Find5_D describes the menu of a
   search that runs, and only its offset is used here, as are those of
   Find6_D, the menu of one that is over, of FnFl_P, the colours of that
   menu, and of Find_B, the dialog that asks what to look for. FnFl6_S
   is " files" and FnFl7_S " found.": each is copied with what follows it
   in the original, 15 bytes, and is written that long. */
extern const u8 __far Mask_S[];
extern const u8 __far Find5_D[];
extern const u8 __far Find6_D[];
extern const u8 __far FnFl_P[];
extern const u8 __far Find_B[];
extern const u8 __far FnFl6_S[];
extern const u8 __far FnFl7_S[];

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define SEGMENT(pointer) ((u16)((u32)(const void __far *)(pointer) >> 16))

/* What the size column says for a directory. */
static const u8 CODE SubDir[7] = "SUB-DIR";

/* Errors of Intr21. */
#define ERR_NOT_READY 0x53

/* A place on the screen `rows` and `columns` from another, each within
   its byte: nothing is carried from the column to the row. */
#define MOVED(place, rows, columns) \
    ((u16)((place) + ((rows) << 8) & 0xFF00 | (place) + (columns) & 0xFF))

/* The rows of the box that show lines of the list: all but eight. The
   assembly takes the number for a signed byte where it compares with it,
   and for an unsigned one where it counts. */
#define ROWS(size) ((u16)(((size) >> 8) - 8 & 0xFF))
#define SIGNED(byte) ((u16)(int)(signed char)(byte))

/* What a key or the mouse asks of FndMove. */
enum {
    ASK_NOTHING, ASK_UP, ASK_DOWN, ASK_HOME, ASK_END, ASK_PGUP, ASK_PGDN,
    ASK_ENTER, ASK_ESC, ASK_LINE, ASK_CLICK
};

/* Where a line of the list is in the memory. The lines follow one
   another from ViewSeg on, and the address is made so that a line never
   crosses the end of a segment: of 20 bits, like the assembly's. */
static u8 __far *Line(u16 number)
{
    u32 at = ((u32)number << 6) + number;       /* 65 bytes to a line */

    return (u8 __far *)((u32)(u16)((u16)(at >> 4) + md.ViewSeg) << 16
                        | (u16)at & 0x0F);
}

/* Write a line of the list to the screen buffer, in the colour of the
   box: 64 cells from `place`. */
static void FndLine(u16 number, u16 place)
{
    const u8 __far *line = Line(number);
    u16 *screen = BasAdr(place);
    u16 color = Color(Conf_C) << 8;
    u16 i;

    for (i = 0; i < LenFnf - 1; i++)
        *screen++ = color | line[i];
}

/* Show a line of the list in a colour, by its number: that of the box, or
   that of the cursor. `first` is the line at the top of the list, and the
   colour is in the high byte of `color`. */
static void FndCurs(u16 color, u16 line, u16 first, u16 place, u16 size)
{
    u16 width = (size - 12) & 0xFF;

    place = (u16)(place + ((line - first) << 8)) + 0x205;
    FndLine(line, place);
    MvsColr(BasAdr(place), width, Color(color >> 8));
    Window(place, 0x100 | width);
}

/* Show below the list what is being searched: the path at TmpBuf, in
   capitals and shortened to 54 characters. */
static void FndStat(void)
{
    const u8 *text = md.TmpErr;
    u8 *screen;
    u16 place, i;
    u8 c;

    for (i = 0; i < LenPath + 13; i++)
        md.TmpErr[i] = md.TmpBuf[i];
    ShrtLin(md.TmpErr, LenFnf - 11);
    PathLin(md.TmpErr);
    place = MOVED(FND->FndBg, (FND->FndSz >> 8) - 5, 5 + 10);
    screen = (u8 *)BasAdr(place);
    for (i = LenFnf - 11; i; i--) {
        c = *text++;
        if (c == 0) {
            text--;
            c = ' ';
        }
        *screen = c;
        screen += 2;
    }
    Window(place, 0x100 | (LenFnf - 11));
}

/* What the mouse asks of FndMove, which is there with a button down.
   `at` is CX as FndMove got it, which GetMous leaves alone where there is
   no mouse. The middle button, let go without another, is Esc. Outside
   the list, a button held down moves the cursor a line up or down once
   it has been in the list, and the left button let go outside the box
   is Esc. In the list the cursor goes to the line under the mouse, at
   `*line`, and a second click on that line is Enter. */
static u16 FnMv00(u16 at, u16 place, u16 size, u16 STACK *line)
{
    struct Mouse mouse;
    u16 buttons, was, last, target, clicks;

    mouse = GetMous(at);
    for (;;) {
        /* FnMv01 */
        buttons = mouse.buttons;
        at = mouse.place;
        ClrKbd();
        was = at;
        if (buttons & 4) {
            TypMous('x');
            do {
                mouse = GetMous(at);
                buttons = mouse.buttons;
                at = mouse.place;
                ClrKbd();
            } while (buttons & 4);
            TypMous(0);
            if (!(buttons & 3))
                return ASK_ESC;
        }
        /* FnMv03 */
        if (!InWind(at, place + 0x204, size - 0x80A))
            break;
        if (md.WinMous != 0) {
            if ((u8)(at >> 8) < (u8)((place >> 8) + 2))
                return ASK_UP;
            if ((u8)(at >> 8) < (u8)((place >> 8) + (size >> 8) - 6))
                return ASK_NOTHING;
            return ASK_DOWN;
        }
        /* FnMv05 */
        if (!(buttons & 1))
            return ASK_NOTHING;
        mouse = GetMous(at);
        if (mouse.buttons != 0)
            continue;
        if (InWind(was, place, size - 0x102))
            return ASK_ESC;
        return ASK_NOTHING;
    }
    /* FnMv08 */
    md.WinMous |= 1;
    last = FND->FndNumb;
    if (last == 0)
        return ASK_NOTHING;
    target = (u8)((at >> 8) - (place >> 8) - 2) + FND->FndFrst;
    last--;
    if (target > last) {
        *line = last;
        return ASK_LINE;
    }
    *line = target;
    clicks = DoublMo(at & 0xFF00 | (place + 2) & 0xFF,
                     0x100 | (size - 10) & 0xFF, target);
    if (clicks == 2)
        return ASK_LINE;
    if (clicks == 0)
        return ASK_NOTHING;
    TypMous(0);
    return ASK_CLICK;
}

/* What a key asks of FndMove: Find_T in the assembly, a table for Case.
   A key without a scan code is none of these, and Enter and Esc are
   known by their characters. */
static u16 Find_T(u16 key)
{
    if ((key >> 8) == 0)
        return ASK_NOTHING;
    switch (key) {
    case 0x4800: return ASK_UP;
    case 0x5000: return ASK_DOWN;
    case 0x4700: return ASK_HOME;
    case 0x4F00: return ASK_END;
    case 0x4900: return ASK_PGUP;
    case 0x5100: return ASK_PGDN;
    case 0x4400: return ASK_ESC;        /* F10 */
    }
    if ((u8)key == 0x0D)
        return ASK_ENTER;
    if ((u8)key == 0x1B)
        return ASK_ESC;
    return ASK_NOTHING;
}

/* Take a key, or the mouse, for the list in the box at `place`: move the
   cursor and show the list where it has moved. The low byte of `mode` is
   put into FndMode: MODE_CURSOR for a key that was pressed, nothing for
   the search, which has the cursor follow the end of the list. Returns 0
   for a key that has been dealt with, and else 1 in the high byte and in
   the low one the item of the menu that Enter chose, FFh for Esc and 1
   for a second click on a line. */
static u16 FndMove(u16 key, u16 mode, u16 place, u16 size)
{
    u16 line = FND->FndCur;
    u16 first = FND->FndFrst;
    u16 ask, rows, old, wasLine, wasFirst, moved, count, at, number;
    u16 scroll, color;
    struct Paged paged;

    if ((key >> 8) == KEY_MOUSE)
        ask = FnMv00(mode, place, size, (u16 STACK *)&line);
    else
        ask = Find_T(key);
    rows = SIGNED((size >> 8) - 8);
    switch (ask) {
    case ASK_NOTHING:
        return 0;
    case ASK_ENTER:
        return 0x100 | FND->FndMenu;
    case ASK_ESC:
        return 0x1FF;
    case ASK_CLICK:
        return 0x101;
    case ASK_HOME:
        line = 0;
        break;
    case ASK_END:
        line = FND->FndNumb;
        /* and one up */
    case ASK_UP:
        if (line != 0)
            line--;
        break;
    case ASK_DOWN:
        line++;
        if (line >= FND->FndNumb)
            line--;
        break;
    case ASK_PGUP:
        paged = PgUp(line, first, rows);
        line = paged.cursor;
        first = paged.first;
        break;
    case ASK_PGDN:
        paged = PgDn(line, first, rows, FND->FndNumb);
        line = paged.cursor;
        first = paged.first;
        break;
    }

    /* FnMv20: the list is shown from where the cursor is on it. */
    old = FND->FndMode;
    FND->FndMode |= (u8)mode;
    if (line < first)
        first = line;
    if (line >= (u16)(rows + first))
        first = line - rows + 1;
    wasLine = FND->FndCur;
    wasFirst = FND->FndFrst;
    FND->FndCur = line;
    FND->FndFrst = first;
    moved = first - wasFirst;
    if (moved == 0) {
        if (!(FND->FndMode & MODE_CURSOR))
            goto cursor;
        if (wasLine == line) {
            if (!(old & MODE_CURSOR))
                goto cursor;
            return 0;
        }
    }
    /* FnMv24: the line that had the cursor, where it was. */
    FndCurs(Conf_C << 8, wasLine, wasFirst, place, size);
    if (moved != 0) {
        count = ROWS(size);
        at = place + 0x205;
        number = FND->FndFrst;
        do {
            FndLine(number, at);
            number++;
            at += 0x100;
        } while (--count);
        /* By one line the BIOS scrolls the screen, and only the line
           that came in is put there from the buffer. */
        at = place + 0x205;
        rows = ROWS(size);
        if (moved == 1 || moved == 0xFFFF) {
            scroll = 0x600 | (rows - 1) & 0xFF;
            if (moved != 1)
                scroll = 0x700;
            at = Int10m(scroll & 0xFF00 | 1,
                        Color(Conf_C) << 8 | at & 0xFF, at,
                        (((rows - 1) & 0xFF) << 8 | 63) + at).upper;
            at += (scroll & 0xFF) << 8;
            rows = 1;
        }
        Window(at, rows << 8 | 64);
    }
cursor:
    /* FnMv28 */
    if (FND->FndNumb == 0)
        return 0;
    color = Conf_C;
    if (FND->FndMode & MODE_CURSOR)
        color = CnfRe_C;
    FndCurs(color << 8, FND->FndCur, FND->FndFrst, place, size);
    return 0;
}

/* Add the line at TmpErr to the list and show it: at the end of the
   list, which the cursor follows, or, once a key has moved the cursor,
   where the line falls in what the box shows. Returns 1 if the memory
   has no room for the line. */
static u16 FndStuf(void)
{
    u16 number = FND->FndNumb;
    u16 place, size, line, first, shown, color, i;
    u8 __far *to;

    if (FND->FndSize < number)
        return 1;
    FND->FndNumb++;
    to = Line(number);
    for (i = 0; i < LenFnf; i++)
        to[i] = md.TmpErr[i];
    place = FND->FndBg;
    size = FND->FndSz;
    if (!(FND->FndMode & MODE_CURSOR)) {
        FndMove(0x4F00, 0, place, size);
        return 0;
    }
    line = FND->FndNumb - 1;
    first = FND->FndFrst;
    shown = line - first + 8;
    if ((shown >> 8) == 0 && (u8)shown < (u8)(size >> 8)) {
        color = Conf_C;
        if (line == 0)
            color = CnfRe_C;
        FndCurs(color << 8, line, first, place, size);
    }
    return 0;
}

/* Between two files, or two parts of one: take the keys and the mouse
   that have come in. Returns 1 for the search to go on, and 0 to stop
   it: for Esc, Ctrl-C and Ctrl-Break, and for an answer to the menu,
   which is then in FndMenu, with MODE_ANSWER set.

   The menu takes the keys that are its own, and what FndMove then gets
   is not the key: MnMn40, which draws the menu anew, ends in Window, and
   that leaves the carry flag clear, where the assembly tested it to go
   back for the next key. So FndMove is called with what MnMn40 leaves in
   AX, two bytes of a colour, and does nothing, unless the colour is that
   of Enter or of Esc: 0Dh or 1Bh. */
static u16 FndBack(void)
{
    u32 answer;
    struct Mouse pointer;
    u16 key, mode, place, size, menu, item, go;

    for (;;) {
        /* The place of the mouse, if there is one, gets to FndMove as
           the high byte of `mode`, where nothing reads it. */
        pointer = GetMous(0);
        key = pointer.buttons;
        mode = pointer.place;
        if (key != 0) {
            key = KEY_MOUSE << 8 | key & 0xFF;
        } else {
            if (Int16(0x100).none) {
                go = 1;
                break;
            }
            key = Int16(0).key;
            if (key == 0 || key == 0x2E03 || key == 0x011B) {
                go = 0;
                break;
            }
            if (key == 0x3B00) {
                Help00();
                continue;
            }
        }
        place = FND->FndBg;
        size = FND->FndSz;
        if (md.WinMous == 0) {
            menu = MOVED(place, (size >> 8) - 4, 21);
            answer = MnMn10(key, menu, FND->FndMenu, 0x11F, OFFSET(Find5_D));
            key = (u16)answer;
            item = (u16)(answer >> 16) & 0xFF;
            mode = item;
            FND->FndMenu = (u8)item;
            if (answer >> 24)
                key = MnMn40(Conf_C << 8 | CnfRe_C, menu, item, 0x11F,
                             OFFSET(Find5_D));
        }
        key = FndMove(key, mode & 0xFF00 | MODE_CURSOR, place, size);
        if ((key >> 8) == 0)
            continue;
        FND->FndMode |= MODE_ANSWER;
        FND->FndMenu = (u8)key;
        go = 0;
        break;
    }
    md.WinMous = 0;
    md.MousEnt = 0;
    return go;
}

/* Search the directory at TmpBuf, and those below it, for the files that
   FindStr names and, if SrchStr is a text, that contain it; DirFunc
   calls this for every file of a directory, and for all but directories
   it does nothing. `attributes` are those of what is at TmpBuf. Returns
   0 to go on with the next, and 100h if the search has been stopped: by
   a key, by an error, or because the memory is full. */
u16 FindFil(u16 attributes)
{
    struct DosRegs regs;
    struct DateText date;
    struct TimeText time;
    const u8 __far *from;
    u8 *end, *to, *name;
    u16 length, suffix, handle, room, chunk, segment, count, found, i;
    struct Answer opened;
    u8 c, kind;

    if (!(attributes & ATTR_DIRECTORY))
        return 0;
    FndStat();
    for (length = 0; length < LenPath + 13 - 1; length++)
        if (md.TmpBuf[length] == 0)
            break;
    if (length >= LenFnf)
        return 0;
    end = md.TmpBuf + length;
    if (end[-1] != ':' && end[-1] != '\\')
        *end++ = '\\';

    /* FnFl12: the mask, made whole. A name without a full stop is taken
       to begin what is looked for, and gets *.* behind it; one with a
       full stop gets the four bytes behind that, a zero and three more. */
    from = CutPath(md.FindStr);
    to = md.TmpErr + 20;
    suffix = 1;
    while ((c = *from++) != 0) {
        if (c == '.')
            suffix = 4;
        *to++ = c;
    }
    for (i = 0; i < 4; i++)
        *to++ = Mask_S[suffix + i];
    Convert(md.TmpErr + 20, md.TmpErr);
    UnConv(end, md.TmpErr);
    FND->FndDir = 0;

next:
    /* FnFl40 */
    if (!FndBack())
        return 0x100;
    if (ChgDisk(md.TmpBuf))
        return 0x100;
    regs.ax = 0x4F00;
    if (!(FND->FndDir & DIR_BEGUN)) {
        FND->FndDir |= DIR_BEGUN;
        regs.cx = 0x37;
        regs.ax = 0x4E00;
    }
    SetDTA();
    regs.name = md.TmpBuf;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax == ERR_NOT_READY)
            return 0x100;
        /* FnFl50: no more files here. On to the directories. */
        *(u8 *)CutPath(md.TmpBuf) = 0;
        return DirFunc();
    }
    /* FnFl44 */
    if (md.DTA.FilName[0] == '.')
        goto next;
    if (md.SrchStr[0] != 0) {
        /* The file is read a part at a time, into the memory behind the
           list, and the text looked for in each; a part begins 80 bytes
           before the end of the one before. */
        V81_00();
        name = (u8 *)CutPath(md.TmpBuf);
        for (i = 0; i < 13; i++)
            name[i] = md.DTA.FilName[i];
        FndStat();
        opened = OpenFil(md.TmpBuf, 0x40);
        if (opened.failed) {
            if (opened.value == ERR_NOT_READY)
                return 0x100;
            goto next;
        }
        handle = opened.value;
        room = SEGMENT(&md) + md.FreMem2 - md.ViewSeg;
        chunk = 0x8000 + 80;
        if (!(room & 0xF000)) {
            room <<= 4;
            if (room < chunk)
                chunk = room;
        }
        found = 0;
        if (chunk < 4000)
            goto stopped;
        segment = SEGMENT(Line(FND->FndNumb + 1));
        for (;;) {
            /* FnFl46 */
            if (!FndBack())
                goto stopped;
            regs.ax = 0x3F00;
            regs.bx = handle;
            regs.cx = chunk;
            regs.name = (const u8 __far *)((u32)segment << 16);
            if (Intr21((struct DosRegs STACK *)&regs))
                goto stopped;
            count = regs.ax;
            if ((u16)V82_00(segment, 0, count) != 0x100) {
                found = 1;
                break;
            }
            if (count < chunk)
                break;
            regs.ax = 0x4201;
            regs.bx = handle;
            regs.cx = 0xFFFF;
            regs.name = (const u8 __far *)((u32)segment << 16 | (u16)-80);
            Intr21((struct DosRegs STACK *)&regs);
        }
        /* FnFl49 */
        regs.ax = 0x3E00;
        regs.bx = handle;
        regs.name = md.TmpBuf;
        Intr21((struct DosRegs STACK *)&regs);
        if (!found)
            goto next;
    }

    /* FnFl20: a file to list. Before the first one of a directory, the
       directory, and before that an empty line if it is not the first. */
    if (!(FND->FndDir & DIR_LISTED)) {
        FND->FndDir |= DIR_LISTED;
        if (FND->FndNumb != 0) {
            if ((u16)(FND->FndNumb + 2) >= FND->FndSize)
                return 0x100;
            for (i = 0; i < LenFnf; i++)
                md.TmpErr[i] = 0;
            FndStuf();
        }
        for (i = 0; i < LenPath + 13; i++)
            md.TmpErr[i] = md.TmpBuf[i];
        name = (u8 *)CutPath(md.TmpErr);
        if ((u16)(name - md.TmpErr) > 3)
            name--;
        *name = 0;
        PathLin(md.TmpErr);
        FilZero(md.TmpErr, LenFnf);
        FndStuf();
    }
    /* FnFl30: the line of the file. */
    for (i = 0; i < LenFnf; i++)
        md.TmpErr[i] = ' ';
    to = md.TmpErr + 3;
    kind = md.DTA.FilAttr;
    for (i = 0; (c = md.DTA.FilName[i]) != 0; i++) {
        if (!(kind & ATTR_DIRECTORY))
            c = (u8)LowCase(c);
        *to++ = c;
    }
    to = md.TmpErr + 3 + 14;
    if (kind & ATTR_DIRECTORY) {
        to += 6;
        for (i = 0; i < 7; i++)
            *to++ = SubDir[i];
    } else {
        to = Number(to, (u16)(md.DTA.FilSize >> 16), (u16)md.DTA.FilSize,
                    NUM_SEPARATED | NUM_PADDED);
        *to = ' ';
    }
    to += 3;
    *to++ = (kind & 0x20) ? 'A' : '.';
    *to++ = (kind & ATTR_SYSTEM) ? 'S' : '.';
    *to++ = (kind & ATTR_HIDDEN) ? 'H' : '.';
    *to++ = (kind & 0x01) ? 'R' : '.';
    to += 3;
    PutDate(md.DTA.FilDate, (struct DateText STACK *)&date);
    to = BufDate((struct DateText STACK *)&date, to);
    to += 3;
    PutTime(md.DTA.FilTime, (struct TimeText STACK *)&time);
    BufTime((struct TimeText STACK *)&time, to);
    if (FndStuf())
        return 0x100;
    FND->FndFls++;
    goto next;

stopped:
    /* FnFl48 */
    regs.ax = 0x3E00;
    regs.bx = handle;
    regs.name = md.TmpBuf;
    Intr21((struct DosRegs STACK *)&regs);
    return 0x100;
}

/* What the box of the command shows. The colours are those of Find_P in
   find_t.c, which the dialog reads there. */
static const u8 CODE FindColors[4] = { Conf_C, CnfRe_C, 21, 22 };
static const u8 CODE Find0_S[] = " Find File ";
static const u8 CODE Find1_S[] = "File name:";
static const u8 CODE Find2_S[] = "Containing:";
static const u8 CODE Find3_S[] = "Scanning:";
static const u8 CODE Find5_S[] =
    "^r[ Stop ]^r\xFF\x03[ Go to ]\xFF\x03[ Quit ]";
static const u8 CODE Find6_S[] =
    "[ New search ]\xFF\x03[ Go to ]\xFF\x03[ Quit ]";

#define FIND_BAR 1                      /* the key bar of the box: bKeyBar
                                           of Find_B */
#define SPACE       ' '
#define LINE        0xC4                /* a single line */
#define BOTH_SIDES  0xBABA              /* the frame, left and right */
#define LEFT_RIGHT  0xC7B6              /* where a line meets it */
#define ALL_DRIVES  0x3A2A              /* "*:", as a word */

/* Whether a file is shown in hex: ViTxHex of the viewer, which the
   search reads files with. */
#define ViTxHex md.TmpBuf[216]

/* DX of a call of DOS, where it is not the offset of a name. */
#define DX_OF(regs) (*(u16 STACK *)&(regs).name)

#define NEAR(pointer) ((u8 *)(u16)(u32)(pointer))
#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

/* Alt-F7, the file search: a box over the panels, in which a dialog asks
   for a name, with a path or without, and for a text; then the search,
   from the directory that the name gives, or the current one, down; and
   then the list, with "New search", which asks again, "Go to", which
   makes the directory of the line under the cursor the current one, with
   the cursor of the panel on the file, and "Quit". The search answers
   the same way if a key answered its menu while it ran.

   A name on the drive "*:" is looked for on every drive from A: on that
   is of a network or is not removable, and on no CD-ROM, from its root
   if no path is given. The name is then left out of the path: FindFil
   takes it from FindStr.

   Ported as it is: Go to asks for the disk of a drive that shares the
   floppy drive, ChgDisk, with the address of the line, which is in the
   memory of the list, and ChgDisk reads there in the main data, two
   bytes of the first palette. So it asks for the disk of the current
   drive, unless the second of the two is a colon. And the path of the
   line goes into the panel with what follows it, 69 bytes, one more
   than a path has room for there, which is the first of where the
   cursor is and is set next. */
u32 F67_00(void)
{
    struct LineArgs line;
    struct DosRegs regs;
    struct WCBdef __far *wcb;
    const u8 __far *from;
    u8 __far *to;
    const u8 *rest;
    u8 *end;
    u16 *screen;
    u16 place = 0x203;
    u16 size = (u16)(u8)(md.Lines - 3) << 8 | 76;
    u16 color, row, count, answer, first, drive, cell, i, number, panel;
    u16 key, mode, room;
    u32 pair;
    struct MuxRegs mux;
    struct Mouse pointer;

    HidCur();
    PutMnu(3);
    SavWin(place, size, (u16)md.WinBuf);
    MinBox(place, size, Color(Conf_C) << 8);
    /* No line of the box has ^I: the include line is any text. */
    line.colors = FindColors;
    line.include = Find0_S;
    line.value = place + 0x100 & 0xFF00 | 40;
    line.line = Find0_S;
    CntLine((struct LineArgs STACK *)&line);

again:
    /* F67_01: the box empty, with the two questions */
    color = Color(Conf_C) << 8;
    row = place + 0x200;
    for (count = (u8)((size >> 8) - 5); count != 0; count--)
        row = StdLine(row, color | SPACE, BOTH_SIDES, size);
    row = StdLine(row - 0x500, color | LINE, LEFT_RIGHT, size);
    row = ADD_LOW(row, 5);
    line.value = color | 25;
    line.screen = BasAdr(row);
    line.line = Find1_S;
    NgLine((struct LineArgs STACK *)&line);
    row += 0x200;
    line.screen = BasAdr(row);
    line.line = Find2_S;
    NgLine((struct LineArgs STACK *)&line);
    Window(place, size);
    /* The dialog has its box there already: its nine lines. */
    answer = Dialog(OFFSET(Find_B), 9 << 8 | size & 0xFF,
                    place + (size & 0xFF00) - 0x900, 0, MAIN_SEG);
    if ((u8)answer != 0)
        goto close;

    /* F67_03: what is searched, and the menu, in their place */
    color = Color(Conf_C) << 8;
    row = place + (size & 0xFF00) - 0x800;
    row = StdLine(row, color | SPACE, BOTH_SIDES, size);
    row = StdLine(row, color | SPACE, BOTH_SIDES, size);
    row += 0x100;
    row = StdLine(row, color | SPACE, BOTH_SIDES, size);
    row = StdLine(row, color | SPACE, BOTH_SIDES, size);
    row = StdLine(row - 0x300, color | LINE, LEFT_RIGHT, size);
    row = ADD_LOW(row, 5);
    line.value = color | 25;
    line.screen = BasAdr(row);
    line.line = Find3_S;
    NgLine((struct LineArgs STACK *)&line);
    line.value = row + 0x100 & 0xFF00 | 40;
    line.line = Find5_S;
    CntLine((struct LineArgs STACK *)&line);
    Window(place, size);
    FND->FndBg = place;
    FND->FndSz = size;
    FND->FndCur = 0;
    FND->FndFrst = 0;
    FND->FndNumb = 0;
    FND->FndFls = 0;
    FND->FndMode = 0;
    FND->FndMenu = 0;
    md.WinMous = 0;
    md.MousEnt = 0;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;
    ViTxHex = 0;
    /* As many lines as the memory behind the program has room for. */
    room = SEGMENT(&md) + md.FreMem2 - md.ViewSeg;
    FND->FndSize = (u16)(((u32)room << 4) / LenFnf) - 1;
    FND->FndDrv = 'A';

    for (;;) {
        /* F67_08: the path to search, at TmpErr, with a name of one
           letter in place of what is looked for */
        rest = md.FindStr + 2;
        end = md.TmpErr + 2;
        *(u16 *)md.TmpErr = *(u16 *)md.FindStr;
        if (*(u16 *)md.FindStr == ALL_DRIVES
            && NEAR(CutPath(rest)) == rest)
            *end++ = '\\';
        for (i = 0; i < LenPath + 13 - 2; i++)
            *end++ = *rest++;
        NormStr(md.TmpErr);
        if (md.TmpErr[0] == 0)
            goto close;
        *(u16 *)NEAR(CutPath(md.TmpErr)) = 'A';
        first = *(u16 *)md.TmpErr;
        if (first == ALL_DRIVES) {
            for (;;) {
                /* F67_10: the next drive that is searched */
                drive = FND->FndDrv;
                md.TmpErr[0] = (u8)drive;
                FND->FndDrv++;
                drive = (u8)(drive - 'A');
                mux = Int2F(0x150B, 0, drive);
                if (mux.bx != 0xADAD || mux.ax == 0) {
                    /* F67_11: no CD-ROM */
                    regs.name = md.TmpErr;
                    regs.bx = drive + 1;
                    regs.cx = drive;
                    regs.ax = 0x4409;
                    if (!Int21d((struct DosRegs STACK *)&regs)
                        && (DX_OF(regs) & 0x1000))
                        break;
                    regs.ax = 0x4408;
                    if (!Int21d((struct DosRegs STACK *)&regs)
                        && (u8)regs.ax == 1)
                        break;
                }
                /* F67_13 */
                if (FND->FndDrv > 'Z')
                    goto searched;
            }
        }
        /* F67_15: the path made whole, at TmpBuf */
        if (ChgDisk(md.TmpErr))
            goto close;
        rest = md.TmpErr + 2;
        drive = *(u16 *)md.TmpErr;
        if ((drive >> 8) != ':') {
            rest = md.TmpErr;
            drive = *(u16 *)md.DosPth;
        }
        drive = UpCase(drive);
        *(u16 *)md.TmpBuf = drive;
        end = md.TmpBuf + 2;
        count = LenPath + 13;
        if (*rest != '\\') {
            *end++ = '\\';
            regs.name = md.TmpBuf;
            DX_OF(regs) = (u8)(drive - ('A' - 1));
            regs.si = (u16)end;
            regs.ax = 0x4700;
            if (Intr21((struct DosRegs STACK *)&regs))
                goto found;
            while (count != 0) {
                count--;
                if (*end++ == 0)
                    break;
            }
            end = NEAR(AddFile(end - 1));
        }
        /* F67_17 */
        for (; count != 0; count--)
            *end++ = *rest++;
        *end = 0;
        *NEAR(CutPath(md.TmpBuf)) = 0;
        md.FncAddr = FNC_FINDFIL;
        answer = FindFil(ATTR_DIRECTORY);
        if ((answer >> 8) != 0 || first != ALL_DRIVES || FND->FndDrv > 'Z')
            break;
    }

searched:
    /* F67_18 */
    if ((FND->FndMode & MODE_ANSWER) && FND->FndMenu != 0) {
        answer = FND->FndMenu;
        goto answered;
    }

found:
    /* F67_19: "N files found", and the menu of a search that is over */
    end = Number(md.TmpBuf, 0, FND->FndFls, NUM_SEPARATED);
    end = NEAR(MvzLine(end, FnFl6_S, 15));
    if (FND->FndFls == 1)
        end--;
    for (i = 0; i < 15; i++)
        end[i] = FnFl7_S[i];
    place = FND->FndBg;
    size = FND->FndSz;
    row = MOVED(place, (size >> 8) - 5, 5);
    screen = BasAdr(row);
    count = (u8)(size - 12);
    cell = *screen & 0xFF00;            /* in the colour that is there */
    for (i = 0; i < count; i++)
        screen[i] = cell;
    line.value = count;
    line.screen = screen;
    line.line = md.TmpBuf;
    NgLine((struct LineArgs STACK *)&line);
    Window(row, 0x100 | count);
    line.value = MOVED(place, (size >> 8) - 4, 0) & 0xFF00 | 40;
    line.line = Find6_S;
    CntLine((struct LineArgs STACK *)&line);
    FND->FndMenu = 1;
    if (FND->FndCur != 0)
        FndCurs(CnfRe_C << 8, FND->FndCur, FND->FndFrst, place, size);
    do {
        /* F67_21: a key for the menu, and then for the list. With a
           button of the mouse down, the list alone. */
        pointer = GetMous(0);
        key = pointer.buttons;
        mode = pointer.place;
        if (key != 0) {
            key = KEY_MOUSE << 8 | key & 0xFF;
        } else {
            md.WinMous = 0;
            md.MousEnt = 0;
            pair = MinMnu(OFFSET(Find6_D),
                          MOVED(place, (size >> 8) - 4, 18), 0x125,
                          OFFSET(FnFl_P), KEY_MOUSE << 8 | FIND_BAR,
                          FND->FndMenu);
            key = LOW(pair);
            mode = HIGH(pair);
            FND->FndMenu = (u8)mode;
        }
        answer = FndMove(key, mode & 0xFF00 | MODE_CURSOR, place, size);
    } while ((answer >> 8) == 0);

answered:
    /* F67_23 */
    if ((u8)answer == 0)
        goto again;
    if ((u8)answer != 1) {
close:
        /* F67_25 */
        ResWin(place, size, (u16)md.WinBuf);
        Window(place, size);
        return TO_FUNC04;
    }

    /* F67_30: Go to */
    ResWin(place, size, (u16)md.WinBuf);
    Window(place, size);
    if (FND->FndNumb == 0)
        return TO_FUNC04;
    /* The line of the directory: the first above that begins with no
       space. */
    number = FND->FndCur;
    do
        from = Line(number--);
    while (*from == ' ');
    if (ChgDisk(NEAR(from)))
        return TO_FUNC04;
    regs.name = from;
    regs.ax = 0x3B00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return TO_FUNC04;
    DX_OF(regs) = (u8)(*from - 'A');
    regs.ax = 0x0E00;
    Int21d((struct DosRegs STACK *)&regs);
    /* The panel that gets the directory: the current one, or the other
       if that one shows no files. Its list is emptied, but for the name
       of the file, where the cursor is to be when it is read. */
    panel = md.WCBcrn;
    if (PANEL(panel)->WinTyp != 0)
        panel = InvWCB(panel);
    wcb = PANEL(panel);
    to = wcb->WinPath;
    for (i = 0; i < LenPath + 1; i++)
        to[i] = from[i];
    wcb->CurAdr = DSKBUF;
    wcb->First = DSKBUF + sizeof(struct FILENT);
    wcb->EndBuf = DSKBUF + sizeof(struct FILENT);
    wcb->DskBuf[0].NameF[0] = 0;
    wcb->DskBuf[1].NameF[0] = 0;
    from = Line(FND->FndCur);
    if (*from == ' ') {
        to = wcb->DskBuf[0].NameF;
        for (i = 0; i < 12; i++)
            to[i] = from[3 + i];
        to[12] = 0;
        NormStr(to);
    }
    regs.ax = 0x1900;
    Int21d((struct DosRegs STACK *)&regs);
    if ((u8)((u8)regs.ax + 'A') != md.DosPth[0])
        return TO_FNCEX1;
    return (u32)panel << 16 | TO_F24_02;
}
