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

u16 F32_00(u16 main);
u16 F32_70(u16 begin, u16 size);
u16 Run_00(u16 text, u16 view);

/* How the assembly calls these (see the stubs in port/asm/VCFUNC.INC),
   and F32_70 how the history's box does, in port/cmdline.c. */
#pragma aux F32_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux F32_70 "*_c" __far parm [bx] [dx] value [ax] modify exact [ax]
#pragma aux Run_00 "*_c" __far parm [ax] [dx] value [ax] modify exact [ax]

/* Routines of the rest of the program that are still assembly, by their
   entries for C. HidCur puts the cursor below the screen and Cursor1 at
   its place in the command line. SavWin keeps what the screen
   buffer has where a window is to be, ResWin puts it back; MinBox draws a
   box with a shadow there. SftEGA moves a place down on a screen of more
   than 25 lines, by half of what it has more.

   GetMous returns the buttons of the mouse that are down, and in the high
   word where the mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo is asked
   about a click on the line `line` of a list: 2 for the first click on
   it, 1 for the second, 0 for a second one that ended elsewhere. ClrKbd
   empties the keyboard buffer if the setup has it so.

   ChgDisk has the disk of a drive that shares a floppy drive put in,
   for the path at `path` in the main data, and is not 0 if the user would
   not.

   F34_30 looks for the next command of a line, and F34_40 at whether a
   command is the name of a file of the kind that is being read, a menu
   here: then it has put the path of that file at TmpBuf. Both return a
   place in the high word, and the low one is not 0 if there is none.

   PutMnu, Input0, FndItem, DosCom and ClsLine are C, of vc.h. */
void HidCur(void);
void Cursor1(void);
void SavWin(u16 begin, u16 size, u16 buffer);
void ResWin(u16 begin, u16 size, u16 buffer);
void MinBox(u16 begin, u16 size, u16 color);
u16 SftEGA(u16 place);
u32 GetMous(u16 place);
void TypMous(u16 look);
u16 InWind(u16 place, u16 begin, u16 size);
u16 DoublMo(u16 begin, u16 size, u16 line);
void ClrKbd(void);
u16 ChgDisk(u16 path);
u32 F34_30(u16 text);
u32 F34_40(u16 text, u16 view);
#pragma aux HidCur  "*_a" __far modify exact [es]
#pragma aux Cursor1 "*_a" __far modify exact [es]
#pragma aux SavWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux MinBox  "*_a" __far parm [bx] [dx] [ax] modify exact [es]
#pragma aux SftEGA  "*_a" __far parm [bx] value [bx] modify exact [bx es]
#pragma aux GetMous "*_a" __far parm [cx] value [dx ax] modify exact [ax cx dx es]
#pragma aux TypMous "*_a" __far parm [ax] modify exact [es]
#pragma aux InWind  "*_a" __far parm [cx] [bx] [dx] value [ax] modify exact [ax es]
#pragma aux DoublMo "*_a" __far parm [bx] [dx] [si] value [ax] modify exact [ax cx es]
#pragma aux ClrKbd  "*_a" __far modify exact [es]
#pragma aux ChgDisk "*_a" __far parm [dx] value [ax] modify exact [ax es]
#pragma aux F34_30  "*_a" __far parm [si] value [dx ax] modify exact [ax dx si es]
#pragma aux F34_40  "*_a" __far parm [si] [cx] value [dx ax] modify exact [ax bx cx dx si di es]

/* FndItem of vc.h, which is C, for a place in the main data, with its
   answer as F34_30 and F34_40 have theirs: where the next line of a menu
   is that is neither empty, nor a comment, nor a command, in the high
   word, and the low one not 0 if there is none. */
static u32 MenuItem(u16 text)
{
    u32 item = FndItem((const u8 *)text);

    return (u32)(u16)item << 16 | (u16)(item >> 16);
}

/* C routines of other files that vc.h does not have. */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
const u8 __far *FndLet(const u8 __far *text);           /* string.c */
u8 __far *MainFil(u8 __far *to, const u8 __far *name);
u32 TotSel(u16 panel);                                  /* files.c */
void Help00(void);                                      /* help.c */
#pragma aux MvsBw   "*_c" __far parm [es di] [dx si] [cx] [ax] value [es di] modify exact [di es]
#pragma aux FndLet  "*_c" __far parm [es si] value [es si] modify exact [si es]
#pragma aux MainFil "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]
#pragma aux TotSel  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux Help00  "*_c" __far modify exact [ax]

/* What the assembly keeps of the menu, because its routines read it
   there: the name of the menu file, which F34_40 and the editor have too,
   with the title of the menu behind it, and the box that says that no
   menu file was found. And two names that are copied with what follows
   them in the assembly: that of the list file, and the mask that stands
   behind the name of a directory in it. */
extern const u8 __far Menu_F[];         /* "VC.MNU" */
extern const u8 __far User_S[];         /* " User Menu " */
extern const u8 __far UsrEr_B[];        /* "Could not find the menu file" */
extern const u8 __far List_F[];         /* "vc.lst" */
extern const u8 __far Mask_S[];         /* "\*.*" */
#pragma aux Menu_F "*"
#pragma aux User_S "*"
#pragma aux UsrEr_B "*"
#pragma aux List_F "*"
#pragma aux Mask_S "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

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

static const u8 CODE CrLf_S[] = { 13, 10 };

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
static void F32_60(u16 color, u16 begin, u16 row, u16 keys, u16 size)
{
    u16 width;

    begin += 0x205;
    begin += row << 8;
    begin = ADD_LOW(begin, keys);
    width = (u8)(size - 12 - keys);
    MvsColr(BasAdr(begin), width, Color(color));
    Window(begin, 0x100 | width);
}

/* What the mouse does in a list that is in a box, at `begin` and of the
   size `size`, while a button of it is down. Returns in the high byte 0
   for nothing, 1 for a click on a line, whose number is in the low byte,
   1Ch for a second click on the same line, which chooses it, and FFh for
   the right button, or the left one let go outside the box: that leaves
   the list. The middle button does nothing. */
u16 F32_70(u16 begin, u16 size)
{
    u32 pair;
    u16 buttons, look, last = 0, mouse = 0, line, answer;

F32_71:
    pair = GetMous(mouse);
    buttons = LOW(pair);
    if (buttons == 0)
        goto F32_74;
    mouse = HIGH(pair);
    ClrKbd();
    look = 'x';
    if (buttons & 3)
        look = 0;
    if (look != last >> 8)
        TypMous(look);
    last = look << 8 | buttons;
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
    line = (u16)(signed char)((u8)((u16)(mouse - begin) >> 8) - 2);
    answer = DoublMo((begin + 0x204 & 0xFF) | (u8)(((begin + 0x204) >> 8)
                                                   + line) << 8,
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
   marked as having been. Not 0 if no list was written: the panel shows no
   files, the cursor is on "..", or DOS would not. */
static u16 FileLst(void)
{
    struct DosRegs regs;
    struct WCBdef __far *wcb;
    struct FILENT __far *file;
    u8 __far *end;
    u32 selected;
    u16 i, handle, any, at, length;

    end = MvzLine(LIST_NAME, md.TempDir, LenPath);
    for (i = 0; i < 13; i++)
        end[i] = List_F[i];
    regs.bx = 0;
    regs.si = 0;
    regs.di = 0;
    regs.name = LIST_NAME;
    if (ChgDisk((u16)LIST_NAME))
        goto FiLs18;
    regs.cx = 0x20;                     /* delete the one that is there */
    regs.ax = 0x4301;
    if (!Intr21((struct DosRegs STACK *)&regs))
        goto FiLs02;
    if (regs.ax == 2)
        goto FiLs03;
    if (regs.ax != 5)
        goto FiLs18;
FiLs02:
    regs.ax = 0x4100 | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto FiLs18;
FiLs03:
    wcb = PANEL(md.WCBcrn);
    if (!FilePan(md.WCBcrn))
        goto FiLs18;
    selected = TotSel(md.WCBcrn);
    any = HIGH(selected) | LOW(selected);
    if (any == 0) {
        if (wcb->CurAdr >= wcb->EndBuf)
            goto FiLs18;
        if (ENTRY(wcb, wcb->CurAdr)->NameF[0] == '.')
            goto FiLs18;
    }
    regs.cx = 0x20;                     /* and make it */
    regs.ax = 0x3C00 | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto FiLs18;
    handle = regs.ax;
    regs.bx = handle;
    at = wcb->CurAdr;
    if (any == 0)
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
    regs.name = file->NameF;
    regs.cx = length;
    regs.ax = 0x4000;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto FiLs17;
    if (file->AttrF & ATTR_DIRECTORY) {
        regs.name = Mask_S;
        regs.cx = 4;
        regs.ax = 0x4000 | regs.ax & 0xFF;
        if (Intr21((struct DosRegs STACK *)&regs))
            goto FiLs17;
    }
    regs.name = CrLf_S;
    regs.cx = 2;
    regs.ax = 0x4000 | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto FiLs17;
FiLs16:
    at += sizeof(struct FILENT);
    goto FiLs12;

FiLs17:
    regs.ax = 0x3E00 | regs.ax & 0xFF;
    Intr21((struct DosRegs STACK *)&regs);
FiLs18:
    return 1;

FiLs20:
    regs.ax = 0x3E00 | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto FiLs18;
    for (at = DSKBUF; at < wcb->EndBuf; at += sizeof(struct FILENT)) {
        file = ENTRY(wcb, at);
        file->AttrF &= 0x7F;
        if (file->AttrF & ATTR_SELECTED) {
            file->AttrF &= 0x3F;
            file->AttrF |= 0x80;
        }
    }
    return 0;
}

/* Make the commands of a line of a menu or of an extension file, which
   begin at `text` in MenuBuf, into DosBuf, and show the first of them in
   the command line. `view` is not 0 for the file that is being viewed or
   edited, whose name is in ViewFil, and 0 for the file under the cursor
   of the current panel. Returns 1 if there is a command to run.

   A first command that begins with "@" is run without the prompt and its
   line on the screen. Of a line no more than 123 characters are taken,
   but what stands for "!" is not counted in full, and all commands
   together have 998 bytes. */
u16 Run_00(u16 text, u16 view)
{
    const u8 *from = (const u8 *)text;  /* SI */
    u8 *to = md.DosBuf;                 /* DI */
    const u8 __far *name;
    const u8 *more;
    struct WCBdef __far *wcb;
    u16 pair, c, left, to_dot, path, segment = MAIN_SEG;

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
    from = (const u8 *)(u16)(u32)FndLet(from);
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
            c = FileLst();
            ListFlg = 1;
            if (c)
                Beep();
        }
        more = LIST_NAME;
        path = 0;
        goto Run_15;
    }
    if (c != '\\')
        goto Run_18;
    from++;
    if (md.PathEr != 0)
        goto Run_05;
    path = 1;
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
    if (view != 0) {
        name = CutPath(md.ViewFil);
        more = md.ViewFil;
        for (c = (u16)(u32)name - (u16)md.ViewFil; c != 0; c--) {
            *to++ = *more++;
            if (to >= COMMANDS_END)
                goto Run_30;
        }
    } else {
        wcb = PANEL(md.WCBcrn);
        /* The assembly leaves DS on the panel when the commands have no
           room for the name: see ClsLine below. */
        segment = md.WCBcrn;
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
    segment = MAIN_SEG;
    goto Run_05;

Run_30:                                 /* that is all of them */
    *to = 0;
    *(u16 *)COMMANDS_END = 0;
    for (c = 0; c < 255; c++)
        if (md.DosBuf[c] == 0)
            break;
    md.DosCol = (u8)c;
    DosCom();
    Cursor1();
    if (md.DosBuf[0] == 0)
        return 0;
    if (md.LoadFl & 0x80)
        ClsLine((u16)md.DosLin << 8, segment);
    return 1;
}

/* The user menu: that of the current directory, or else the main one, of
   the program's directory; the main one at once if `main` is not 0.
   Returns 1 if a line was chosen and has a command to run, which Run_00
   has put into DosBuf then. */
u16 F32_00(u16 main)
{
    struct DosRegs regs;
    u32 opened, pair;
    const u8 *text;                     /* SI */
    u16 *screen;                        /* DI */
    u16 *key;                           /* BP */
    u16 begin, size;                    /* BX and DX: the box */
    u16 keys;                           /* CL: how wide the keys are */
    u16 row;                            /* CH: the line of the cursor */
    u16 include, failed, count, c, next, width, widest, lines, length;
    u16 limit, i, to, answer, room, command;

    if (main)
        goto F42_00;
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = Menu_F[i];
    F42_10();
    include = (u16)md.TmpBuf;
    if (ChgDisk((u16)md.TmpBuf))
        goto F32_04;
    opened = OpenFil(md.TmpBuf, 0x40);
    if (!HIGH(opened))
        goto F32_05;
    if ((u8)opened == 2)
        goto F32_01;
    if ((u8)opened != 5)
        goto F32_04;
    goto F32_01;

F42_00:                                 /* Shift+F2 */
    F42_10();
F32_01:                                 /* the main menu */
    MainFil(md.TmpBuf, Menu_F);
F32_02:
    include = (u16)md.TmpBuf;
    if (ChgDisk((u16)md.TmpBuf))
        goto F32_04;
    opened = OpenFil(md.TmpBuf, 0x40);
    if (!HIGH(opened))
        goto F32_05;
    if ((u8)opened > 3)
        goto F32_04;
F32_03:
    Dialog(OFFSET(UsrEr_B), include, 0, 0, MAIN_SEG);
F32_04:
    return 0;

F32_05:                                 /* read it */
    regs.ax = 0x3F00 | LOW(opened) & 0xFF;
    regs.bx = LOW(opened);
    regs.cx = LenMenu - 1;
    regs.name = md.MenuBuf;
    regs.si = 0;
    regs.di = 0;
    failed = Intr21((struct DosRegs STACK *)&regs);
    count = regs.ax;
    regs.ax = 0x3E00 | count & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto F32_04;
    if (failed)
        goto F32_04;
    md.MenuBuf[count] = EOF_CHAR;
    include = (u16)md.MenuBuf;
    pair = MenuItem((u16)md.MenuBuf);
    if (LOW(pair))
        goto F32_03;
    text = (const u8 *)HIGH(pair);

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
    c = UpCase(next << 8 | c);
    next = c >> 8;
    c &= 0xFF;
    if (next == ':')
        goto F32_13;
    if (c != 'F')
        goto F32_15;
    if (next < '1' || next > '9')
        goto F32_15;
    to = next + (0x3B - '1');           /* F1 to F9 */
    if (text[2] == ':')
        goto F32_12;
    if (next != '1')
        goto F32_15;
    if (text[3] != ':')
        goto F32_15;
    if (text[2] > '2')
        goto F32_15;
    to = 0x44;                          /* F10 */
    if (text[2] == '0')
        goto F32_11;
    if (text[2] < '0')
        goto F32_15;
    to = (u8)(text[2] + (0x85 - '1'));  /* F11 and F12 */
F32_11:
    length++;
F32_12:
    length++;
    c = 0;
    goto F32_14;
F32_13:
    to = 0;
F32_14:
    length += 2;
    *key = to << 8 | c;
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
    pair = MenuItem((u16)text);
    if (LOW(pair))
        goto F32_18;
    text = (const u8 *)HIGH(pair);
    goto F32_10;

F32_18:                                 /* the box */
    key = KEYS;
    widest = (u8)(widest + keys);
    if (widest > 63)
        widest = 63;
    if (widest < 11)
        widest = 11;
    size = ((lines & 0xFF) << 8 | widest) + 0x50E;
    begin = SftEGA(0x0C28);
    limit = (u8)((begin >> 8) - 7);
    begin -= (u8)(((size >> 8) + 1 & 0xFF) >> 1) << 8
        | (u8)((size - 1 & 0xFF) >> 1);
    if (begin >> 8 > limit)
        begin = limit << 8 | begin & 0xFF;
    SavWin(begin, size, (u16)md.WinBuf);
    c = Color(Hist_C) << 8;
    MinBox(begin, size, c);
    MvsBw(BasAdr((begin + 0x100 & 0xFF00) | 34), User_S, 11, c);

    /* Its lines. */
    lines = (u8)((size >> 8) - 5);
    to = begin + 0x205;
    text = md.MenuBuf;
F32_22:
    text = (const u8 *)HIGH(MenuItem((u16)text));
    screen = BasAdr(to);
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
    to += 0x100;
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
    if (LOW(GetMous(0)))
        goto F32_45;
    md.MousEnt = 0;
    pair = Input0(1);
    answer = LOW(pair);
    if (HIGH(pair))
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
    to = row - 1;
    goto F32_40;
F32_37:                                 /* Down */
    if (row >= (u8)((size >> 8) - 6))
        goto F32_38;
    to = row + 1;
    goto F32_40;
F32_38:                                 /* Home, PgUp */
    to = 0;
    goto F32_40;
F32_39:                                 /* End, PgDn */
    to = (u8)((size >> 8) - 6);
F32_40:
    if (to == row)
        goto F32_31;
    F32_60(Hist_C, begin, row, keys, size);
    row = to;
    goto F32_30;

F32_45:                                 /* the mouse */
    answer = F32_70(begin, size);
    if (answer >> 8 < 1)
        goto F32_31;
    if (answer >> 8 == 1) {
        to = answer & 0xFF;
        goto F32_40;
    }
    if (answer >> 8 != 0x1C)
        goto F32_55;

F32_50:                                 /* Enter: the commands of the line */
    text = md.MenuBuf;
    i = (u8)(row + 1);
F32_51:
    text = (const u8 *)HIGH(MenuItem((u16)text));
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
    ResWin(begin, size, (u16)md.WinBuf);
    Window(begin, size);
    if (answer != 0) {
        md.MenuFl = 0;
        return 0;
    }
    pair = F34_30((u16)text);
    command = HIGH(pair);
    if (LOW(pair))
        goto F32_57;
    pair = F34_40(command, 0);
    command = HIGH(pair);
    key = 0;
    if (LOW(pair))
        goto F32_02;                    /* another menu */
F32_57:
    md.MenuFl = md.AutoMnu;
    return Run_00(command, (u16)key);
}
