/*
 * The mouse: finding it, its pointer, where it is and which buttons are
 * held, and a second click.
 *
 * Translated from VCSUB1.INC.
 *
 * The mouse driver counts in dots, eight to a cell of the text screen;
 * the program goes by cells, the row in the high byte of a place and the
 * column in the low one. What it asks of the driver goes through Int33
 * and MousAt of dos.c.
 */
#include "vc.h"

#define MoDelay 40                      /* Mouse double click delay */

#define TICK 0xFB                       /* the pointer on what a second
                                           click would take */

/* Look for a mouse, and set it up if there is one: the pointer a block
   that turns the colours of its cell round, the rows it may go to those
   of the screen where that has more than 25, and the pointer where it
   was when the program last noted that, shown. Without a mouse the
   place noted is none. */
void IniMous(void)
{
    u16 answer = Int33(0x21, 0, 0, 0);

    if (answer == 0x21)
        answer = Int33(0, 0, 0, 0);
    md.Mouse = answer == 0xFFFF;
    if (md.Mouse == 0) {
        md.MouseX = 0xFFFF;
        md.MouseY = 0xFFFF;
        return;
    }
    TypMous(0);
    if (md.Lines != 25)
        Int33(8, 0, 0, ((u16)md.Lines << 3) - 1);
    if ((md.MouseX & md.MouseY) != 0xFFFF)
        Int33(4, 0, md.MouseX, md.MouseY);
    CurMous(1);
}

/* What the mouse pointer looks like: the character in the low byte of
   `pointer`, in white on black, or for 0 the cell that it is on with
   its colours turned round. */
void TypMous(u16 pointer)
{
    pointer &= 0xFF;
    if (pointer == 0)
        Int33(0x0A, 0, 0xFFFF, 0x7700);
    else
        Int33(0x0A, 0, 0xFF00, 0x7700 | pointer);
}

/* Note where the mouse pointer is, for IniMous to put it back. */
void SavMous(void)
{
    struct MouseAt at;

    if (md.Mouse == 0)
        return;
    MousAt((struct MouseAt STACK *)&at);
    md.MouseX = at.x;
    md.MouseY = at.y;
}

/* Show the mouse pointer, or hide it if the low byte of `show` is 0. */
void CurMous(u16 show)
{
    if (md.Mouse != 0)
        Int33((show & 0xFF) == 0 ? 2 : 1, 0, 0, 0);
}

/* Where the mouse is and which of its buttons are held: the buttons in
   the low word, the left one 1, the right one 2 and the middle one 4,
   the first two the other way round for a left hand, and the place in
   the high word. Without a mouse no button is held and the place is
   `place` as it came. */
u32 GetMous(u16 place)
{
    struct MouseAt at;
    u16 buttons = 0;

    if (md.Mouse != 0) {
        MousAt((struct MouseAt STACK *)&at);
        place = (at.y >> 3) << 8 | (at.x >> 3) & 0xFF;
        buttons = at.buttons;
        if (md.LftMous != 0)
            buttons = buttons & 4 | buttons >> 1 & 1 | buttons << 1 & 2;
    }
    return (u32)place << 16 | buttons & 7;
}

/* A button has been pressed on something that a second click takes, at
   `begin` and of the size `size`, which is the line `cursor` of a list.
   Returns 2 to go on as for any click: for a button that is still held
   from the click before, and for a first click, of which the time and
   the line are noted. A click on the same line within 40 hundredths of a
   second of the first is the second: the pointer is a tick while it is
   on the place, until the buttons are let go, and what comes back is 1
   if that was on the place, to do what Enter does, and 0 if not.

   The time is DOS's, in four bytes, and the 40 hundredths are added to
   it byte by byte: a hundredth beyond 99 carries a second over and stays
   as it is, 100 or more, which no time of day reaches; and so with the
   seconds and the minutes. The second click is in time until the clock
   has gone beyond that, which for the last 40 hundredths of a second is
   when the next second ends. */
u16 DoublMo(u16 begin, u16 size, u16 cursor)
{
    u32 pair;
    u16 hours, seconds, pointer;

    if (md.MousEnt != 0)
        return 2;
    md.MousEnt = 1;
    pair = DosTime();
    hours = (u16)pair;
    seconds = (u16)(pair >> 16);
    if ((u16)(md.MousTim >> 16) != 0xFFFF && cursor == md.MousPos
        && (hours < (u16)(md.MousTim >> 16)
            || hours == (u16)(md.MousTim >> 16)
               && seconds <= (u16)md.MousTim)) {
        /* The pointer is kept in the high byte of the hours, as the
           assembly has it in CH, and the minutes below it go to GetMous
           with it as the place for a mouse that is not there. */
        hours &= 0xFF;
        do {
            pair = GetMous(hours);
            pointer = InWind((u16)(pair >> 16), begin, size) ? 0 : TICK;
            if (pointer != hours >> 8) {
                hours = hours & 0xFF | pointer << 8;
                TypMous(pointer);
            }
        } while ((u16)pair & 7);
        return hours >> 8 != 0;
    }
    seconds = ADD_LOW(seconds, MoDelay);
    if ((seconds & 0xFF) >= 100) {
        seconds += 0x100;
        if (seconds >> 8 >= 60) {
            hours = ADD_LOW(hours, 1);
            if ((hours & 0xFF) >= 60)
                hours += 0x100;
        }
    }
    md.MousTim = (u32)hours << 16 | seconds;
    md.MousPos = cursor;
    return 2;
}

/* Is a place outside a window? `where` is the place, `begin` where the
   window begins and `size` its rows and columns, each with the row in
   the high byte. Not 0 if it is outside. A window of no rows or no
   columns has 256 of them. */
u16 InWind(u16 where, u16 begin, u16 size)
{
    u16 row = where >> 8, column = where & 0xFF;

    if (row < begin >> 8)
        return 1;
    row -= begin >> 8;
    if (column < (begin & 0xFF))
        return 1;
    column -= begin & 0xFF;
    if ((u8)((size >> 8) - 1) < row)
        return 1;
    return (u8)(size - 1) < column;
}
