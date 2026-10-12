/*
 * Find file: the search of a directory and all below it for files by
 * their names and by what they contain, the list of what was found, and
 * the keys that are taken while the search runs.
 *
 * Translated from VCFIND.INC.
 *
 * What the command asks and what it does with the answer, F67_00, is
 * still assembly. It sets up what is kept here, has FindFil search the
 * directory at TmpBuf, and lets FndMove take the keys once the search is
 * over. FindFil is also what DirFunc, which goes through the directories
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
u16 FndMove(u16 key, u16 mode, u16 place, u16 size);
void FndCurs(u16 color, u16 line, u16 first, u16 place, u16 size);
u8 __far *FndAdr(u16 number);

/* How the assembly passes arguments to these, and what it expects back. */
#pragma aux FindFil "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux FndMove "*_c" __far parm [ax] [cx] [bx] [dx] value [ax] modify exact [ax]
#pragma aux FndCurs "*_c" __far parm [ax] [si] [di] [bx] [dx] modify exact [ax]
#pragma aux FndAdr  "*_c" __far parm [ax] value [dx si] modify exact [ax dx si]

/* Routines that are still assembly, by their entries for C.

   GetMous returns the buttons of the mouse in the low word and where it
   is in the high one, the row in its high byte; without a mouse that is
   `place` as it came. ClrKbd empties the keyboard buffer, if the setup
   says so, and TypMous sets the character that the mouse pointer is.
   InWind is not 0 if a place is outside a window. DoublMo returns 2 for
   a click that may be the first of two, 1 for the second of two on the
   line `line`, and 0 for a second one that was let go elsewhere.

   MnMn10 has the menu at `place` take a key: `table` is where the
   assembly describes it, an offset in its code segment. It returns the
   key in the low word, the item in the next byte, and something that is
   not 0 in the highest byte if the menu has dealt with the key. MnMn40
   draws the menu with `item` chosen, in the two colours of `colors`, and
   returns two bytes of the second of them.

   PgUp and PgDn move the cursor of a list a page, `rows` lines: they
   return the cursor in the low word and the first line shown in the high
   one. Int10m calls the BIOS for the screen, and returns CX as that
   leaves it. Int16 calls the BIOS for the keyboard: the high word of
   what it returns is 1 if the zero flag was set. DirFunc calls what
   FncAddr names for every file of the directory at TmpBuf, and returns
   what the last call did, or 100h when it was stopped. Help00 is the
   help, which is C. */
u32 GetMous(u16 place);
void ClrKbd(void);
void TypMous(u16 pointer);
u16 InWind(u16 where, u16 place, u16 size);
u16 DoublMo(u16 place, u16 size, u16 line);
u32 MnMn10(u16 key, u16 place, u16 item, u16 size, u16 table);
u16 MnMn40(u16 colors, u16 place, u16 item, u16 size, u16 table);
u32 PgUp(u16 cursor, u16 first, u16 rows);
u32 PgDn(u16 cursor, u16 first, u16 rows, u16 lines);
u16 Int10m(u16 ax, u16 bx, u16 cx, u16 dx);
u32 Int16(u16 ax);
u16 ChgDisk(const u8 *path);
u16 DirFunc(void);
void Help00(void);                                      /* help.c */
#pragma aux GetMous "*_a" __far parm [cx] value [dx ax] modify exact [ax cx dx es]
#pragma aux ClrKbd  "*_a" __far modify exact [es]
#pragma aux TypMous "*_a" __far parm [ax] modify exact [es]
#pragma aux InWind  "*_a" __far parm [cx] [bx] [dx] value [ax] modify exact [ax es]
#pragma aux DoublMo "*_a" __far parm [bx] [dx] [si] value [ax] modify exact [ax cx es]
#pragma aux MnMn10  "*_a" __far parm [ax] [bx] [cx] [dx] [di] value [dx ax] modify exact [ax cx dx si es]
#pragma aux MnMn40  "*_a" __far parm [ax] [bx] [cx] [dx] [di] value [ax] modify exact [ax si es]
#pragma aux PgUp    "*_a" __far parm [si] [di] [ax] value [dx ax] modify exact [ax dx si di]
#pragma aux PgDn    "*_a" __far parm [si] [di] [ax] [bx] value [dx ax] modify exact [ax dx si di]
#pragma aux Int10m  "*_a" __far parm [ax] [bx] [cx] [dx] value [cx] modify exact [ax bx cx dx es]
#pragma aux Int16   "*_a" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux ChgDisk "*_a" __far parm [dx] value [ax] modify exact [ax es]
#pragma aux DirFunc "*_a" __far value [ax] modify exact [ax bx cx dx si di es]
#pragma aux Help00  "*_c" __far modify exact [ax]

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
u8 __far *UnConv(u8 __far *name, const u8 __far *buffer);   /* mask.c */
void V81_00(void);                                      /* view.c */
u32 V82_00(u16 segment, u16 at, u16 count);
#pragma aux PutDate "*_c" __far parm [ax] [bx] modify exact [ax bx cx dx]
#pragma aux BufDate "*_c" __far parm [bx] [di] value [di] modify exact [ax bx di]
#pragma aux PutTime "*_c" __far parm [ax] [bx] modify exact [ax bx cx dx]
#pragma aux BufTime "*_c" __far parm [bx] [di] value [di] modify exact [ax bx di]
#pragma aux FilZero "*_c" __far parm [es si] [cx] modify exact [ax]
#pragma aux UnConv  "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]
#pragma aux V81_00  "*_c" __far modify exact [ax]
#pragma aux V82_00  "*_c" __far parm [bx] [si] [cx] value [dx ax] modify exact [ax dx]

/* Objects of the assembly's code segment: Mask_S is \*.*, and four bytes
   are copied from it or from behind it; Find5_D describes the menu of a
   search that runs, and only its offset is used here. */
extern const u8 __far Mask_S[];
extern const u8 __far Find5_D[];
#pragma aux Mask_S "*"
#pragma aux Find5_D "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define SEGMENT(pointer) ((u16)((u32)(const void __far *)(pointer) >> 16))

/* What the size column says for a directory. */
static const u8 CODE SubDir[] = "SUB-DIR";

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

/* For the assembly: the address of a line, for DS and SI. */
u8 __far *FndAdr(u16 number)
{
    return Line(number);
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
void FndCurs(u16 color, u16 line, u16 first, u16 place, u16 size)
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
    u32 mouse;
    u16 buttons, was, last, target, clicks;

    mouse = GetMous(at);
    for (;;) {
        /* FnMv01 */
        buttons = (u16)mouse;
        at = (u16)(mouse >> 16);
        ClrKbd();
        was = at;
        if (buttons & 4) {
            TypMous('x');
            do {
                mouse = GetMous(at);
                buttons = (u16)mouse;
                at = (u16)(mouse >> 16);
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
        if ((u16)mouse != 0)
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
u16 FndMove(u16 key, u16 mode, u16 place, u16 size)
{
    u16 line = FND->FndCur;
    u16 first = FND->FndFrst;
    u16 ask, rows, old, wasLine, wasFirst, moved, count, at, number;
    u16 scroll, color;
    u32 paged;

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
        line = (u16)paged;
        first = (u16)(paged >> 16);
        break;
    case ASK_PGDN:
        paged = PgDn(line, first, rows, FND->FndNumb);
        line = (u16)paged;
        first = (u16)(paged >> 16);
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
                        (((rows - 1) & 0xFF) << 8 | 63) + at);
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
    u16 key, mode, place, size, menu, item, go;

    for (;;) {
        /* The place of the mouse, if there is one, gets to FndMove as
           the high byte of `mode`, where nothing reads it. */
        answer = GetMous(0);
        key = (u16)answer;
        mode = (u16)(answer >> 16);
        if (key != 0) {
            key = KEY_MOUSE << 8 | key & 0xFF;
        } else {
            if (Int16(0x100) >> 16) {
                go = 1;
                break;
            }
            key = (u16)Int16(0);
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
    u32 opened;
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
        if (opened >> 16) {
            if ((u16)opened == ERR_NOT_READY)
                return 0x100;
            goto next;
        }
        handle = (u16)opened;
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
