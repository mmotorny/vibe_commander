/*
 * The dialogs: a box with its lines of text, the fields of a dialog with
 * the keys and the mouse that each of them takes, and a line of buttons.
 *
 * Translated from VCSUB1.INC.
 *
 * A box and a dialog are described where the assembly has them, in its
 * code segment, and are named by their offsets there.
 *
 * A box is four numbers of colours, the number of its lines and of the
 * texts that are put into them, the place in the main data where what it
 * covers is kept, for each such text four bytes, and then a word for
 * each line: where its text is, or 0 for a line across the box. The first
 * line is the title, in the frame. A text to be put in is copied to
 * TmpErr, the first to its second byte and the second to its 102nd,
 * where the lines of the box have it as Var_S; its four bytes are where
 * it is, in the segment of whoever asks for the box, how it is to be
 * written and how long it may be.
 *
 * A dialog is a header, struct BOXHD of tables.h, and behind it its
 * fields, struct BOXMN, up to a byte 0. A field is a line to type into, a
 * group of which one is chosen, a box to tick or a line of buttons. While
 * the dialog is up, what is typed is in DosBuf, behind the command line:
 * three bytes and the text, and 100 bytes further on the same for a
 * second line. The three bytes are the column of the cursor and a byte
 * that is 0 until a key has been typed there: the first one typed
 * replaces the text.
 */
#include "tables.h"

/* A word of a box or a dialog that names something, as the offset that
   it is: tables.h has such a word as a pointer. */
#define WORD(names) ((u16)(names))

/* The kinds of a field. A line to type into has its text at mPoint in
   the segment of whoever asks for the dialog, its length in the low byte
   of mCtrl and INPUT_ flags in the high one. A group has, at mPoint in
   the main data, the line chosen and behind it the line of the cursor,
   and the number of its lines in the low byte of mCtrl. A box to tick has
   its value at mCtrl in the caller's segment, and while the dialog is up
   at mPoint in the main data. Buttons have the one chosen at mPoint in
   the main data, and at mCtrl in the assembly their widths and columns,
   two bytes each, up to a width of 0. */
#define FIELD_INPUT   1
#define FIELD_SELECT  2
#define FIELD_FLIP    3
#define FIELD_BUTTONS 4

#define INPUT_SECOND 0x100              /* the second of two lines */
#define INPUT_NORMAL 0x200              /* taken with NormStr at the end */

#define LINK_FIELD 0x3F                 /* the number of the field */
#define LINK_LAST  0x40                 /* a group: on its last line, */
#define LINK_SET   0x80                 /* or its first if only this is set */

/* The flags of a dialog. */
#define BOX_CLEAR 0x01                  /* empty the keyboard buffer first */
#define BOX_BEEP  0x02
#define BOX_ERROR 0x04                  /* lower by SftErr */
#define BOX_TREE  0x08                  /* F10 and Alt-F10 are answers */
#define BOX_THERE 0x10                  /* the box is on the screen already */

#define LenCmd 124                      /* Length of command line */

/* Where a line that is typed into is kept: its three bytes. */
#define INPUT(control) \
    (md.DosBuf + LenCmd + 10 + ((control) & INPUT_SECOND ? 100 : 0))

#define KEY_MOUSE 0xFC                  /* the high byte of a "key" that
                                           says the mouse did something */
#define KEY_ENTER 0x1C0D

#define BULLET 7                        /* marks the line chosen of a group */
#define TICK   0xFB                     /* the mouse pointer on an answer */
#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on the
                                           left and on the right */

/* What is at an offset in a segment. */
#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))
#define FIELD(offset) ((const struct BOXMN __far *)AT(code, offset))

/* A byte and a word of the main data, by their offsets. */
#define MD(offset)  (*(u8 *)(offset))
#define MDW(offset) (*(u16 *)(offset))

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte. ChgCur gives the cursor the shape it has in
   the program and returns the one it had, which CurShp gives back. ClrKbd
   empties the keyboard buffer if the setup has it so. SavWin keeps what
   the screen buffer has where a window is to be, ResWin puts it back;
   MinBox draws a box with a shadow there, growing if windows are to zoom,
   in the colour in the high byte of `color`.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window.

   AsmSeg returns the segment of the program's constants, where the boxes
   and the dialogs are. Naming one of them would give it too, as a number
   that is set when the program is loaded; the test of the scenarios
   knows code by its bytes in the file and would not see a function run that
   begins with such a number.

   The routines that a dialog may name for Enter and F10, the SUB_ of
   tables.h, get the answer as Dialog has it, the key. F38_10 returns the
   answer to go by. M30_60 does too, and in the high word something that
   is not 0 to go on with the dialog. F35_61 gets the place of the line
   that the tree is for as well, and is not 0 to go on with the dialog;
   the answer stays what it was. */
u16 F38_10(u16 answer);                                 /* dirfunc.c */
u32 M30_60(u16 answer);
u16 F35_61(u16 answer, u16 place);                      /* copy.c */

/* C routines of other files that vc.h does not have. */
u16 LnStr(const struct LineArgs STACK *args);           /* line.c */
void NormStr(u8 __far *text);                           /* string.c */

/* Copy a text that a box puts into its lines to where the lines have it,
   and write it as the box asks: `entry` is its four bytes in the box. */
static void DiBx30(const u8 __far *entry, u8 *to, u16 segment)
{
    const u8 __far *from = AT(segment, *(const u16 __far *)entry);
    u8 *text = to;
    u16 count = LenPath + 13;
    u16 mode;

    do
        *text++ = *from++;
    while (--count);
    mode = entry[2];
    if (mode & 4)
        NormStr(to);
    if (mode & 2) {
        PathLin(to);
        if (mode & 1)
            LetLin(to);
    }
    ShrtLin(to, (u16)(signed char)entry[3]);
}

/* Put the box at `box` into the screen buffer, with the middle of its
   top at `place`, the row in the high byte, or lower on a screen of more
   than 25 lines. `include` is the text that ^I stands for in its lines,
   an offset in the assembly too, and `segment` where the texts are that
   it puts in. `where` gets where the box is and where what it covers is
   kept. A box without a line has 65536. */
void DialBox(u16 place, u16 box, u16 include, u16 segment,
             struct Box STACK *where)
{
    u16 code = AsmSeg();
    const u8 __far *head = AT(code, box);
    struct LineArgs args;
    u16 texts, lines, line, count, text, width, length, size, begin, row;
    u16 *screen;

    HidCur();
    if (md.Lines > 25)                  /* SftEGA */
        place += (u16)((u8)(md.Lines - 25) >> 1) << 8;
    texts = head[5] & 3;
    if (texts != 0) {
        DiBx30(head + 8, md.TmpErr + 1, segment);
        if (texts != 1)
            DiBx30(head + 12, md.TmpErr + 101, segment);
    }

    /* As wide as its longest line. */
    lines = box + 8 + (texts << 2);
    args.value = 0;
    args.screen = 0;
    args.colors = head;
    args.include = AT(code, include);
    width = 0;
    line = lines;
    count = head[4];
    do {
        text = WORD_AT(code, line);
        line += 2;
        if (text != 0) {
            args.line = AT(code, text);
            length = LnStr((struct LineArgs STACK *)&args);
            if (length > width)
                width = length;
        }
    } while (--count);
    size = ((u16)head[4] << 8 | width & 0xFF) + 0x40C;
    begin = ADD_LOW(place, -((((size & 0xFF) - 1) & 0xFF) >> 1));
    where->begin = begin;
    where->size = size;
    where->saved = WORD_AT(code, box + 6);
    SavWin(begin, size, where->saved);
    MinBox(begin, size, Color(head[0]) << 8);

    row = place;
    line = lines;
    count = head[4];
    do {
        row += 0x100;
        text = WORD_AT(code, line);
        if (text != 0) {
            args.value = row;
            args.line = AT(code, text);
            CntLine((struct LineArgs STACK *)&args);
        } else {
            screen = BasAdr(row) - ((((size & 0xFF) - 1) & 0xFFFE) >> 1);
            BoxLine(screen, Color(head[0]) << 8 | LINE, LEFT_RIGHT, 3,
                    (size & 0xFF) - 10 & 0xFF);
        }
        line += 2;
    } while (--count);
}

/* Show what a line that is typed into holds: `text`, in the main data, in
   `length` cells at `place`, in the colour whose number is in the high
   byte of `color`. A length of 0 is one of 65536, for a text that long. */
void InSt10(u16 place, u16 text, u16 length, u16 color)
{
    u16 *screen = BasAdr(place);
    u16 cell = Color(color >> 8) << 8;
    const u8 *from = (const u8 *)text;
    u16 count = length;
    u16 c;

    for (;;) {
        c = *from++;
        if (c == 0)
            break;
        *screen++ = cell | c;
        if (--count == 0)
            break;
    }
    while (count != 0) {
        *screen++ = cell | ' ';
        count--;
    }
    Window(place, 0x100 | length & 0xFF);
}

/* How many cells a field takes at `place`: up to two spaces, one after
   the other, and 0 for 256 without. */
static u16 Slct20(u16 place)
{
    const u8 *cell = (const u8 *)BasAdr(place);
    u16 width = 0;

    do {
        if (cell[0] == ' ' && cell[2] == ' ')
            break;
        cell += 2;
        width = width + 1 & 0xFF;
    } while (width != 0);
    return width;
}

/* True if the button `item` of the buttons at `table` is the last. */
static u16 MnMn50(u16 table, u16 item, u16 code)
{
    return *AT(code, table + ((item & 0xFF) << 1) + 2) == 0;
}

/* The size of the line that the buttons at `table` take: as far as the
   last of them goes, and one row. */
static u16 MnMn60(u16 table, u16 code)
{
    const u8 __far *button = AT(code, table);

    do
        button += 2;
    while (button[0] != 0);
    return 0x100 | (u8)(button[-2] + button[-1]);
}

/* Give the button `item` of the buttons at `table`, which begin at
   `screen`, the colour with the number `number`. Returns that colour, in
   both bytes. */
static u16 MnMn41(u16 number, u16 item, u16 table, u16 *screen, u16 code)
{
    u16 color = Color(number);
    const u8 __far *button;
    u16 count = item & 0xFF;

    count += count;
    button = AT(code, table + count);
    count = count & 0xFF00 | button[1];
    count += count;
    screen = (u16 *)((u8 *)screen + count);
    count = count & 0xFF00 | button[0];
    MvsColr(screen, count, color);
    return color << 8 | color;
}

/* Show the buttons at `table`, which take `size` at `place`, in the
   colour whose number is in the high byte of `colors`, and the button
   `item` in that of the low byte. Returns the second colour, in both
   bytes. */
u16 MnMn40(u16 colors, u16 place, u16 item, u16 size, u16 table)
{
    u16 *screen = BasAdr(place);
    u16 both;

    MvsColr(screen, size & 0xFF, Color(colors >> 8));
    both = MnMn41(colors & 0xFF, item, table, screen, AsmSeg());
    Window(place, size);
    return both;
}

/* Have the buttons at `table`, which begin at `place`, take a key.
   `*state` is the button chosen, and in its high byte the key bar, with
   80h for buttons that are a field of a dialog: there Home and End go to
   the first and the last field. Returns the key, which is Enter for the
   letter of a button and for a click on the button chosen, and whether
   the buttons have dealt with it: the carry flag of the assembly. */
struct ButtonKey {
    u16 key;
    u16 taken;                          /* 1 if the buttons have dealt with
                                           the key */
};
static struct ButtonKey MnMn11(u16 key, u16 place, u16 STACK *state,
                               u16 table, u16 code)
{
    struct ButtonKey pressed;
    const u8 __far *button;
    u16 chosen = *state;
    u16 taken = 1;
    u16 item, low, count, buttons, mouse, column, look, now, begin, size;
    const u16 *screen;
    struct Mouse pointer;

    if (key >> 8 == KEY_MOUSE) {
        for (;;) {                      /* MnMn11 */
            pointer = GetMous(0);
            buttons = pointer.buttons;
            mouse = pointer.place;
            key = KEY_MOUSE << 8 | buttons;
            if (buttons == 0)
                goto MnMn16;
            if (buttons != 1)
                goto MnMn23;
            if (InWind(mouse, place, MnMn60(table, code)))
                goto MnMn23;
            column = mouse - place & 0xFF;
            for (item = table; ; item += 2) {
                button = AT(code, item);
                if (button[0] == 0 || column < button[1])
                    break;
                if (column < (u8)(button[1] + button[0]))
                    goto MnMn13;
            }
        }
MnMn13:
        key = item - table >> 1;
        if ((key & 0xFF) != (chosen & 0xFF)) {
            chosen = chosen & 0xFF00 | key & 0xFF;
            goto MnMn16;
        }
        /* The button chosen: pressed, if the mouse is let go on it. */
        begin = ADD_LOW(place, button[1]);
        size = 0x100 | button[0];
        look = 0;
        for (;;) {                      /* MnMn14 */
            pointer = GetMous(0);
            if (pointer.buttons == 0)
                break;
            now = InWind(pointer.place, begin, size) ? 0 : TICK;
            if (now == look)
                continue;
            look = now;
            TypMous(now);
        }
        TypMous(0);
        key = 0;
        if (look == 0)
            goto MnMn16;
        key = KEY_ENTER;
        goto MnMn23;
    }

    if (key >> 8 != 0) {                /* Case takes none without a scan code */
        low = key & 0xFF;
        if (key == 0x4B00 || low == 8) {            /* Left, Backspace */
            if ((chosen & 0xFF) != 0)
                chosen--;
            else
                while (!MnMn50(table, chosen, code))
                    chosen++;
            goto MnMn16;
        }
        if (key == 0x4D00 || low == ' ') {          /* Right, Space */
            if (MnMn50(table, chosen, code))
                chosen &= 0xFF00;
            else
                chosen++;
            goto MnMn16;
        }
        if (key == 0x4700) {                        /* Home */
            if (chosen & 0x8000) {
                key = 0x4900;
                chosen &= 0xFF00;
            }
            goto MnMn23;
        }
        if (key == 0x4F00) {                        /* End */
            if (chosen & 0x8000) {
                key = 0x5100;
                while (!MnMn50(table, chosen, code))
                    chosen++;
            }
            goto MnMn23;
        }
    }
    key = UpCase(key);
    low = key & 0xFF;
    if (low < 'A' || low > 'Z')
        goto MnMn23;
    /* The button that has this capital in it. */
    for (item = table; ; item += 2) {
        button = AT(code, item);
        if (button[0] == 0)
            goto MnMn16;
        screen = BasAdr(ADD_LOW(place, button[1]));
        count = button[0];
        do
            if ((u8)*screen++ == low)
                goto MnMn22;
        while (--count);
    }
MnMn22:
    chosen = item - table >> 1;
    key = KEY_ENTER;
MnMn23:
    taken = 0;
MnMn16:
    *state = chosen;
    pressed.key = key;
    pressed.taken = taken;
    return pressed;
}

/* MnMn10, for a menu that is not MinMnu's: see MnMn11. `item` is what
   that has in `*state`. Returns the key, the button chosen, and whether
   the buttons have dealt with the key. `size` is not read: the assembly
   did not read it either. */
struct Taken MnMn10(u16 key, u16 place, u16 item, u16 size, u16 table)
{
    struct Taken taken;
    struct ButtonKey pressed;

    (void)size;
    pressed = MnMn11(key, place, (u16 STACK *)&item, table, AsmSeg());
    taken.key = pressed.key;
    taken.button = (u8)item;
    taken.taken = (u8)pressed.taken;
    return taken;
}

/* A line of buttons: have the buttons at `control`, which take `size` at
   `begin`, take keys until one is for the caller. `colors` is where the
   assembly has three numbers of colours: of the line, of the button
   chosen while it takes keys, and of the button chosen when it is left.
   `keys` is the key bar, with 80h for a field of a dialog, and `item` the
   button chosen. Returns the key, and the button chosen, with `keys` in
   the high byte of that, or 0 there for the letter of a button. */
struct Buttons MinMnu(u16 control, u16 begin, u16 size, u16 colors, u16 keys,
                      u16 item)
{
    struct Buttons buttons;
    u16 code = AsmSeg();
    const u8 __far *color = AT(code, colors);
    u16 state = (keys & 0xFF) << 8 | item & 0xFF;
    u16 key;
    struct ButtonKey pressed;

    HidCur();
    for (;;) {
        MnMn40((u16)color[0] << 8 | color[1], begin, state, size, control);
        key = Input(state >> 8 & 0x7F);
        pressed = MnMn11(key, begin, (u16 STACK *)&state, control, code);
        key = pressed.key;
        if (pressed.taken != 0)
            continue;
        if (key >> 8 != KEY_MOUSE)
            MnMn40((u16)color[0] << 8 | color[2], begin, state, size,
                   control);
        break;
    }
    buttons.key = key;
    buttons.chosen = state;
    return buttons;
}

/* What a field of a dialog returns when a key is not for it: the key,
   and what the field holds then, for Dialog to keep. A line that is
   typed into, InpStr; a group of lines, Select; a box to tick, FlipFl.
   The buttons have theirs in vc.h, with MinMnu. */
struct Typed {
    u16 key;
    u16 cursor;                         /* the column of the cursor */
};
struct Chosen {
    u16 key;
    u16 lines;                          /* the line chosen, and in the high
                                           byte the line of the cursor */
};
struct Ticked {
    u16 key;
    u16 value;                          /* not 0 if the box is ticked */
};

/* A line to type into: `text`, in the main data, of `length` characters
   at the most, at `place`, in the colour with the number `color`, the
   cursor in the column `cursor`. Takes keys until one is not for the
   line. Returns that key, and the column of the cursor. */
static struct Typed InpStr(u16 place, u8 *text, u16 cursor, u16 length,
                           u16 color, u16 bar)
{
    struct Typed typed;
    u16 key, low, buttons, mouse, column, end, count;
    const u8 *from;
    struct Mouse pointer;

    for (;;) {                          /* InSt01 */
        InSt10(place, (u16)text, length, color << 8);
        Cursor(place + cursor);
        key = Input(bar);
        if (key >> 8 == KEY_MOUSE) {
            for (;;) {                  /* InSt05 */
                pointer = GetMous(0);
                buttons = pointer.buttons;
                mouse = pointer.place;
                key = KEY_MOUSE << 8 | buttons;
                if (buttons == 0)
                    break;
                ClrKbd();
                if (buttons != 1)
                    goto InSt08;
                if (InWind(mouse, place, 0x100 | length & 0xFF))
                    goto InSt08;
                text[-1] = 1;
                column = mouse - place;
                /* No further than the text goes, or one short of that
                   for a text as long as the line. */
                from = text;
                for (count = length; count != 0; count--)
                    if (*from++ == 0)
                        break;
                end = (u16)(from - text) - 1;
                if (column > end)
                    column = end;
                if (column == cursor)
                    continue;
                cursor = column;
                break;
            }
            continue;
        }
        /* Case takes no key without a scan code. */
        if (md.Quote == 0 && key >> 8 != 0) {
            low = key & 0xFF;
            if (low == 9 || low == 0x0D || low == 0x1B)
                goto InSt08;
            if (low == 0x11)            /* Ctrl-Q */
                goto InSt04;
            switch (key) {
            case 0x0F00:                /* Shift-Tab */
            case 0x4800:                /* Up */
            case 0x5000:                /* Down */
            case 0x4900:                /* PgUp */
            case 0x5100:                /* PgDn */
            case 0x4400:                /* F10 */
            case 0x7100:                /* Alt-F10 */
                goto InSt08;
            case 0x4B00:                /* Left */
                if (cursor == 0)
                    goto InSt08;
            }
        }
        if (text[-1] == 0) {            /* InSt02 */
            /* The first key: one that types replaces the text. */
            text[-1] = 1;
            low = key & 0xFF;
            if (md.Quote != 0 || key >> 8 == 0
                || !(low < ' ' || low == 0x7F))
                text[0] = 0;
        }
InSt04:
        cursor = EdLin(key, text, cursor, length).cursor;
    }
InSt08:
    typed.key = key;
    typed.cursor = cursor;
    return typed;
}

/* A group of `total` lines at `place`, of which one is chosen: `lines`
   has the line chosen and in its high byte the line of the cursor. Takes
   keys until one is not for the group, and returns it, and the two lines
   as they are then. */
static struct Chosen Select(u16 place, u16 bar, u16 total, u16 lines)
{
    struct Chosen answer;
    u16 chosen = lines & 0xFF;
    u16 cursor = lines >> 8;
    u16 key = 0;
    u16 low, buttons, mouse, row;
    struct Mouse pointer;

Slct01:
    Cursor(place + (cursor << 8));
    if ((key & 0xFF) != 0)              /* until the mouse is let go */
        while (GetMous(0).buttons != 0)
            ;
Slct03:
    key = Input(bar);
    if (key >> 8 == KEY_MOUSE) {
        for (;;) {                      /* Slct04 */
            pointer = GetMous(0);
            buttons = pointer.buttons;
            mouse = pointer.place;
            key = KEY_MOUSE << 8 | buttons;
            if (buttons == 0)
                goto Slct03;
            ClrKbd();
            if (buttons != 1)
                goto Slct13;
            if (InWind(mouse, place - 1, total << 8 | Slct20(place - 1)))
                goto Slct13;
            row = (mouse >> 8) - (place >> 8) & 0xFF;
            key = row << 8 | mouse & 0xFF;
            if (row == cursor)
                continue;
            cursor = row;
            goto Slct07;
        }
    }
    if (key >> 8 == 0)                  /* Case takes none without a scan code */
        goto Slct03;
    low = key & 0xFF;
    if (low == ' ') {
        if (cursor == chosen) {         /* on the one chosen: the next */
            cursor = cursor + 1 & 0xFF;
            if (cursor >= total)
                cursor = 0;
        }
        goto Slct07;
    }
    if (low == 9 || low == 0x1B)
        goto Slct13;
    if (low == 0x0D) {
        if (cursor != chosen)
            goto Slct07;
        goto Slct13;
    }
    switch (key) {
    case 0x0F00:                        /* Shift-Tab */
    case 0x4B00:                        /* Left */
    case 0x4D00:                        /* Right */
    case 0x4400:                        /* F10 */
        goto Slct13;
    case 0x4800:                        /* Up */
        if (cursor == 0)
            goto Slct13;
        cursor--;
        goto Slct01;
    case 0x5000:                        /* Down */
        cursor = cursor + 1 & 0xFF;
        if (cursor < total)
            goto Slct01;
        /* past the last line */
    case 0x4F00:                        /* End */
    case 0x5100:                        /* PgDn */
        cursor = total - 1 & 0xFF;
        goto Slct13;
    case 0x4700:                        /* Home */
    case 0x4900:                        /* PgUp */
        cursor = 0;
        goto Slct13;
    }
    goto Slct03;

Slct07:                                 /* choose the line of the cursor */
    *(u8 *)BasAdr(place + (chosen << 8)) = ' ';
    Window(place + (chosen << 8), 0x101);
    *(u8 *)BasAdr(place + (cursor << 8)) = BULLET;
    Window(place + (cursor << 8), 0x101);
    chosen = cursor;
    key = cursor << 8 | key & 0xFF;
    goto Slct01;

Slct13:
    answer.key = key;
    answer.lines = cursor << 8 | chosen;
    return answer;
}

/* A box to tick at `place`, which is ticked if `value` is not 0. Space
   and a click change it; any other key is returned, and the value as it
   is then. */
static struct Ticked FlipFl(u16 place, u16 bar, u16 value)
{
    struct Ticked ticked;
    u16 key, buttons;
    u8 *cell;
    struct Mouse pointer;

    value &= 0xFF;
    Cursor(place);
    for (;;) {
        key = Input(bar);
        if (key != 0x3920) {
            if (key >> 8 != KEY_MOUSE)
                break;
            pointer = GetMous(0);
            buttons = pointer.buttons;
            key = KEY_MOUSE << 8 | buttons;
            if (buttons == 0)
                continue;
            ClrKbd();
            if (buttons != 1)
                break;
            if (InWind(pointer.place, place - 1, 0x100 | Slct20(place - 1)))
                break;
        }
        cell = (u8 *)BasAdr(place);
        *cell = ' ';
        if (value == 0) {
            *cell = 'x';
            value = 1;
        } else
            value = 0;
        Window(place, 0x101);
        if ((key & 0xFF) != ' ')        /* until the mouse is let go */
            while (GetMous(0).buttons != 0)
                ;
    }
    ticked.key = key;
    ticked.value = value;
    return ticked;
}

/* Where the buttons of a dialog begin: `place` is where the box begins
   plus what the field has for them, of which the row counts and the
   column is counted from the left half of the screen or from the right,
   whichever the box begins in, at `column`. */
static u16 Dial18(u16 place, u16 column)
{
    place = ADD_LOW(place, -column);
    if (column >= 40)
        place = ADD_LOW(place, 40);
    return place;
}

/* The last field of a dialog behind the field `field`, which may not be
   the last itself. */
static u16 Dial85(u16 field, u16 code)
{
    do
        field += sizeof(struct BOXMN);
    while (FIELD(field)->mType != 0);
    return field - sizeof(struct BOXMN);
}

/* A dialog: show the box of the dialog at `box`, have its fields take
   keys and the mouse, and take the box off the screen again. `segment`
   is where the caller has what the fields set: the lines that are typed
   and the values of the boxes to tick, which get there when the dialog
   is answered with Enter on the first button, or on a field that is no
   button. `include` is the text that ^I stands for in a line of the box.
   For a dialog whose box is on the screen already, `begin`, `include` and
   `saved` are where that is, its size and where what it covers is kept,
   as DialBox has given them.

   Returns in the low byte 0 for that answer, the number of another
   button, 1 for F10 and Alt-F10 where they are answers, with 1 and 0 in
   the high byte, and FFh for Esc. A dialog without a field takes no key
   and does not end. */
u16 Dialog(u16 box, u16 include, u16 begin, u16 saved, u16 segment)
{
    u16 code = AsmSeg();
    const struct BOXHD __far *head = (const struct BOXHD __far *)AT(code, box);
    const struct BOXMN __far *field;
    const u8 __far *colors;
    const u8 __far *from;
    u8 __far *to;
    u8 *text;
    u16 *screen;
    struct Box where;
    u32 pair;
    struct Typed typed;
    struct Chosen chosen;
    struct Ticked ticked;
    struct Buttons menu;
    struct Mouse pointer;
    u16 first = box + sizeof(struct BOXHD);
    u16 size = include;
    u16 shape, key, item, cursor, type, place, control, count, link, value;
    u16 look, now, buttons, mouse, window;

    md.Quote = 0;
    shape = ChgCur();
    key = head->bMenu;
    if (!(head->bFlags & BOX_THERE)) {
        begin = head->BoxBase;
        /* Of a panel: in the half of the screen that the panel has. */
        if (key == 1 && segment != md.WCB1seg) {
            key = 5;
            begin = ADD_LOW(begin, 40);
        }
    }
    if (key != 0xFF)
        PutMnu(key);
    PutKey(head->bKeyBar);
    if (head->bFlags & BOX_ERROR)
        begin += (u16)md.SftErr << 8;
    if (!(head->bFlags & BOX_THERE)) {
        DialBox(begin, WORD(head->pDiBox), include, segment,
                (struct Box STACK *)&where);
        begin = where.begin;
        size = where.size;
        saved = where.saved;
    }
    colors = AT(code, WORD(head->pDiBox));

    /* The fields as they are when the dialog opens. */
    for (item = first; ; item += sizeof(struct BOXMN)) {
        field = FIELD(item);
        type = field->mType;
        place = begin + field->mBase;
        screen = BasAdr(place);
        if (type == FIELD_INPUT) {
            control = WORD(field->mCtrl);
            text = INPUT(control);
            text[0] = 0;
            text[1] = 0;
            text[2] = 0;
            text += 3;
            from = AT(segment, field->mPoint);
            for (count = control & 0xFF; count != 0; count--)
                *text++ = *from++;
            *text = 0;
            InSt10(place, (u16)(INPUT(control) + 3), control & 0xFF,
                   (u16)colors[1] << 8);
        } else if (type == FIELD_SELECT) {
            *((u8 *)screen + MD(field->mPoint) * (2 * 80)) = BULLET;
        } else if (type == FIELD_FLIP) {
            value = *AT(segment, WORD(field->mCtrl));
            MD(field->mPoint) = (u8)value;
            if (value != 0)
                *(u8 *)screen = 'x';
        } else if (type == FIELD_BUTTONS) {
            screen = BasAdr(Dial18(place, begin & 0xFF));
            MD(field->mPoint) = 0;
            MnMn41(colors[2], 0, WORD(field->mCtrl), screen, code);
        } else
            break;
    }
    Window(begin, size);
    if (head->bHelp != 0)
        md.HlpPage = head->bHelp;
    if (head->bFlags & BOX_CLEAR)
        ClrKbd();
    if (head->bFlags & BOX_BEEP)
        Beep();

    cursor = first;
Dial31:                                 /* the field of the cursor has the keys */
    field = FIELD(cursor);
    type = field->mType;
    place = begin + field->mBase;
    if (type == FIELD_INPUT) {
        control = WORD(field->mCtrl);
        text = INPUT(control);
        typed = InpStr(place, text + 3, *(u16 *)text, control & 0xFF,
                       colors[1], head->bKeyBar);
        *(u16 *)text = typed.cursor;
        key = typed.key;
    } else if (type == FIELD_SELECT) {
        chosen = Select(place, head->bKeyBar, WORD(field->mCtrl) & 0xFF,
                        MDW(field->mPoint));
        MDW(field->mPoint) = chosen.lines;
        key = chosen.key;
    } else if (type == FIELD_FLIP) {
        ticked = FlipFl(place, head->bKeyBar, MD(field->mPoint));
        MD(field->mPoint) = (u8)ticked.value;
        key = ticked.key;
    } else if (type == FIELD_BUTTONS) {
        menu = MinMnu(WORD(field->mCtrl), Dial18(place, begin & 0xFF),
                      MnMn60(WORD(field->mCtrl), code), WORD(head->pDiBox),
                      head->bKeyBar | 0x80, MD(field->mPoint));
        MD(field->mPoint) = (u8)menu.chosen;
        key = menu.key;
    } else
        key = (begin & 0xFF) << 8 | type;
    if (key >> 8 == KEY_MOUSE)
        goto Dial40;
    if (key >> 8 == 0)                  /* Case takes none without a scan code */
        goto Dial31;
    switch (key & 0xFF) {
    case 9:                             /* Tab */
        cursor += sizeof(struct BOXMN);
        if (FIELD(cursor)->mType == 0)
            cursor = first;
        goto Dial31;
    case 0x0D:
        goto Dial70;
    case 0x1B:
        goto Dial81;
    }
    switch (key) {
    case 0x0F00:                        /* Shift-Tab */
        if ((u16)(cursor - sizeof(struct BOXMN)) >= first)
            cursor -= sizeof(struct BOXMN);
        else
            cursor = Dial85(first, code);
        goto Dial31;
    case 0x4800:
        link = field->mUp;
        break;
    case 0x5000:
        link = field->mDown;
        break;
    case 0x4B00:
        link = field->mLeft;
        break;
    case 0x4D00:
        link = field->mRight;
        break;
    case 0x4700:                        /* Home */
    case 0x4900:                        /* PgUp */
        link = LINK_SET;
        break;
    case 0x4F00:                        /* End */
        cursor = Dial85(cursor, code);
        field = FIELD(cursor);
        if (field->mType == FIELD_BUTTONS)
            MD(field->mPoint) = 0;
        /* and as PgDn */
    case 0x5100:                        /* PgDn */
        cursor = Dial85(cursor, code);
        goto Dial31;
    case 0x4400:                        /* F10 */
        if (!(head->bFlags & BOX_TREE))
            goto Dial81;
        key = 0x101;
        goto Dial71;
    case 0x7100:                        /* Alt-F10 */
        if (!(head->bFlags & BOX_TREE))
            goto Dial31;
        key = 1;
        goto Dial71;
    default:
        goto Dial31;
    }
    /* To the field that the field of the cursor names for the key. */
    cursor = first + (link & LINK_FIELD) * sizeof(struct BOXMN);
    field = FIELD(cursor);
    if (field->mType == FIELD_SELECT && (link & LINK_SET)) {
        value = 0;
        if (link & LINK_LAST)
            value = WORD(field->mCtrl) - 1;
        MD(field->mPoint + 1) = (u8)value;
    }
    goto Dial31;

Dial40:                                 /* the mouse */
    look = 0;
    for (;;) {                          /* Dial41 */
        pointer = GetMous(0);
        buttons = pointer.buttons;
        mouse = pointer.place;
        if (buttons == 0)
            break;
        if (buttons != 1) {
            now = 'x';
            if (buttons == 2)
                now = TICK;
        } else if (InWind(mouse, begin, size - 0x102)) {
            now = 0xFF;
        } else {
            /* The field that the mouse is on gets the keys. */
            for (item = first; ; item += sizeof(struct BOXMN)) {
                field = FIELD(item);
                type = field->mType;
                if (type == 0)
                    break;
                place = begin + field->mBase;
                window = WORD(field->mCtrl);
                if (type == FIELD_INPUT) {
                    window = 0x100 | window & 0xFF;
                } else if (type <= FIELD_FLIP) {
                    window = (window & 0xFF) << 8 | Slct20(place);
                    if (type != FIELD_SELECT)
                        window = 0x100 | window & 0xFF;
                } else {
                    place = Dial18(place, begin & 0xFF);
                    window = MnMn60(window, code);
                }
                if (!InWind(mouse, place, window))
                    goto Dial49;
            }
            now = 0;
        }
        if (now == look)
            continue;
        look = now;
        TypMous(now == 0xFF ? 0 : now);
    }
    key = 0;
    TypMous(0);
    if (look == TICK)
        goto Dial70;
    if (look == 'x' || look == 0xFF)
        goto Dial81;
    goto Dial31;

Dial49:
    /* Buttons that the cursor leaves have none chosen any more. */
    field = FIELD(cursor);
    if (field->mType == FIELD_BUTTONS)
        MnMn40((u16)colors[0] << 8 | colors[2],
               Dial18(begin + field->mBase, begin & 0xFF),
               MD(field->mPoint), MnMn60(WORD(field->mCtrl), code),
               WORD(field->mCtrl));
    cursor = item;
    goto Dial31;

Dial70:                                 /* Enter */
    field = FIELD(cursor);
    key = 0;
    if (field->mType == FIELD_BUTTONS)
        key = MD(field->mPoint);
Dial71:
    /* The routine that the dialog names. The assembly calls it by its
       address, and each hands back the answer in AX and the carry flag
       to go on with the dialog, in its own way. */
    switch (head->SubFunc) {
    case SUB_F38_10:
        key = F38_10(key);
        break;
    case SUB_M30_60:
        pair = M30_60(key);
        key = LOW(pair);
        if (HIGH(pair) != 0)
            goto Dial31;
        break;
    case SUB_F35_60:
        if (F35_61(key, begin + 0x200 + 0x305) != 0)
            goto Dial31;
        break;
    case SUB_F35_61:
        if (F35_61(key, begin + 0x305) != 0)
            goto Dial31;
        break;
    }
    if ((key & 0xFF) != 0)
        goto Dial82;
    /* What the fields hold goes to where the caller has it. */
    for (item = first; ; item += sizeof(struct BOXMN)) {
        field = FIELD(item);
        type = field->mType;
        if (type == 0)
            break;
        if (type == FIELD_INPUT) {
            control = WORD(field->mCtrl);
            text = INPUT(control) + 3;
            if (control & INPUT_NORMAL)
                NormStr(text);
            to = AT(segment, field->mPoint);
            for (count = control & 0xFF; count != 0; count--)
                *to++ = *text++;
            *to = 0;
        } else if (type == FIELD_FLIP)
            *AT(segment, WORD(field->mCtrl)) = MD(field->mPoint);
    }
    goto Dial82;

Dial81:                                 /* Esc */
    key |= 0xFF;
Dial82:
    HidCur();
    if (!(head->bFlags & BOX_THERE)) {
        ResWin(begin, size, saved);
        Window(begin, size);
    }
    CurShp(shape);
    return key;
}
