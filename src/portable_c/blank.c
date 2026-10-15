/*
 * The screen blanker, and the word that the program is idle.
 *
 * Translated from VCSUB2.INC.
 *
 * Input0 calls LeaveTm every time it has gone round waiting for a key,
 * and Blank when no key has come for the minutes of the setup, or when
 * the mouse has gone into the top right corner of the screen. Blank shows
 * a night sky in place of the screen, on the display itself, through the
 * BIOS: the screen buffer keeps what the program had drawn, and Input0
 * puts that back.
 *
 * A star comes out at a place that the program's random numbers choose,
 * shines for some hundred turns of the timer, and goes out in a few
 * turns, or flares up first. The numbers are Rnd's, of input.c, which
 * Input0 steps too each time it goes round: so the sky is another every
 * time. Nothing sets the two words that the numbers are made of, Rand1
 * and Rand2, when the program starts: the first sky follows from what
 * the memory held.
 */
#include "vc.h"

#define Sky1_C 36                       /* the colour of the sky, and of a
                                           star that shines */
#define Sky2_C 37                       /* of a star that goes out */

/* The corner that the mouse may move in without ending the sky: four
   columns and two rows at the top right. */
#define CORNER_BEGIN 76
#define CORNER_SIZE  0x204

/* How long a star has left, the low byte of its StarMod, when it changes
   its look, and what it looks like from then on, as a character of
   StarTab: the first of those is the star as it comes out. */
#define FLARE_1 12
#define FLARE_2 9
#define FLARE_3 6
#define FLARE_4 3

#define NO_CURSOR 0x2000                /* a shape of the cursor that does
                                           not show */

/* Tell DOS that the program has nothing to do at the moment. The
   original tells Windows too, and DESQview: see DEVIATIONS.md. */
void LeaveTm(void)
{
    Idle();
}

/* A random number below `limit`, which is 255 at the most: the high word
   of the next of the program's numbers times the limit. The product has
   32 bits, and is worked out here in halves that have 16. */
static i32 Below(i32 limit)
{
    u16 number = Rnd();

    return ((number >> 8) * limit + ((number & 0xFF) * limit >> 8)) >> 8;
}

/* Write a character on the display, at a row and a column, the row in
   the high byte of `place`, in a colour of the program's. The system is
   asked for the page that is shown first, and the cursor goes to the
   place. */
static void Star(u16 place, u16 look, u16 color)
{
    u16 page = ScreenState().page;

    CursorTo(page, place);
    ScreenFill(page, look, Color(color), 1);
}

/* Blank the screen and show the sky, until a key is pressed, which is
   taken, Shift, Ctrl or Alt is held or a lock changes, a button of the
   mouse goes down, or the mouse moves, other than within the corner.
   Then the cursor is where it was and as it was; the screen is not.

   The sky changes every time the timer has ticked. A star comes out
   then, more often than not, unless there is one at its place already or
   there are as many as Stars has room for; and every star is a turn
   older.

   Ported as they are: with Shift, Ctrl or Alt held when the sky comes
   up, it goes away at once, after its first turn. The clock of a star is
   stepped as a word, with what kind of star it is in the high byte, and
   kept as a byte. */
void Blank(void)
{
    struct STAR *star;
    struct Mouse pointer;
    struct CursorAt cursor;             /* the cursor's, to put it back */
    u16 mouse;                          /* where the mouse is, last seen */
    u16 held;                           /* Shift, Ctrl, Alt and the locks */
    u16 ticks = 0;                      /* the timer's, at the last turn */
    u16 place, number, look, now;
    i32 clock, kind, mode, vacant, i;

    CurMous(0);
    cursor = CursorWhere(ScreenState().page);
    CursorShape(NO_CURSOR);
    /* The screen empty, as a window scrolled by all its rows. */
    ScreenScroll(false, 0, Color(Sky1_C), 0,
                 (u16)((u8)(md.Lines - 1) << 8 | 79));
    for (i = 0; i < NmbStrs; i++) {
        md.Stars[i].StarMod = 0;
        md.Stars[i].StarPos = 0;
    }
    mouse = GetMous(0).place;
    held = KeysHeld();

    for (;;) {
        /* Blnk01: a star comes out. */
        place = (u16)(Below(md.Lines) << 8);
        place |= Below(80);
        clock = Rnd() >> 3 & 0xFF;
        if (clock < 100)
            goto older;
        number = Rnd();
        look = number & 0xFF;
        look = (look >> 5 | look << 3) & 0xFF;
        if (look < 50)
            goto older;
        kind = (look | number >> 8) & 1;
        vacant = NmbStrs;
        for (i = 0; i < NmbStrs; i++) {         /* Blnk02 */
            if (md.Stars[i].StarMod == 0) {
                if (vacant == NmbStrs)
                    vacant = i;
            } else if (md.Stars[i].StarPos == place)
                goto older;
        }
        if (vacant == NmbStrs)
            goto older;
        md.Stars[vacant].StarMod = (u16)(kind << 8 | clock);
        md.Stars[vacant].StarPos = place;
        Star(place, md.StarTab[0], Sky1_C);

    older:
        /* Blnk05: every star a turn older. One of the kind 0 flares up
           four times before it goes; one of the other kind goes when
           the first would flare up for the second time. */
        for (i = 0; i < NmbStrs; i++) {
            star = &md.Stars[i];
            mode = star->StarMod;
            if (mode == 0)
                continue;
            place = star->StarPos;
            mode--;
            *(u8 *)&star->StarMod = (u8)mode;
            clock = mode & 0xFF;
            if (clock == FLARE_1)
                look = md.StarTab[1];
            else {
                if (clock == FLARE_2) {
                    if (mode >> 8 == 0) {
                        look = md.StarTab[2];
                        goto shown;
                    }
                    clock = 0;
                }
                if (clock == FLARE_3)           /* Blnk07 */
                    look = md.StarTab[3];
                else if (clock == FLARE_4)
                    look = md.StarTab[4];
                else if (clock != 0)
                    continue;
                else {
                    star->StarMod = 0;
                    star->StarPos = 0;
                    look = ' ';
                }
            }
        shown:                                  /* Blnk08 */
            Star(place, look, Sky2_C);
        }

        /* Blnk10: until the timer ticks. */
        for (;;) {
            if (!KeyWaiting().none) {
                KeyRead();                      /* Blnk13 */
                goto wake;
            }
            now = KeysHeld();
            i = (held & 0xF0) ^ now;
            held = now;
            if (i != 0)
                goto wake;
            pointer = GetMous(0);
            if (pointer.buttons != 0)
                goto wake;
            if (!InWind(pointer.place, CORNER_BEGIN, CORNER_SIZE))
                mouse = pointer.place;
            if (pointer.place != mouse)            /* Blnk11 */
                goto wake;
            now = (u16)Timer();         /* its low word: has it ticked */
            if (now != ticks)
                break;
            LeaveTm();                          /* Blnk12 */
        }
        ticks = now;
    }

wake:                                           /* Blnk14 */
    CursorTo(ScreenState().page, cursor.place);
    CursorShape(cursor.shape);
    CurMous(1);
}
