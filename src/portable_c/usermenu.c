/*
 * The user menu, of F2 and Shift+F2, and the commands that a line of it
 * or of an extension file stands for.
 *
 * Translated from VCFUNC.INC.
 *
 * A menu file is text. A line that begins at the left margin is a line of
 * the menu: a key, a letter or F1 to F12, and a colon, and behind them
 * what the menu shows; or only that. The lines below it that begin with a
 * space or a tab are its commands, and a line that begins with an
 * apostrophe is a comment. A command that is the name of another menu
 * file, *.mnu and nothing else, shows that menu instead.
 *
 * The file is read into MenuBuf, with the end-of-file character behind
 * it, and the keys of its lines are kept as words from TmpBuf+100 on: the
 * letter in capitals, or the scan code of the function key in the high
 * byte, and 0 for a line without a key.
 *
 * In a command "!" stands for the name of the file under the cursor
 * without its extension, "!.!" for the name with it, "!:" for the letter
 * of the current drive, "!\" for the current directory and "!@" for the
 * name of a file that lists the selected files, which is written then;
 * "!!" is "!". The commands of a line are put into DosBuf, one behind the
 * other, each with its zero byte, and run from there.
 */
#include "vc.h"

u16 F32_70(u16 begin, u16 size);

/* Other files call these: the history's box F32_70, in cmdline.c, and
   Enter and the commands of the viewer Run_00, which is declared in vc.h
   with FndItem. F32_00, the command, is declared in vc.h too: see "The
   commands" there. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen and Cursor1 at its place in the
   command line. SavWin keeps what the screen buffer has where a window is
   to be, ResWin puts it back; MinBox draws a box with a shadow there.
   SftEGA moves a place down on a screen of more than 25 lines, by half of
   what it has more.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo is asked
   about a click on the line `line` of a list: 2 for the first click on
   it, 1 for the second, 0 for a second one that ended elsewhere. ClrKbd
   empties the keyboard buffer if the setup has it so.

   ChgDisk has the disk of a drive that shares a floppy drive put in,
   for the path at `path` in the main data, and is not 0 if the user would
   not.

   PutMnu, Input0, FndItem, DosCom and ClsLine are C, of vc.h, and so are
   F34_30, which looks for the next command of a line, and F34_40, which
   looks at whether a command is the name of a file of the kind that is
   being read, a menu here: then it has put the path of that file at
   TmpBuf. Both return where in MenuBuf, and whether there is no command,
   or whether it is such a name. FndItem returns where the next line of a
   menu is that is neither empty, nor a comment, nor a command, and
   whether there is none, the end of the file. */

/* C routines of other files that vc.h does not have. */
u16 *MvsBw(u16 *screen, const u8 *text, u16 count,
           u16 color);                                  /* screen.c */
const u8 *FndLet(const u8 *text);                       /* string.c */
u8 *MainFil(u8 *to, const u8 *name);
void Help00(void);                                      /* help.c */

/* What keys_t.c has of the menu: the name of the menu file, which
   F34_40 and the editor have too, with the title of the menu behind it,
   and the box that says that no menu file was found. And two names that
   are copied with what follows them: that of the list file, which is
   there too, and the mask that stands behind the name of a directory in
   it. */
extern const u8 Menu_F[];               /* "VC.MNU" */
extern const u8 User_S[];               /* " User Menu " */
extern const struct BOXHD UsrEr_B;      /* "Could not find the menu file" */
extern const u8 List_F[];               /* "vc.lst" */
extern const u8 Mask_S[];               /* "\*.*" */

#define LenCmd 124                      /* Length of command line */

#define Hist_C  23                      /* the colour of the menu */
#define HstCu_C 24                      /* and of the line of its cursor */

/* The end of a line: the two characters in either order. */
#define IS_CRLF(pair) ((pair) == 0x0D0A || (pair) == 0x0A0D)

#define EOF_CHAR 0x1A

/* Where the keys of the lines of the menu are kept. */
#define KEYS ((u16 *)(md.TmpBuf + 100))

/* Not 0 once the list file has been written for a line of the menu. */
#define ListFlg (md.TmpBuf[0])

/* Where the name of the list file is put. */
#define LIST_NAME (md.TmpBuf + 100)

/* Where no more of a command is put into DosBuf. */
#define COMMANDS_END (md.DosBuf + LenBat - 2)

static const u8 CrLf_S[] = { 13, 10 };

/* The word at an address in the main data. */
#define WORD_AT(address) (*(const u16 *)(address))

/* What the menu shows instead of the key bar and the menu bar. */
static void F42_10(void)
{
    md.HlpPage = 17;
    HidCur();
    PutMnu(2);
}

/* Give the line `row` of the menu, whose box is at `begin` and has the
   size `size`, the colour whose number is `color`: all of it behind the
   keys, which are `keys` cells wide. */
static void F32_60(u16 color, u16 begin, i32 row, i32 keys, u16 size)
{
    i32 width;

    begin += 0x205;
    begin += row << 8;
    begin = ADD_LOW(begin, keys);
    width = (u8)(size - 12 - keys);
    MvsColr(BasAdr(begin), (u16)width, Color(color));
    Window(begin, (u16)(0x100 | width));
}

/* What the mouse does in a list that is in a box, at `begin` and of the
   size `size`, while a button of it is down. Returns in the high byte 0
   for nothing, 1 for a click on a line, whose number is in the low byte,
   1Ch for a second click on the same line, which chooses it, and FFh for
   the middle button, or the left one let go outside the box: that leaves
   the list. The right button is as the left one on a line, and does
   nothing elsewhere. While the middle button alone is down the pointer
   has the look of an x. */
u16 F32_70(u16 begin, u16 size)
{
    struct Mouse pointer;
    u16 buttons, look, last = 0, mouse = 0;
    i32 answer, line;

F32_71:
    pointer = GetMous(mouse);
    buttons = pointer.buttons;
    if (buttons == 0)
        goto F32_74;
    mouse = pointer.place;
    ClrKbd();
    look = 'x';
    if (buttons & 3)
        look = 0;
    if (look != last >> 8)
        TypMous(look);
    last = (u16)(look << 8 | buttons);
    if (!(last & 3))
        goto F32_71;
    if (InWind(mouse, begin + 0x204, size - 0x50A))
        goto F32_71;
    goto F32_77;

F32_74:
    TypMous(0);
    if ((u8)last == 2)
        goto F32_76;
    if ((u8)last < 1)
        goto F32_76;
    if ((u8)last != 1)
        goto F32_75;
    if (!InWind(mouse, begin, size - 0x102))
        goto F32_76;
F32_75:
    return 0xFF00 | (u8)last;
F32_76:
    return (u8)last;

F32_77:
    line = (u8)((u16)(mouse - begin) >> 8) - 2;
    answer = DoublMo((u16)((begin + 0x204 & 0xFF)
                           | (u8)(((begin + 0x204) >> 8) + line) << 8),
                     0x100 | (u8)(size - 10), line);
    if (answer == 2)
        return 0x100 | line & 0xFF;
    if (answer == 0)
        return line & 0xFF;             /* F32_76, with the line in AL */
    TypMous(0);
    return 0x1C00;
}

/* Write the list file, in the directory that TEMP names: the names of the
   selected files of the current panel, or of the file under its cursor,
   each in a line, and "\*.*" behind the name of a directory. A list file
   that is there goes first. What was selected is not afterwards, and is
   marked as having been. True if no list was written: the panel shows no
   files, the cursor is on "..", or DOS would not. */
static bool FileLst(void)
{
    struct WCBdef *wcb;
    struct FILENT *file;
    u8 *end;
    struct Selected selected;
    struct Answer asked;
    i32 i, length;
    u16 handle, at;
    bool any;

    end = MvzLine(LIST_NAME, md.TempDir, LenPath);
    for (i = 0; i < 13; i++)
        end[i] = List_F[i];
    if (ChgDisk(LIST_NAME))
        goto FiLs18;
    /* delete the one that is there */
    asked = FileSetAttributes(LIST_NAME, 0x20);
    if (!asked.failed)
        goto FiLs02;
    if (asked.value == 2)
        goto FiLs03;
    if (asked.value != 5)
        goto FiLs18;
FiLs02:
    if (FileDelete(LIST_NAME).failed)
        goto FiLs18;
FiLs03:
    wcb = CurrentPanel;
    if (!FilePan(CurrentPanel))
        goto FiLs18;
    selected = TotSel(CurrentPanel);
    any = selected.files != 0 || selected.directories != 0;
    if (!any) {
        if (wcb->CurAdr >= wcb->EndBuf)
            goto FiLs18;
        if (ENTRY(wcb, wcb->CurAdr)->NameF[0] == '.')
            goto FiLs18;
    }
    asked = FileCreate(LIST_NAME, 0x20);    /* and make it */
    if (asked.failed)
        goto FiLs18;
    handle = asked.value;
    at = wcb->CurAdr;
    if (!any)
        goto FiLs13;
    at = DSKBUF;
FiLs12:
    if (at >= wcb->EndBuf)
        goto FiLs20;
    if (!(ENTRY(wcb, at)->AttrF & ATTR_SELECTED))
        goto FiLs16;
FiLs13:
    file = ENTRY(wcb, at);
    for (length = 0; file->NameF[length] != 0; length++)
        ;
    if (FileWrite(handle, file->NameF, length).failed)
        goto FiLs17;
    if (file->AttrF & ATTR_DIRECTORY)
        if (FileWrite(handle, Mask_S, 4).failed)
            goto FiLs17;
    if (FileWrite(handle, CrLf_S, 2).failed)
        goto FiLs17;
FiLs16:
    at += sizeof(struct FILENT);
    goto FiLs12;

FiLs17:
    FileClose(handle);
FiLs18:
    return true;

FiLs20:
    if (FileClose(handle).failed)
        goto FiLs18;
    for (at = DSKBUF; at < wcb->EndBuf; at += sizeof(struct FILENT)) {
        file = ENTRY(wcb, at);
        file->AttrF &= 0x7F;
        if (file->AttrF & ATTR_SELECTED) {
            file->AttrF &= 0x3F;
            file->AttrF |= 0x80;
        }
    }
    return false;
}

/* Make the commands of a line of a menu or of an extension file, which
   begin at `text` in MenuBuf, into DosBuf, and show the first of them in
   the command line. `view` is true for the file that is being viewed or
   edited, whose name is in ViewFil, and false for the file under the
   cursor of the current panel. Returns true if there is a command to
   run.

   A first command that begins with "@" is run without the prompt and its
   line on the screen. Of a line no more than 123 characters are taken,
   but what stands for "!" is not counted in full, and all commands
   together have 998 bytes. */
bool Run_00(u8 *text, bool view)
{
    const u8 *from = text;              /* SI */
    u8 *to = md.DosBuf;                 /* DI */
    const u8 *name;
    const u8 *more;
    struct WCBdef *wcb;
    const struct MainDat *data = &md;
    u16 pair, c, to_dot;
    i32 left, count;
    bool path, unlisted;

    ListFlg = 0;
Run_01:
    pair = WORD_AT(from);
    if (IS_CRLF(pair))
        goto Run_02;
    if ((u8)pair == ' ' || (u8)pair == '\t')
        goto Run_04;
    if ((u8)pair != '\'')
        goto Run_30;
Run_02:                                 /* to the end of the line */
    pair = WORD_AT(from);
    from += 2;
    if (IS_CRLF(pair))
        goto Run_01;
    from--;
    if ((u8)pair != EOF_CHAR)
        goto Run_02;
    goto Run_30;

Run_04:                                 /* a command */
    from = FndLet(from);
    pair = WORD_AT(from);
    if (IS_CRLF(pair))
        goto Run_02;
    if ((u8)pair == EOF_CHAR)
        goto Run_30;
    left = LenCmd - 1;
    if (to <= md.DosBuf && (u8)pair == '@') {
        md.LoadFl |= 0x80;
        *(u8 *)from = ' ';
    }
Run_05:
    pair = WORD_AT(from);
    from += 2;
    if (IS_CRLF(pair))
        goto Run_07;
    from--;
    c = (u8)pair;
    if (c == EOF_CHAR)
        goto Run_07;
    if (left == 0)
        goto Run_05;
    if (c == '!')
        goto Run_10;
    if (c == '\t')
        c = ' ';
Run_06:
    *to++ = (u8)c;
    if (to >= COMMANDS_END)
        goto Run_30;
    left--;
    goto Run_05;

Run_07:                                 /* the end of the command */
    *to++ = 0;
    if (to >= COMMANDS_END)
        goto Run_30;
    if ((u8)pair == EOF_CHAR)
        goto Run_30;
    goto Run_01;

Run_10:                                 /* "!" and what follows it */
    pair = WORD_AT(from);
    c = (u8)pair;
    if (c == '!') {
        from++;
        goto Run_06;
    }
    if (c == ':') {
        c = md.DosPth[0];
        goto Run_06;
    }
    if (c == '@') {
        from++;
        if (ListFlg == 0) {
            unlisted = FileLst();
            ListFlg = 1;
            if (unlisted)
                Beep();
        }
        more = LIST_NAME;
        path = false;
        goto Run_15;
    }
    if (c != '\\')
        goto Run_18;
    from++;
    if (md.PathEr != 0)
        goto Run_05;
    path = true;
    more = md.DosPth + 2;
Run_15:
    c = *more;
    if (c == 0)
        goto Run_17;
    more++;
    *to++ = (u8)c;
    if (to >= COMMANDS_END)
        goto Run_30;
    if (--left != 0)
        goto Run_15;
    goto Run_05;
Run_17:                                 /* a path ends in a backslash */
    if (!path)
        goto Run_05;
    c = '\\';
    if (more[-1] == c)
        goto Run_05;
    goto Run_06;

Run_18:                                 /* the name of the file */
    to_dot = '.';
    if (pair == ('!' << 8 | '.')) {
        from += 2;
        to_dot = 0;
    }
    if (view) {
        name = CutPath(md.ViewFil);
        more = md.ViewFil;
        while (more != name) {          /* the path in front of it */
            *to++ = *more++;
            if (to >= COMMANDS_END)
                goto Run_30;
        }
    } else {
        wcb = CurrentPanel;
        /* The assembly leaves DS on the panel when the commands have no
           room for the name: see ClsLine below. */
        data = PANEL_AS_MAIN(CurrentPanel);
        if (wcb->Visible == 0)
            goto Run_23;
        if (wcb->WinTyp != 0)
            goto Run_23;
        if (wcb->CurAdr >= wcb->EndBuf)
            goto Run_23;
        name = ENTRY(wcb, wcb->CurAdr)->NameF;
    }
    for (;;) {
        c = *name++;
        if (c == to_dot)
            break;
        if (c == 0)
            break;
        *to++ = (u8)c;
        if (to >= COMMANDS_END)
            goto Run_30;
        if (--left == 0)
            break;
    }
Run_23:
    data = &md;
    goto Run_05;

Run_30:                                 /* that is all of them */
    *to = 0;
    *(u16 *)COMMANDS_END = 0;
    for (count = 0; count < 255; count++)
        if (md.DosBuf[count] == 0)
            break;
    md.DosCol = (u8)count;
    DosCom();
    Cursor1();
    if (md.DosBuf[0] == 0)
        return false;
    if (md.LoadFl & 0x80)
        ClsLine((u16)(md.DosLin << 8), data);
    return true;
}

/* The user menu: that of the current directory, or else the main one, of
   the program's directory; the main one at once if `main` is true.
   Returns true if a line was chosen and has a command to run, which
   Run_00 has put into DosBuf then. */
bool F32_00(bool main)
{
    struct Entry item;
    struct Named named;
    struct Next found;
    struct Key pressed;
    struct Answer opened;
    struct Moved read;
    const u8 *text;                     /* SI */
    u16 *screen;                        /* DI */
    u16 *key;                           /* BP */
    u16 begin, size;                    /* BX and DX: the box */
    i32 keys;                           /* CL: how wide the keys are */
    i32 row;                            /* CH: the line of the cursor */
    const u8 *include;                  /* DX: what ^I stands for in a box */
    u16 c, next, color, scan, place, answer;
    i32 width, widest, lines, length, limit, i, target, room;
    u8 *command;

    if (main)
        goto F42_00;
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = Menu_F[i];
    F42_10();
    include = md.TmpBuf;
    if (ChgDisk(md.TmpBuf))
        goto F32_04;
    opened = OpenFil(md.TmpBuf, 0x40);
    if (!opened.failed)
        goto F32_05;
    if ((u8)opened.value == 2)
        goto F32_01;
    if ((u8)opened.value != 5)
        goto F32_04;
    goto F32_01;

F42_00:                                 /* Shift+F2 */
    F42_10();
F32_01:                                 /* the main menu */
    MainFil(md.TmpBuf, Menu_F);
F32_02:
    include = md.TmpBuf;
    if (ChgDisk(md.TmpBuf))
        goto F32_04;
    opened = OpenFil(md.TmpBuf, 0x40);
    if (!opened.failed)
        goto F32_05;
    if ((u8)opened.value > 3)
        goto F32_04;
F32_03:
    Dialog(&UsrEr_B, include, 0, 0, &md);
F32_04:
    return false;

F32_05:                                 /* read it */
    read = FileRead(opened.value, md.MenuBuf, LenMenu - 1);
    if (FileClose(opened.value).failed)
        goto F32_04;
    if (read.failed)
        goto F32_04;
    md.MenuBuf[read.count] = EOF_CHAR;
    include = md.MenuBuf;
    item = FndItem(md.MenuBuf);
    if (item.end)
        goto F32_03;
    text = item.at;

    /* The keys of its lines, how wide they are, and how wide the widest
       line behind them is. */
    keys = 0;
    widest = 0;
    lines = 0;
    key = KEYS;
F32_10:
    width = 0;
    length = 0;
    *key = 0;
    c = text[0];
    next = text[1];
    if (c <= ' ' || c == 0xFF || c == 0x7F)
        goto F32_15;
    c = UpCase((u16)(next << 8 | c));
    next = c >> 8;
    c &= 0xFF;
    if (next == ':')
        goto F32_13;
    if (c != 'F')
        goto F32_15;
    if (next < '1' || next > '9')
        goto F32_15;
    scan = next + (0x3B - '1');         /* F1 to F9 */
    if (text[2] == ':')
        goto F32_12;
    if (next != '1')
        goto F32_15;
    if (text[3] != ':')
        goto F32_15;
    if (text[2] > '2')
        goto F32_15;
    scan = 0x44;                        /* F10 */
    if (text[2] == '0')
        goto F32_11;
    if (text[2] < '0')
        goto F32_15;
    scan = (u8)(text[2] + (0x85 - '1'));  /* F11 and F12 */
F32_11:
    length++;
F32_12:
    length++;
    c = 0;
    goto F32_14;
F32_13:
    scan = 0;
F32_14:
    length += 2;
    *key = (u16)(scan << 8 | c);
    text += length;
    while (*text == ' ' || *text == '\t')
        text++;
    if (length > keys)
        keys = length;
F32_15:                                 /* how long the line is */
    c = WORD_AT(text);
    text += 2;
    if (IS_CRLF(c))
        goto F32_16;
    text--;
    c &= 0xFF;
    if (c == EOF_CHAR)
        goto F32_16;
    if (width >= 80)
        goto F32_15;
    width++;
    goto F32_15;
F32_16:
    if (width > widest)
        widest = width;
    key++;
    lines++;
    if ((u8)c == EOF_CHAR)
        goto F32_18;
    if ((u8)lines >= (u8)(md.Lines - 6))
        goto F32_18;
    item = FndItem(text);
    if (item.end)
        goto F32_18;
    text = item.at;
    goto F32_10;

F32_18:                                 /* the box */
    key = KEYS;
    widest = (u8)(widest + keys);
    if (widest > 63)
        widest = 63;
    if (widest < 11)
        widest = 11;
    size = (u16)(((lines & 0xFF) << 8 | widest) + 0x50E);
    begin = SftEGA(0x0C28);
    limit = (u8)((begin >> 8) - 7);
    begin -= (u8)(((size >> 8) + 1 & 0xFF) >> 1) << 8
        | (u8)((size - 1 & 0xFF) >> 1);
    if (begin >> 8 > limit)
        begin = (u16)(limit << 8 | begin & 0xFF);
    SavWin(begin, size, md.WinBuf);
    color = (u16)(Color(Hist_C) << 8);
    MinBox(begin, size, color);
    MvsBw(BasAdr((begin + 0x100 & 0xFF00) | 34), User_S, 11, color);

    /* Its lines. */
    lines = (u8)((size >> 8) - 5);
    place = begin + 0x205;
    text = md.MenuBuf;
F32_22:
    text = FndItem(text).at;
    screen = BasAdr(place);
    if (*key != 0) {
        i = 0;
        c = *text++;
        do {
            *(u8 *)(screen + i) = (u8)UpCase(c);
            i++;
            c = *text++;
        } while (c != ':');
        while (*text == ' ' || *text == '\t')
            text++;
    }
    screen += keys + 1;
    room = (u8)(size - 14 - keys);
F32_26:
    c = WORD_AT(text);
    text += 2;
    if (IS_CRLF(c))
        goto F32_28;
    text--;
    c &= 0xFF;
    if (c == EOF_CHAR)
        goto F32_29;
    if (room == 0)
        goto F32_26;
    if (c == '\t')
        c = ' ';
    *(u8 *)screen = (u8)c;
    screen++;
    room--;
    goto F32_26;
F32_28:
    key++;
    place += 0x100;
    if (--lines != 0)
        goto F32_22;
F32_29:
    Window(begin, size);
    row = 0;
    md.MousTim = md.MousTim & 0xFFFF | 0xFFFF0000UL;
    md.MousEnt = 0;

F32_30:                                 /* the cursor */
    F32_60(HstCu_C, begin, row, keys, size);
F32_31:
    if (GetMous(0).buttons)
        goto F32_45;
    md.MousEnt = 0;
    pressed = Input0(1);
    answer = pressed.key;
    if (pressed.clicked)
        goto F32_33;
    if (answer >> 8 == 0xFC)
        goto F32_45;
    if (answer >> 8 != 0)
        switch (answer & 0xFF) {
        case 0:
            switch (answer >> 8) {
            case 0x48:
                goto F32_36;
            case 0x50:
                goto F32_37;
            case 0x47:
            case 0x49:
                goto F32_38;
            case 0x4F:
            case 0x51:
                goto F32_39;
            }
            break;
        case 0x0D:
            goto F32_50;
        case 0x1B:
            goto F32_55;
        }
    if ((answer & 0xFF) != 0)
        answer = UpCase(answer & 0xFF);
    lines = (u8)((size >> 8) - 5);      /* the key of a line */
    for (i = 0; i < lines; i++)
        if (KEYS[i] == answer) {
            row = i;
            goto F32_50;
        }
F32_33:
    if (answer == 0x3B00) {
        Help00();
        goto F32_31;
    }
    if (answer != 0x4400)
        goto F32_31;
    goto F32_55;

F32_36:                                 /* Up */
    if (row == 0)
        goto F32_39;
    target = row - 1;
    goto F32_40;
F32_37:                                 /* Down */
    if (row >= (u8)((size >> 8) - 6))
        goto F32_38;
    target = row + 1;
    goto F32_40;
F32_38:                                 /* Home, PgUp */
    target = 0;
    goto F32_40;
F32_39:                                 /* End, PgDn */
    target = (u8)((size >> 8) - 6);
F32_40:
    if (target == row)
        goto F32_31;
    F32_60(Hist_C, begin, row, keys, size);
    row = target;
    goto F32_30;

F32_45:                                 /* the mouse */
    answer = F32_70(begin, size);
    if (answer >> 8 < 1)
        goto F32_31;
    if (answer >> 8 == 1) {
        target = answer & 0xFF;
        goto F32_40;
    }
    if (answer >> 8 != 0x1C)
        goto F32_55;

F32_50:                                 /* Enter: the commands of the line */
    text = md.MenuBuf;
    i = (u8)(row + 1);
F32_51:
    text = FndItem(text).at;
F32_52:
    answer = WORD_AT(text);
    text += 2;
    if (IS_CRLF(answer))
        goto F32_53;
    if ((u8)answer == EOF_CHAR)
        goto F32_55;                    /* it has none */
    text--;
    goto F32_52;
F32_53:
    i = (u8)(i - 1);
    if (i != 0)
        goto F32_51;
    answer = 0;

F32_55:                                 /* Esc, and the box goes */
    ResWin(begin, size, md.WinBuf);
    Window(begin, size);
    if (answer != 0) {
        md.MenuFl = 0;
        return false;
    }
    found = F34_30(text);
    command = found.at;
    if (found.none)
        goto F32_57;
    named = F34_40(found.at, 0);
    command = named.at;
    /* BP, which was on the keys, is 0 from here on, and Run_00 takes it
       for whether a file is being viewed. Where F34_30 finds no command
       it is still on the keys, and Run_00 is told that one is. */
    key = 0;
    if (named.file)
        goto F32_02;                    /* another menu */
F32_57:
    md.MenuFl = md.AutoMnu;
    return Run_00(command, key != 0);
}
