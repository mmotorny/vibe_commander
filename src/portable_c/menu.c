/*
 * The menu bar and its pull-down menus.
 *
 * Translated from VCMENU.INC.
 *
 * F9, Shift-F10 or a click on the first row of the screen shows the bar,
 * and from there on this takes the keys and the mouse until an item of a
 * menu is chosen or the bar is left. What the item does is not here: the
 * main loop finds its command in the tables of commands_t.c, one
 * for each menu and two more for the commands of the Files menu with
 * Shift and with Ctrl (F39_01 of main.c).
 *
 * While the bar is up, the row it covers is kept in ErrBuf and what a
 * menu covers in WinBuf. TmpBuf has a byte for every line of the menu
 * that is open, MnuList: the letter that chooses the item, or 0 for a
 * line across the menu and for an item that is dark. The 201st byte of
 * TmpBuf, MnuMous, is what the buttons of the mouse were when last asked.
 */
#include "vc.h"

#define MnuBx_C 9                       /* the colours of a menu: its frame, */
#define MnuTx_C 10                      /* an item, */
#define MnuBr_C 11                      /* the letter of an item, */
#define MnuCu_C 12                      /* the item under the cursor, */
#define MnuCB_C 13                      /* its letter, */
#define MnuDr_C 14                      /* an item that cannot be chosen, */
#define MnuMi_C 15                      /* and the minus before one */

#define MnuList md.TmpBuf
#define MnuMous md.TmpBuf[200]

/* The byte of MnuList for an item, numbered from 1. The assembly makes a
   word of the number with its sign, so an item of 80h or more, which no
   menu has, is looked for before the list: the main data as bytes. */
#define LIST(item) MD_BYTES[MD_AT(TmpBuf) + (short)(signed char)(item) - 1]
/* Whether an item cannot be chosen, where the assembly holds its byte
   against the high byte of that word: 0 for every item there is. */
#define DARK(item) (LIST(item) == ((item) & 0x80 ? 0xFF : 0))

#define TICK   0xFB                     /* an option that is on */
#define LINE   0xC4                     /* a single line */
#define SIDE   0xB3                     /* the side of a menu */
#define TOP    0xDABF                   /* its corners, left and right: top */
#define BOTTOM 0xC0D9                   /* and bottom */

#define KEY_F10 0x4400
#define KEY_ALT 0xFD00                  /* Alt, pressed and let go by itself */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen. ClrKbd empties the keyboard
   buffer if the setup has it so. SavWin keeps what the screen buffer has
   where a window is to be, ResWin puts it back. GetMous returns the
   buttons of the mouse that are down and where the mouse is, or `place`
   as it was if there is no mouse. InWind is not 0 if a
   place is outside a window.

   Of the C routines in vc.h, PutKey shows a key bar, if there is one;
   PtMnu1 writes the menu bar with an item marked, 0 for none, whether or
   not the bar is always visible; and Input waits for a key, with a key
   bar, and returns it, the scan code in the high byte: F1 it answers
   with the help of the page HlpPage itself. */

/* F39_00, the bar itself, is declared in vc.h: see "The commands"
   there. The text of the bar, the names of its menus, is of
   bars_t.c. */
extern const u8 PtMnu_S[];

/* The lines of the menus, one behind the other, each ended by a 0, and
   an empty one for a line across the menu. FFh and a number are that
   many spaces, ^ is written Ctrl- and @ Alt-. The first capital letter
   of a line chooses it. Two menus have lines less than the original's
   (see DEVIATIONS.md): the Commands menu "memory Info" with Alt-F5
   behind the volume label and "EGA lines" with Alt-F9 behind the
   history, and the Options menu " Memory allocation" behind the clock.
   So each has a page of the help less in MnuHlp and a row less in
   MnuTab for every such line, and in KeyTab Alt+F5 and Alt+F9 choose
   nothing, the keys of the Commands menu below each choose an item
   further up, and so does Shift-F9, Save setup. And the Options menu has a line that the
   original has not, " More setup..." at its bottom, behind Save setup
   and a line across the menu of its own: the setup program, which is a
   program of its own there. Every item above it is where it would be
   without it, the menu is two rows taller for it in MnuTab, and the
   item has no page of the help: F1 on it shows the index. */
#define LEFT_S \
    " Brief\0" \
    " Full\0" \
    " Info\0" \
    " Tree\0" \
    " On/Off\377\007^F1\0" \
    "\0" \
    " Name\0" \
    " eXtension\0" \
    " tiMe\0" \
    " Size\0" \
    " Unsorted\0" \
    "\0" \
    " Re-read\377\006^R\0" \
    " fiLter...\377\004^F\0" \
    " Drive...\377\005@F1\0"
#define FILES_S \
    "Help\377\016F1\0" \
    "User menu\377\011F2\0" \
    "View\377\016F3\0" \
    "Edit\377\016F4\0" \
    "Copy\377\016F5\0" \
    "Rename or move\377\004F6\0" \
    "Make directory\377\004F7\0" \
    "Delete\377\014F8\0" \
    "\0" \
    "file Attributes\377\003^A\0" \
    "\0" \
    "select Group\377\006Gray +\0" \
    "uNselect group\377\004Gray -\0" \
    "Invert group\377\006Gray *\0" \
    "reStore selection ^M\0" \
    "\0" \
    "Quit\377\016F10\0"
#define COMMANDS_S \
    "Tree\377\022@F10\0" \
    "Volume label\377\012^F4\0" \
    "Directory sizes\377\007@F6\0" \
    "Find file\377\015@F7\0" \
    "History\377\017@F8\0" \
    "\0" \
    "Swap panels\377\013^U\0" \
    "Panels on/off\377\011^O\0" \
    "Compare directories\377\003^C\0" \
    "\0" \
    "Menu file edit\0"
#define OPTIONS_S \
    " Configuration...\0" \
    " advanced Options...\0" \
    " eXtension file edit...\0" \
    " Viewers...\0" \
    " Editors...\0" \
    "\0" \
    " Auto menus\0" \
    " Path prompt\0" \
    " Key bar\377\014^B\0" \
    " Full screen\0" \
    " auto Directory sizes\0" \
    " miNi status\0" \
    " cLock\0" \
    "\0" \
    " Save setup\377\011Shift-F9\0" \
    "\0" \
    " More setup...\0"
static const u8 MenuLn[sizeof(LEFT_S FILES_S COMMANDS_S OPTIONS_S) - 1]
    NONSTRING =
    LEFT_S FILES_S COMMANDS_S OPTIONS_S;

/* Where in MenuLn the lines of each menu begin. */
#define MenuLn1 0
#define MenuLn2 (MenuLn1 + sizeof LEFT_S - 1)
#define MenuLn3 (MenuLn2 + sizeof FILES_S - 1)
#define MenuLn4 (MenuLn3 + sizeof COMMANDS_S - 1)

static const u8 Ctrl_S[9] NONSTRING = "Ctrl-Alt-";
#define Alt_S 5                         /* where in it Alt- begins */

/* The pages of the help: for each kind of menu one for the menu itself
   and one for each of its lines. */
static const u8 MnuHlp[] = {
     7,  8,  8,  9, 10, 11,  0, 12, 12, 12, 12, 12,  0, 13, 14, 15,
    16,  0, 17, 18, 19, 20, 21, 22, 23,  0, 24,  0, 25, 25, 25, 25,  0, 26,
    27, 28, 29, 31, 32, 33,  0, 35, 36, 37,  0, 38,
    39, 40, 41, 42, 43, 44,  0, 45, 46, 47, 48, 49, 50, 51,  0, 53,  0,  0,
};
#define MnuHlp1 0                       /* where in it each kind begins */
#define MnuHlp2 16
#define MnuHlp3 34
#define MnuHlp4 46

/* The five menus of the bar: where the box of each begins, the row in the
   high byte, its size likewise, with its frame and its shadow, where its
   lines are in MenuLn and where its pages of the help in MnuHlp. */
struct MnuTab {
    u16 begin;
    u16 size;
    u16 lines;
    u16 help;
};
static const struct MnuTab MnuTab[] = {
    { 0x102, 0x121B, MenuLn1, MnuHlp1 },        /* Left */
    { 0x10A, 0x141E, MenuLn2, MnuHlp2 },        /* Files */
    { 0x113, 0x0E23, MenuLn3, MnuHlp3 },        /* Commands */
    { 0x11F, 0x1422, MenuLn4, MnuHlp4 },        /* Options */
    { 0x12A, 0x121B, MenuLn1, MnuHlp1 },        /* Right */
};

/* The letters that open the menus from the bar. */
static const u8 MnuKey[] = "LFCOR";

/* The keys that an open menu takes for one of its items: the key, and
   the item in the high byte over the menu in the low one. */
struct KeyTab {
    u16 key;
    u16 item;
};
static const struct KeyTab KeyTab[] = {
    { 0x5E00, 0x0501 }, { 0x1312, 0x0D01 }, { 0x2106, 0x0E01 },
    { 0x6800, 0x0F01 }, { 0x3B00, 0x0102 }, { 0x3C00, 0x0202 },
    { 0x3D00, 0x0302 }, { 0x3E00, 0x0402 }, { 0x3F00, 0x0502 },
    { 0x4000, 0x0602 }, { 0x4100, 0x0702 }, { 0x4200, 0x0802 },
    { 0x1E01, 0x0A02 }, { 0x4E2B, 0x0C02 }, { 0x4A2D, 0x0D02 },
    { 0x372A, 0x0E02 }, { 0x320D, 0x0F02 }, { 0x4400, 0x1102 },
    { 0x7100, 0x0103 }, { 0x6100, 0x0203 }, { 0x6D00, 0x0303 },
    { 0x6E00, 0x0403 }, { 0x6F00, 0x0503 }, { 0x1615, 0x0703 },
    { 0x180F, 0x0803 }, { 0x2E03, 0x0903 }, { 0x3002, 0x0904 },
    { 0x5C00, 0x0F04 }, { 0x5F00, 0x0505 }, { 0x1312, 0x0D05 },
    { 0x2106, 0x0E05 }, { 0x6900, 0x0F05 },
};
#define KEYS ((i32)(sizeof KeyTab / sizeof KeyTab[0]))

/* What the mouse says, for the menu bar. */
struct MenuMouse {
    u16 now;                            /* the buttons that are down */
    u16 before;                         /* and those that were, last time */
    u16 place;                          /* where it is, the row in the high
                                           byte */
};

/* Ask the mouse, and keep its buttons for the next time. `state` is what
   comes back for its place if there is none: the assembly has the menu
   and the item there. */
static void F40_40(u16 state, struct MenuMouse *mouse)
{
    struct Mouse pointer = GetMous(state);

    mouse->now = pointer.buttons;
    mouse->before = MnuMous;
    MnuMous = (u8)mouse->now;
    mouse->place = pointer.place;
}

/* The menu whose name on the bar is at a column, or 0: each has the two
   columns before its name and the two behind it. `bar` is the text of
   the bar, PtMnu_S: FFh, the number of spaces before a name, the name,
   and so on. */
static i32 F40_50(i32 column, const u8 *bar)
{
    i32 at = 0, menu = 0;

    column &= 0xFF;
    bar++;
    for (;;) {
        if (column < ((at + 2) & 0xFF))
            return menu;
        if (++menu > 5)
            return 0;
        at += *bar++;
        do
            at++;
        while (*bar++ != 0xFF);
        at--;
    }
}

/* Write the line of the item under the cursor in a colour, `color` and
   for its letter `letter`, and put it on the screen: with the colours of
   the cursor, or to take the cursor off it. Nothing for no item, FFh.
   The letter is the first cell of the line that has that character; a
   line without one would have its last cell coloured. */
static void F40_10(u16 color, u16 letter, i32 item, u16 begin, u16 size)
{
    u16 place, cell;
    i32 width, count;
    u16 *screen;

    if (item == 0xFF)
        return;
    place = (u16)((item + 1) << 8 | ((begin + 1) & 0xFF));
    width = (size - 4) & 0xFF;
    color = Color(color);
    screen = BasAdr(place);
    MvsColr(screen, (u16)width, color);
    cell = (u16)(color << 8 | LIST(item));
    for (count = width; count != 0; count--)
        if (*screen++ == cell)
            break;
    ((u8 *)screen)[-1] = (u8)Color(letter);
    Window(place, (u16)(0x100 | width));
}

/* Make an item of the menu that has just been drawn dark: it cannot be
   chosen, and has a minus before it. `top` is where the menu begins in
   the screen buffer. The minus stands a column further on in the menus
   of the panels and of the options, whose lines begin with a space: the
   first column is that of their ticks. */
static void F40_30(i32 item, i32 menu, u16 *top, u16 size)
{
    u16 *line = top + item * 80 + 1;
    u16 *minus = line;

    LIST(item) = 0;
    if (menu != 2 && menu != 3)
        minus++;
    MvsColr(line, (size - 4) & 0xFF, Color(MnuDr_C));
    *minus = (u16)(Color(MnuMi_C) << 8 | '-');
}

/* Draw a menu into the screen buffer: its frame, its lines from `lines`
   in MenuLn on, and its shadow, and fill MnuList for it. Each of the
   five has lines; one without would have 65536 in the original. */
static void F39_52(u16 *top, u16 size, i32 lines)
{
    u16 box = (u16)(Color(MnuBx_C) << 8);
    i32 inner = (size - 4) & 0xFF;      /* between the corners */
    i32 rows = ((size >> 8) - 3) & 0xFF;
    u8 *list = MnuList;
    u16 *line, *cell;
    u16 color, c;
    i32 count, from, cells;

    line = BoxLine(top, box | LINE, TOP, 0, inner);
    do {
        cell = line;
        *list = 0;
        *cell++ = box | SIDE;
        *cell++ = box | ' ';
        count = inner - 2;
        color = box;
        if (MenuLn[lines] == 0) {
            lines++;
            for (; count != 0; count--)
                *cell++ = box | LINE;
        } else {                        /* F39_53 */
            color = (u16)(Color(MnuTx_C) << 8);
            ((u8 *)cell)[-1] = color >> 8;
            do {                        /* F39_54 */
                c = MenuLn[lines++];
                while (c == 0xFF) {     /* F39_95 */
                    cells = MenuLn[lines++];
                    count = ADD_LOW(count, -cells);
                    for (; cells != 0; cells--)
                        *cell++ = color | ' ';
                    c = MenuLn[lines++];
                }
                if (c == '^' || c == '@') {
                    from = c == '^' ? 0 : Alt_S;
                    for (;;) {          /* F39_96 */
                        c = Ctrl_S[from++];
                        if (c == '-')
                            break;
                        *cell++ = color | c;
                        count--;
                    }
                }
                if (c <= 'Z' && c >= 'A' && *list == 0) {       /* F39_97 */
                    *list = (u8)c;
                    *cell++ = (u16)(Color(MnuBr_C) << 8 | c);
                } else
                    *cell++ = color | c;
                count--;
            } while (MenuLn[lines] != 0);
            lines++;
            for (; count != 0; count--)
                *cell++ = color | ' ';
        }
        *cell++ = color | ' ';          /* F39_56 */
        *cell++ = box | SIDE;
        Shadow(Shadow(cell));
        list++;
        line += 80;
    } while (--rows);

    cell = line;
    *cell++ = box | BOTTOM >> 8;
    for (count = inner; count != 0; count--)
        *cell++ = box | LINE;
    *cell++ = box | (BOTTOM & 0xFF);
    Shadow(Shadow(cell));
    cell = line + 82;
    count = inner + 2;
    do
        cell = Shadow(cell);
    while (--count);
}

/* What a menu that has just been drawn says of how things are: the ticks
   of the options that are on, the kind of a panel and the order of its
   files; and its items that cannot be chosen as things are. `panel` is
   the panel that was the current one when the bar came up; the Left and
   the Right menu are of their own panels. */
static void F39_58(i32 menu, struct WCBdef *panel, u16 *top, u16 size)
{
    const struct WCBdef *wcb = panel;
    const struct WCBdef *other;
    u16 *mark;
    i32 order;

    if (menu == 4) {                    /* Options */
        u8 *tick = (u8 *)(top + 561);

#define F40_20(option) \
        if (md.option != 0) \
            *tick = TICK; \
        tick += 2 * 80;
        F40_20(AutoMnu)
        F40_20(Prompt)
        F40_20(KeyBar)
        F40_20(FulScr)
        F40_20(AutoSiz)
        F40_20(MiniSta)
        F40_20(Clock)
#undef F40_20
    } else if (menu == 2) {             /* Files */
        if (wcb->Visible == 0 || wcb->WinTyp != 0) {
            F40_30(10, menu, top, size);
            F40_30(12, menu, top, size);
            F40_30(13, menu, top, size);
            F40_30(14, menu, top, size);
            F40_30(15, menu, top, size);
        }
    } else if (menu == 3) {             /* Commands: Swap panels,
                                           Directory sizes and Compare
                                           directories. The original has
                                           Memory info above the three
                                           and EGA lines above two, so
                                           each is an item or two
                                           further up here; and EGA
                                           lines is dark there on a
                                           display without them */
        if (wcb->Visible == 0) {
            F40_30(7, menu, top, size);
            F40_30(3, menu, top, size);
            F40_30(9, menu, top, size);
        } else if (wcb->WinTyp != 0) {
            F40_30(3, menu, top, size);
            F40_30(9, menu, top, size);
        } else {
            other = InvWCB(panel);
            if (other->Visible == 0 || other->WinTyp != 0)
                F40_30(9, menu, top, size);
        }
    } else {                            /* Left / Right */
        panel = LeftPanel;
        if (menu != 1) {
            panel = RightPanel;
            ++*(u8 *)(top + 422);       /* Ctrl-F2 */
            ++*(u8 *)(top + 1221);      /* and Alt-F2 */
        }
        wcb = panel;
        mark = top + 401;               /* On/Off */
        if (wcb->Visible != 0) {
            mark -= 320;                /* Brief */
            if (wcb->WinTyp == 0) {
                if (wcb->BrfFul != 0)
                    mark += 80;         /* Full */
            } else {
                mark += 160;            /* Info */
                if (wcb->WinTyp != 2) {
                    mark += 80;         /* Tree */
                    if (wcb->WinTyp != 1)
                        mark += 80;
                }
            }
        }
        *(u8 *)mark = TICK;
        if (wcb->Visible != 0 && wcb->WinTyp == 0) {
            order = wcb->SortTyp;
            if (order > 4)
                order = 4;
            *(u8 *)(top + order * 80 + 561) = TICK;
        } else
            F40_30(14, menu, top, size);
        other = InvWCB(panel);
        if (other->Visible != 0 && other->WinTyp == 1)
            F40_30(4, menu, top, size);
    }
}

/* The menu bar: F9, or with `again` Shift-F10, which goes back to the
   item that was chosen last. `panel` is the current panel. Takes keys and
   the mouse until an item is chosen or the bar is left, and returns the
   menu and the item, struct Picked of vc.h, with which of the tables of
   commands_t.c has the command of that item, from 1, or 0 if none was
   chosen.
   The menu is 1 to 5, the item is counted from 1 with the lines across
   the menu, and LastMnu is given both. `held` there is what the assembly
   has in DL when it jumps to the command, for a command that reads it
   there, as renaming does: 50h, of the size of the bar's row, or what
   the BIOS said of the keys held down.

   On the bar alone the item is 0. FFh is a menu that is open with no
   item under the cursor, which only the mouse brings about. */
struct Picked F39_00(bool again, struct WCBdef *panel)
{
    const u8 *names = PtMnu_S;
    struct MenuMouse mouse;
    struct Picked picked;
    i32 menu, item;                     /* CL and CH */
    u16 begin = 0, size = 0x150;        /* of the open menu: BX and DX */
    u16 key, held;
    i32 to, i, count, table;
    bool hit;
    u16 *top;

    if (again) {                        /* F40_00 */
        menu = md.LastMnu & 0xFF;
        item = md.LastMnu >> 8;
    } else {
        item = 0;
        menu = panel->Visible != 0 && md.Active != 0 ? 5 : 1;
    }
    HidCur();                           /* F39_01 */
    SavWin(0, 0x150, md.ErrBuf);
    PutKey(1);

F39_02:
    F40_40((u16)(item << 8 | menu), &mouse);
    if (mouse.now != 0 && mouse.place >> 8 == 0) {
        menu = F40_50(mouse.place, names);
        if (menu == 0) {
            /* A button is down on the bar and on none of its names:
               until it is on one, or let go. */
            PtMnu1(0);
            item = 0;
            do {
                do {                    /* F39_03 */
                    F40_40((u16)(item << 8 | menu), &mouse);
                    if (mouse.now == 0)
                        goto F39_85;
                    ClrKbd();
                } while (mouse.place >> 8 != 0);
            } while (F40_50(mouse.place, names) == 0);
        }
    }
    if (item != 0)                      /* F39_04 */
        goto F39_51;
F39_07:
    PtMnu1((u16)menu);

F39_10:
    /* The page of the help for F1, and then the mouse or a key. */
    md.HlpPage = MnuHlp[MnuTab[menu - 1].help + (item == 0xFF ? 0 : item)];
    F40_40((u16)(item << 8 | menu), &mouse);
    if (mouse.now != 0) {
        ClrKbd();
        if (mouse.place >> 8 != 0) {    /* F39_17: below the bar */
            if (item == 0)
                goto F39_10;
            to = 0xFF;                  /* F39_12 */
            if (!InWind(mouse.place, begin + 0x101, size - 0x304)) {
                to = ((mouse.place >> 8) - 1) & 0xFF;
                if (LIST(to) == 0)
                    to = 0xFF;
            }
            goto F39_41;
        }
        to = F40_50(mouse.place, names);
        if (item != 0) {
            if (to == menu) {
                to = 0xFF;
                goto F39_41;
            }
            ResWin(begin, size, md.WinBuf);    /* F39_13 */
            if (to == 0)
                Window(begin, size);
        }
        menu = to;                      /* F39_14 */
        item = 0xFF;
        if (to != 0)
            goto F39_51;
        goto F39_02;
    }
    if (mouse.before & 7) {             /* F39_15: a button let go */
        if (mouse.place >> 8 != 0) {    /* F39_16 */
            MnuMous = (u8)mouse.before;
            goto F39_44;
        }
        if (item == 0xFF)
            goto F39_39;
    }

    key = Input(1);                     /* F39_18 */
    /* Case takes no key without a scan code. */
    if (key >> 8 != 0) {
        if ((key & 0xFF) == 0x0A)       /* ^Enter */
            goto F39_43;
        if ((key & 0xFF) == 0x1B)       /* Esc */
            goto F39_46;
        switch (key) {
        case 0x4B00: goto F39_25;       /* Left */
        case 0x4D00: goto F39_26;       /* Right */
        case 0x4700:                    /* Home */
        case 0x4900: goto F39_30;       /* PgUp */
        case 0x4F00:                    /* End */
        case 0x5100: goto F39_32;       /* PgDn */
        case 0x4800: goto F39_33;       /* Up */
        case 0x5000: goto F39_37;       /* Down */
        case 0x1C0D: goto F39_43;       /* Enter */
        case KEY_ALT: goto F39_46;
        }
    }
    key = UpCase(key);                  /* Letters */
    if (item == 0) {
        for (i = 0; i < 5; i++)
            if (MnuKey[i] == (key & 0xFF)) {
                menu = i + 1;
                goto F39_50;
            }
        goto F39_20;
    }
    for (i = 0; i < KEYS; i++)          /* F39_21 */
        if (KeyTab[i].key == key && (KeyTab[i].item & 0xFF) == menu) {
            to = KeyTab[i].item >> 8;   /* F39_24 */
            if (!DARK(to)) {
                item = to;
                goto F39_44;
            }
            /* The key of an item that cannot be chosen: the number of
               the item is looked for among the letters, where it is
               not. */
            key = (u16)to;
            break;
        }
    if ((key & 0xFF) != 0) {            /* F39_23 */
        count = ((size >> 8) - 3) & 0xFF;
        i = 0;
        hit = count == 0;
        while (!hit && i != count)
            hit = MnuList[i++] == (key & 0xFF);
        if (hit) {
            item = i & 0xFF;
            goto F39_44;
        }
        goto F39_10;
    }
F39_20:
    if (key == KEY_F10)
        goto F39_46;
    goto F39_10;

F39_25:                                 /* Left */
    menu = (menu - 1) & 0xFF;
    if (menu == 0)
        menu = 5;
    goto F39_27;
F39_26:                                 /* Right */
    if (++menu > 0xFF) {
        menu = 0;
        item = (item + 1) & 0xFF;
    }
    if (menu > 5)
        menu = 1;
F39_27:
    if (item == 0)
        goto F39_07;
    ResWin(begin, size, md.WinBuf);
    goto F39_50;
F39_30:                                 /* Home, PgUp */
    if (item != 0)
        goto F39_39;
    goto F39_10;
F39_32:                                 /* End, PgDn */
    if (item != 0)
        goto F39_35;
    goto F39_10;
F39_33:                                 /* Up */
    if (item == 0)
        goto F39_10;
    to = item;
F39_34:
    to = (to - 1) & 0xFF;
    if (to != 0)
        goto F39_36;
F39_35:
    to = ((size >> 8) - 3) & 0xFF;
F39_36:
    if (DARK(to))
        goto F39_34;
    goto F39_41;
F39_37:                                 /* Down */
    if (item == 0)
        goto F39_50;
    to = item;
F39_38:
    to = (to + 1) & 0xFF;
    if (to < (((size >> 8) - 2) & 0xFF))
        goto F39_40;
F39_39:
    to = 1;
F39_40:
    if (DARK(to))
        goto F39_38;
F39_41:
    if (to == item)
        goto F39_10;
    F40_10(MnuTx_C, MnuBr_C, item, begin, size);        /* Delete cursor */
    item = to;
F39_42:
    F40_10(MnuCu_C, MnuCB_C, item, begin, size);        /* Set cursor */
    goto F39_10;
F39_43:                                 /* Enter */
    if (item == 0)
        goto F39_50;
F39_44:
    ResWin(begin, size, md.WinBuf);
    Window(begin, size);
    goto F39_85;
F39_46:                                 /* Esc, Alt, F10 */
    if (item == 0)
        goto F39_85;
    ResWin(begin, size, md.WinBuf);
    Window(begin, size);
    item = 0;
    if (key != KEY_ALT)
        goto F39_85;
    goto F39_10;

F39_50:                                 /* Window */
    item = 1;
F39_51:
    PtMnu1((u16)menu);
    i = (menu - 1) & 0xFF;
    begin = MnuTab[i].begin;
    size = MnuTab[i].size;
    SavWin(begin, size, md.WinBuf);
    top = BasAdr(begin);
    F39_52(top, size, MnuTab[i].lines);
    F39_58(menu, panel, top, size);
    Window(0x100, 0x1550);              /* F39_80 */
    if (item == 0xFF)
        goto F39_10;
    while (DARK(item)) {                /* F39_81 */
        item = (item + 1) & 0xFF;
        if (item >= (((size >> 8) - 2) & 0xFF))
            item = 1;
    }
    goto F39_42;

F39_85:                                 /* Exit */
    ResWin(0, 0x150, md.ErrBuf);
    Window(0, 0x150);
    table = 0;
    held = 0x50;
    if (item != 0 && item != 0xFF) {
        md.LastMnu = (u16)(item << 8 | menu);
        table = menu;
        if (table == 5)
            table = 1;
        if (table == 2 && item < 9) {
            /* A command of the function keys: with Shift or Ctrl held
               down, or the right or the middle button of the mouse, the
               command that the key has with it. */
            held = KeysHeld();
            if (held & 4)
                table = 6;
            else if (held & 3)
                table = 5;
            else if (MnuMous & 4)
                table = 6;
            else if (MnuMous & 2)
                table = 5;
        }
    }
    picked.menu = (u8)menu;
    picked.item = (u8)item;
    picked.held = (u8)held;
    picked.table = (u8)table;
    return picked;
}
