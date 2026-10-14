/*
 * The box that asks for the drive of a panel, of Alt+F1 and Alt+F2.
 *
 * Translated from VCFUNC.INC.
 *
 * The letters of the drives that there are stand in a line, further
 * apart the fewer they are, with the cursor on the drive that the panel
 * has. The panel then gets the current directory of the drive that is
 * chosen, and is one of files, whatever it was.
 */
#include "vc.h"

/* F62_01 is declared in vc.h: see "The commands" there. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen. SavWin keeps what the screen
   buffer has where a window is to be, ResWin puts it back; MinBox draws a
   box with a shadow there. SftEGA moves a place down on a screen of more
   than 25 lines, by half of what it has more.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. PutMnu and Input
   are C, of vc.h.

   TstDrv is 1 if DOS has a drive of the letter in the low byte of
   `letter`. ErrRdy, which says that a drive is not ready and asks for
   another, is C, of vc.h. What the system is asked, it is asked by the
   functions of sys.h. */

/* A C routine of another file that vc.h does not have. */
u16 *MvsBw(u16 *screen, const u8 *text, u16 count,
           u16 color);                                  /* screen.c */

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define Box_C   16                      /* the colours of the box: its text, */
#define BoxBr_C 17                      /* which panel it is for, */
#define BoxCu_C 18                      /* and the cursor */

#define CHECK 0xFB                      /* the mouse pointer on what a
                                           click would choose */

static const u8 DrLe_S[14] = " Drive letter ";
static const u8 Choo_S[7] = "Choose ";
static const u8 Left_S[4] = "left";
static const u8 Rght_S[5] = "right";
static const u8 Driv_S[7] = " drive:";

/* The letters of the drives that the box offers. */
#define DRIVES (md.TmpBuf)

/* Ask for the drive of the panel `panel`, with the item `item` of the
   menu bar marked, 1 for the left panel and 5 for the right one, and put
   the panel on the drive that is chosen. Returns false if none was. */
bool F62_01(struct WCBdef *panel, i32 item)
{
    struct Answer asked;
    struct Mouse pointer;
    u16 *next;
    u16 *screen;
    u16 *first;                         /* DI: where the first letter is */
    u8 *cell;
    u16 begin, size;                    /* BX and DX: the box */
    i32 count;                          /* CL: how many drives */
    i32 cursor;                         /* CH: the one of the cursor */
    i32 apart;                          /* BP: cells between two letters */
    i32 wide;                           /* columns of the letters */
    i32 unit, active, i, at, index;
    u16 letter, place, color, c, key;
    u16 state, look, shape, mouse;
    bool right;

    HidCur();
    PutMnu((u16)item);
    count = 0;
    for (letter = 'A'; letter <= 'Z'; letter++) {
        if (!TstDrv(letter))
            continue;
        /* Of A: and B:, where they are one floppy drive, only the letter
           that the drive goes by now. */
        if (letter <= 'B' && md.DriveB == 0) {
            unit = letter - ('A' - 1);
            asked = DriveLetter((u16)unit);
            active = 0;
            if (!asked.failed)
                active = asked.value & 0xFF;
            if (active == 3 - unit)
                continue;
        }
        DRIVES[count++] = (u8)letter;
    }

    wide = count;
    apart = 0;
    if (count > 14)
        goto F62_06;
    apart++;
    wide += count;
    wide--;
    if (count > 9)
        goto F62_06;
    apart++;
    wide += count;
    wide++;
    if (count > 7)
        goto F62_06;
    apart++;
    wide += count;
    wide--;
F62_06:
    begin = BegBX(panel);
    size = 18;
    if ((begin & 0xFF) >= 1)
        size++;
    begin = ADD_LOW(begin, 21);
    if (wide > size)
        size = (u16)wide;
    size = ADD_LOW(size, 10 + 2);
    begin = ADD_LOW(begin, -((size + 1 & 0xFF) >> 1));
    begin = 5 << 8 | begin & 0xFF;
    size = (6 + 1) << 8 | size & 0xFF;
    begin = SftEGA(begin);
    SavWin(begin, size, md.WinBuf);
    color = (u16)(Color(Box_C) << 8);
    MinBox(begin, size, color);

    place = (u16)((u8)((begin >> 8) + 1) << 8 | (u8)(BegBX(panel) + 13));
    MvsBw(BasAdr(place), DrLe_S, 14, color);
    right = (place & 0xFF) >= 30;
    place = (place & 0xFF00) + 0x100 | (right ? 11 + 39 : 11);
    next = MvsBw(BasAdr(place), Choo_S, 7, color);
    c = (u16)(Color(BoxBr_C) << 8);
    if (right)
        next = MvsBw(next, Rght_S, 5, c);
    else
        next = MvsBw(next, Left_S, 4, c);
    MvsBw(next, Driv_S, 7, color);

    /* The letters. */
    place = (u16)((begin & 0xFF00 | BegBX(panel) & 0xFF) + 0x314
                  - (wide + 1 >> 1));
    if (apart >= 2)
        place++;
    first = BasAdr(place);
    screen = first;
    i = 0;
    do {
        *screen = color | DRIVES[i++];
        screen += apart + 1;
    } while (i != count);
    cursor = 0;
    for (i = 0; i < count; i++)
        if (DRIVES[i] == panel->WinPath[0]) {
            cursor = i;
            break;
        }
    md.HlpPage = 15;
    if (apart != 0)
        first--;

F62_15:                                 /* the cursor */
    MvsColr(BasAdr(begin) + 3 * 80 + 4, (u8)(size - 8 - 2), Color(Box_C));
    cell = (u8 *)first + (u16)(u8)(apart + 2 + apart) * cursor + 1;
    c = Color(BoxCu_C);
    cell[0] = (u8)c;
    if (apart != 0) {
        cell[2] = (u8)c;
        cell[4] = (u8)c;
    }
    Window(begin, size);

    /* The mouse, while a button of it is down: the cursor goes to the
       letter it is on. */
    pointer = GetMous(0);
    if (pointer.buttons == 0)
        goto F62_30;
    state = 0;
    look = 0;
F62_21:
    state = state & 0x80 | pointer.buttons & 0xFF;
    mouse = pointer.place;
    shape = 'x';
    if (state & 4)
        goto F62_27;
    shape = CHECK;
    if (state & 2)
        goto F62_27;
    at = first - md.ScrBuf;
    place = (u16)((at / 80 & 0xFF) << 8 | at % 80);
    index = 0;
    for (;;) {
        if (!InWind(mouse, place, apart != 0 ? 0x103 : 0x101))
            break;
        place += apart;
        place++;
        index++;
        if ((u8)index >= 27)
            goto F62_26;
    }
    if ((u8)index >= count)
        goto F62_26;
    if (state & 0x80)
        goto F62_25;
    state |= 0x80;
    if ((u8)index == cursor)
        goto F62_25;
    cursor = (u8)index;
    goto F62_15;
F62_25:
    shape = CHECK;
    if ((u8)index == cursor)
        goto F62_27;
F62_26:
    shape = 0;
F62_27:
    if (shape != look) {
        look = shape;
        TypMous(shape);
    }
    pointer = GetMous(mouse);
    if (pointer.buttons != 0)
        goto F62_21;
    TypMous(0);
    if (state & 4)
        goto F62_37;
    if (state & 2)
        goto F62_36;
    if (look != 0)
        goto F62_36;
    pointer = GetMous(mouse);
    if (InWind(pointer.place, begin, size - 0x102))
        goto F62_37;

F62_30:                                 /* a key */
    key = Input(1);
    if (key >> 8 != 0)
        switch (key & 0xFF) {
        case 0x08:
            goto F62_31;
        case 0x20:
            goto F62_34;
        case 0x0D:
            goto F62_36;
        case 0x1B:
            goto F62_37;
        case 0:
            switch (key >> 8) {
            case 0x4B:
                goto F62_31;
            case 0x4D:
                goto F62_34;
            case 0x47:
                goto F62_35;
            case 0x4F:
                goto F62_32;
            case 0x44:
                goto F62_37;
            }
        }
    if ((key & 0xFF) >= 0x80)
        goto F62_15;
    letter = UpCase(key) & 0xFF;        /* the letter of a drive */
    for (i = 0; i < count; i++)
        if (DRIVES[i] == letter)
            goto F62_40;
    goto F62_15;
F62_31:                                 /* Left */
    if (cursor == 0)
        goto F62_32;
    cursor--;
    goto F62_15;
F62_32:                                 /* End */
    cursor = (u8)(count - 1);
    goto F62_15;
F62_34:                                 /* Right */
    cursor++;
    if (cursor < count)
        goto F62_15;
F62_35:                                 /* Home */
    cursor = 0;
    goto F62_15;
F62_36:                                 /* Enter */
    letter = DRIVES[cursor];
    goto F62_40;
F62_37:                                 /* Esc */
    letter = 0;
F62_40:
    ResWin(begin, size, md.WinBuf);
    Window(begin, size);

F62_41:                                 /* the drive, and where it is */
    if (letter == 0)
        return false;
    HidCur();
    unit = (u8)(letter - ('A' - 1));
    DriveSetLetter((u16)unit);
    md.BlocEr = 1;
    asked = DirCurrent((u16)unit, panel->WinPath + 3);
    if (!asked.failed) {
        panel->WinPath[0] = (u8)letter;
        asked = DirChange(panel->WinPath);
    }
    md.BlocEr = 0;
    if (asked.failed) {
        letter = ErrRdy(letter) & 0xFF;
        goto F62_41;
    }
    /* The drive is DOS's too, unless the other panel is the current one
       and shows files. */
    if (panel != CurrentPanel && FilePan(InvWCB(panel)))
        goto F62_45;
    DriveSelect((u16)(unit - 1));
F62_45:
    panel->WinTyp = 0;
    panel->ChgFlg = 1;
    md.BlocEr = 1;
    panel->CurAdr = DSKBUF;
    panel->EndBuf = DSKBUF;
    return true;
}
