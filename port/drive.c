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

u16 F62_01(u16 panel, u16 item);

/* How the assembly calls it (see the stub in port/asm/VCFUNC.INC). */
#pragma aux F62_01 "*_c" __far parm [ax] [dx] value [ax] modify exact [ax]

/* Routines of the rest of the program that are still assembly, by their
   entries for C. HidCur puts the cursor below the screen. PutMnu draws
   the menu bar, if that is always shown, with an item of it marked.
   SavWin keeps what the screen buffer has where a window is to be, ResWin
   puts it back; MinBox draws a box with a shadow there. SftEGA moves a
   place down on a screen of more than 25 lines, by half of what it has
   more.

   GetMous returns the buttons of the mouse that are down, and in the high
   word where the mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. Input waits for a
   key, with the key bar `bar` on the screen, and shows the help for F1.

   TstDrv is 1 if DOS has a drive of the letter in the low byte of
   `letter`. ErrRdy says that a drive is not ready and asks for another:
   it returns the letter in the low byte, or 0 there for none. Int21d
   calls DOS as Intr21 does, with the registers of `regs` and their
   answer there, but not through Intr21, where the assembly called DOS
   itself. */
void HidCur(void);
void PutMnu(u16 item);
void SavWin(u16 begin, u16 size, u16 buffer);
void ResWin(u16 begin, u16 size, u16 buffer);
void MinBox(u16 begin, u16 size, u16 color);
u16 SftEGA(u16 place);
u32 GetMous(u16 place);
void TypMous(u16 look);
u16 InWind(u16 place, u16 begin, u16 size);
u16 Input(u16 bar);
u16 TstDrv(u16 letter);
u16 ErrRdy(u16 letter);
u16 Int21d(struct DosRegs STACK *regs);
#pragma aux HidCur  "*_a" __far modify exact [es]
#pragma aux PutMnu  "*_a" __far parm [ax] modify exact [es]
#pragma aux SavWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux MinBox  "*_a" __far parm [bx] [dx] [ax] modify exact [es]
#pragma aux SftEGA  "*_a" __far parm [bx] value [bx] modify exact [bx es]
#pragma aux GetMous "*_a" __far parm [cx] value [dx ax] modify exact [ax cx dx es]
#pragma aux TypMous "*_a" __far parm [ax] modify exact [es]
#pragma aux InWind  "*_a" __far parm [cx] [bx] [dx] value [ax] modify exact [ax es]
#pragma aux Input   "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux TstDrv  "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux ErrRdy  "*_a" __far parm [ax] value [ax] modify exact [ax es]
#pragma aux Int21d  "*_a" __far parm [bx] value [ax] modify exact [ax bx cx dx si di es]

/* A C routine of another file that vc.h does not have. */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
#pragma aux MvsBw "*_c" __far parm [es di] [dx si] [cx] [ax] value [es di] modify exact [di es]

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define Box_C   16                      /* the colours of the box: its text, */
#define BoxBr_C 17                      /* which panel it is for, */
#define BoxCu_C 18                      /* and the cursor */

#define CHECK 0xFB                      /* the mouse pointer on what a
                                           click would choose */

static const u8 CODE DrLe_S[14] = " Drive letter ";
static const u8 CODE Choo_S[7] = "Choose ";
static const u8 CODE Left_S[4] = "left";
static const u8 CODE Rght_S[5] = "right";
static const u8 CODE Driv_S[7] = " drive:";

/* DX of a call of DOS that is no pointer: see CLAUDE.md. */
#define DX_OF(regs) (*(u16 STACK *)&(regs).name)

/* The letters of the drives that the box offers. */
#define DRIVES (md.TmpBuf)

/* Ask for the drive of the panel `panel`, with the item `item` of the
   menu bar marked, 1 for the left panel and 5 for the right one, and put
   the panel on the drive that is chosen. Returns 0 if none was. */
u16 F62_01(u16 panel, u16 item)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct DosRegs regs;
    u32 pair;
    u16 __far *next;
    u16 *screen;
    u16 *first;                         /* DI: where the first letter is */
    u8 *cell;
    u16 begin, size;                    /* BX and DX: the box */
    u16 count;                          /* CL: how many drives */
    u16 cursor;                         /* CH: the one of the cursor */
    u16 apart;                          /* BP: cells between two letters */
    u16 wide, letter, unit, active, place, right, color, c, i, key;
    u16 state, look, mouse, at, index, failed;

    regs.cx = item;
    regs.name = md.TmpBuf;
    regs.si = 0;
    regs.di = 0;
    HidCur();
    PutMnu(item);
    count = 0;
    for (letter = 'A'; letter <= 'Z'; letter++) {
        if (!TstDrv(letter))
            continue;
        /* Of A: and B:, where they are one floppy drive, only the letter
           that the drive goes by now. */
        if (letter <= 'B' && md.DriveB == 0) {
            unit = letter - ('A' - 1);
            regs.ax = 0x440E;
            regs.bx = (3 - unit) << 8 | unit;
            active = 0;
            if (!Int21d((struct DosRegs STACK *)&regs))
                active = regs.ax & 0xFF;
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
        size = wide;
    size = ADD_LOW(size, 10 + 2);
    begin = ADD_LOW(begin, -((size + 1 & 0xFF) >> 1));
    begin = 5 << 8 | begin & 0xFF;
    size = (6 + 1) << 8 | size & 0xFF;
    begin = SftEGA(begin);
    SavWin(begin, size, (u16)md.WinBuf);
    color = Color(Box_C) << 8;
    MinBox(begin, size, color);

    place = (u8)((begin >> 8) + 1) << 8 | (u8)(BegBX(panel) + 13);
    MvsBw(BasAdr(place), DrLe_S, 14, color);
    right = (place & 0xFF) >= 30;
    place = (place & 0xFF00) + 0x100 | (right ? 11 + 39 : 11);
    next = MvsBw(BasAdr(place), Choo_S, 7, color);
    c = Color(BoxBr_C) << 8;
    if (right)
        next = MvsBw(next, Rght_S, 5, c);
    else
        next = MvsBw(next, Left_S, 4, c);
    MvsBw(next, Driv_S, 7, color);

    /* The letters. */
    place = (begin & 0xFF00 | BegBX(panel) & 0xFF) + 0x314
        - (wide + 1 >> 1);
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
        if (DRIVES[i] == wcb->WinPath[0]) {
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
    pair = GetMous(0);
    if (LOW(pair) == 0)
        goto F62_30;
    state = 0;
    look = 0;
F62_21:
    state = state & 0x80 | LOW(pair) & 0xFF;
    mouse = HIGH(pair);
    c = 'x';
    if (state & 4)
        goto F62_27;
    c = CHECK;
    if (state & 2)
        goto F62_27;
    at = (u16)((u8 *)first - (u8 *)md.ScrBuf) >> 1;
    place = (at / 80 & 0xFF) << 8 | at % 80;
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
    c = CHECK;
    if ((u8)index == cursor)
        goto F62_27;
F62_26:
    c = 0;
F62_27:
    if (c != look) {
        look = c;
        TypMous(c);
    }
    pair = GetMous(mouse);
    if (LOW(pair) != 0)
        goto F62_21;
    TypMous(0);
    if (state & 4)
        goto F62_37;
    if (state & 2)
        goto F62_36;
    if (look != 0)
        goto F62_36;
    pair = GetMous(mouse);
    if (InWind(HIGH(pair), begin, size - 0x102))
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
    ResWin(begin, size, (u16)md.WinBuf);
    Window(begin, size);

F62_41:                                 /* the drive, and where it is */
    if (letter == 0)
        return 0;
    HidCur();
    unit = (u8)(letter - ('A' - 1));
    regs.ax = 0x440F;
    regs.bx = letter << 8 | unit;
    regs.name = (const u8 __far *)wcb;
    DX_OF(regs) = size & 0xFF00 | unit;
    Int21d((struct DosRegs STACK *)&regs);
    md.BlocEr = 1;
    regs.ax = 0x4700 | regs.ax & 0xFF;
    regs.si = (u16)(u32)(void __far *)(wcb->WinPath + 3);
    failed = Intr21((struct DosRegs STACK *)&regs);
    if (!failed) {
        wcb->WinPath[0] = (u8)letter;
        regs.name = wcb->WinPath;
        regs.ax = 0x3B00 | regs.ax & 0xFF;
        failed = Intr21((struct DosRegs STACK *)&regs);
    }
    md.BlocEr = 0;
    if (failed) {
        letter = ErrRdy(letter) & 0xFF;
        goto F62_41;
    }
    /* The drive is DOS's too, unless the other panel is the current one
       and shows files. */
    if (panel != md.WCBcrn && FilePan(InvWCB(panel)))
        goto F62_45;
    regs.ax = 0x0E00 | regs.ax & 0xFF;
    DX_OF(regs) = unit - 1;
    Int21d((struct DosRegs STACK *)&regs);
F62_45:
    wcb->WinTyp = 0;
    wcb->ChgFlg = 1;
    md.BlocEr = 1;
    wcb->CurAdr = DSKBUF;
    wcb->EndBuf = DSKBUF;
    return 1;
}
