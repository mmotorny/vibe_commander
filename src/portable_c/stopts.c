/*
 * The option dialogs of the setup: the Configuration, Screen and Mouse,
 * Panel options with the delay of the tree that is typed there, and
 * Country info.
 *
 * Translated from VCSETUP.ASM, the setup program of the original, which
 * is a part of this program here: every name of that program has St in
 * front. These are items of the setup's menu: the original jumps to one
 * by the table of the menu and each jumps back to the menu, Main00. An
 * item is a function here, which the menu calls and which returns.
 *
 * The dialogs set the setup in the main data, where the setup program
 * has it in its own segment: what is edited holds inside the setup at
 * once, the key bar and the clock among it.
 */
#include "vc.h"

/* Routines of other files that are C. HexDec makes two decimal digits
   of a number below 100, the tens in the low byte. TxtNum reads a number
   in decimal digits, 0FFFFh for one that 16 bits do not hold, and
   returns where the digits end; FndLet returns what follows spaces and
   tabs. */
u16 HexDec(u16 number);                                 /* format.c */
const u8 __far *TxtNum(const u8 __far *text, u16 STACK *number); /* digits.c */
const u8 __far *FndLet(const u8 __far *text);           /* string.c */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   Dialog shows a dialog and has it answered: 0 in the low byte for Ok.
   Window writes a window of the screen buffer to the screen. UpCase and
   LowCase are the capital and the small letter of a character, as the
   country has them. AsmSeg returns the segment of the program's
   tables. */

/* The dialogs, and the lines of Country info that are read and written
   here: of stopts_t.c, in the segment that AsmSeg returns, where Dialog
   reads them by their offsets. */
extern const u8 __far StConf_B[];       /* the Configuration */
extern const u8 __far StScrn_B[];       /* Screen and Mouse */
extern const u8 __far StPanOp_B[];      /* Panel options */
extern const u8 __far StNoTim_B[];      /* "Illegal format of auto change
                                           dir time delay." */
extern const u8 __far StInfo_B[];       /* Country info */
extern u8 __far StInf02_S[];            /* its line of the time, the date
                                           and the number, */
extern const u8 __far StInf05_S[];      /* the first 40 of its characters, */
extern u8 __far StInf06_S[];            /* with their capitals */
extern u8 __far StInf07_S[];            /* and their small letters, */
extern const u8 __far StInf09_S[];      /* and the other 39 with a space, */
extern u8 __far StInf10_S[];            /* with theirs */
extern u8 __far StInf11_S[];

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))

#define NEAR(pointer) ((const u8 *)(u16)(u32)(pointer))

#define PAGE_PANELS 3                   /* the page of the setup's help
                                           that Panel options has */

/* The minutes after which the screen is blanked, as Screen and Mouse
   offers them, from the first line of the group down: Delay_T of the
   setup program, where it stands behind that dialog. */
static const u8 CODE StDelay_T[] = { 40, 20, 5, 3, 1, 0 };

/* The Configuration: all of it boxes to tick, which the dialog sets.
   If the key bar was on and is off now, its row is what DOS had shown
   there again, from the copy of the screen that the setup was entered
   with. Whatever the dialog was answered with: only Ok changes
   anything. */
void StF02_00(void)
{
    u8 bar = md.KeyBar;
    u16 row;

    Dialog(OFFSET(StConf_B), 0, 0, 0, MAIN_SEG);
    if (md.KeyBar != 0 || bar == 0)
        return;
    row = (u8)(md.Lines - 1);
    StUser(row, 1);
    Window(row << 8, 0x150);
    md.BarPrev = 0xFF;
}

/* Screen and Mouse. The palette, the minutes until the screen is
   blanked and Screen output are chosen from groups, which the dialog
   has at TmpBuf, TmpBuf+2 and TmpBuf+4, each with the line of its cursor
   behind it. The group of the minutes leaves 0 to 5. The boxes to tick
   and the characters of the screen saver the dialog sets itself.

   The assembly then calls SetSnow, whatever the answer, which finds out
   whether to wait for a display that shows snow: the portable C has no
   such wait (see DEVIATIONS.md), and nothing here goes by Screen
   output. */
void StF03_00(void)
{
    u8 delay;

    md.TmpBuf[0] = md.ColTyp;
    md.TmpBuf[1] = md.ColTyp;
    for (delay = 0; md.BlDelay < StDelay_T[delay]; delay++)
        ;
    md.TmpBuf[2] = delay;
    md.TmpBuf[3] = delay;
    md.TmpBuf[4] = md.SnowMod;
    md.TmpBuf[5] = md.SnowMod;
    if ((u8)Dialog(OFFSET(StScrn_B), 0, 0, 0, MAIN_SEG) != 0)
        return;
    md.ColTyp = md.TmpBuf[0];
    md.BlDelay = StDelay_T[md.TmpBuf[2]];
    md.SnowMod = md.TmpBuf[4];
}

/* Panel options. The delay of the tree is written for the dialog's line
   at TmpBuf+9: the seconds without a zero in front, the country's
   decimal separator, and two digits of hundredths. Either number is 99
   at the most here, whatever the setup has. The boxes to tick the
   dialog sets itself, and the delay StF04_10, which the dialog names. */
void StF04_00(void)
{
    u8 *text = md.TmpBuf + 9;
    u16 digits;
    u8 number;

    number = md.DelayTm[1];
    if (number > 99)
        number = 99;
    digits = HexDec(number);
    if ((u8)digits != '0')
        *text++ = (u8)digits;
    *text++ = (u8)(digits >> 8);
    *text++ = md.Country.DecimalSep[0];
    number = md.DelayTm[0];
    if (number > 99)
        number = 99;
    digits = HexDec(number);
    *text++ = (u8)digits;
    *text++ = (u8)(digits >> 8);
    *text = 0;
    Dialog(OFFSET(StPanOp_B), 0, 0, 0, MAIN_SEG);
}

/* What StF04_10 returns when it has refused nothing: the answer for the
   dialog to go by. */
static struct Checked Accepted(u16 answer)
{
    struct Checked checked;

    checked.answer = answer;
    checked.refused = 0;
    return checked;
}

/* What Panel options has Dialog call when it is answered: with Ok, 0,
   the delay of the tree is read from the line that was typed, which the
   dialog has at TmpErr+3, and put into the setup, DelayTm: hundredths
   of a second, and seconds. Returns the answer for the dialog to go by,
   `answer` as it came or 0 for Ok; and if the line is no delay, which a
   box says, what that box was answered with and that the line was
   refused, for the dialog to go on.

   A line of nothing but spaces is taken, and sets nothing. A delay is
   seconds, up to 99; then the end, or the country's decimal separator
   directly behind the digits; then the end, or digits directly behind
   the separator, with nothing but spaces behind them. It may begin with
   the separator.

   Ported as it is. Of the fraction the first two digits are meant, and
   they are had like this: the digits are read as one number, without
   the zeros in front; a number below 10 is taken ten times; and then it
   is divided by 10 as often as it is not below 100, 10 or 1, for no
   zero in front of it, one, or more. So ".5" is 50, ".05" is 5, ".005"
   is 0 and ".123" is 12. A separator with spaces behind it is taken,
   "5. ", and one with nothing at all in front and behind is not. */
struct Checked StF04_10(u16 answer)
{
    struct Checked checked;
    const u8 *text, *from;
    u16 number, limit;
    u8 seconds = 0, hundredths = 0, zeros;
    u8 separator = md.Country.DecimalSep[0];

    if ((u8)answer != 0)
        return Accepted(answer);
    text = NEAR(FndLet(md.TmpErr + 3));
    if (*text == 0)
        return Accepted(answer);
    if (*text != separator) {
        from = text;
        text = NEAR(TxtNum(from, (u16 STACK *)&number));
        if (text == from || number > 99)
            goto refused;
        seconds = (u8)number;
        if (*FndLet(text) == 0)
            goto taken;
        if (*text != separator)
            goto refused;
        if (*FndLet(text + 1) == 0)
            goto taken;
    }
    /* F04_11 */
    text++;
    zeros = 0;
    for (from = text; *from == '0'; from++)
        zeros++;
    from = text;
    text = NEAR(TxtNum(from, (u16 STACK *)&number));
    if (text == from)
        goto refused;
    if (*FndLet(text) != 0)
        goto refused;
    if (number < 10)
        number *= 10;
    limit = 100;
    if (zeros != 0) {
        limit = 10;
        if (zeros != 1)
            limit = 1;
    }
    while (number >= limit)
        number /= 10;
    hundredths = (u8)number;
taken:
    md.DelayTm[0] = hundredths;
    md.DelayTm[1] = seconds;
    return Accepted(0);

refused:
    checked.answer = Dialog(OFFSET(StNoTim_B), 0, 0, 0, MAIN_SEG);
    /* The box has no page of the help, and the dialog that goes on has
       its own again. */
    md.HlpPage = PAGE_PANELS;
    checked.refused = 1;
    return checked;
}

/* The capitals of 40 characters, */
static void StF09_10(const u8 __far *from, u8 __far *to)
{
    u16 count = 40;

    do
        *to++ = (u8)UpCase(*from++);
    while (--count);
}

/* and their small letters. */
static void StF09_20(const u8 __far *from, u8 __far *to)
{
    u16 count = 40;

    do
        *to++ = (u8)LowCase(*from++);
    while (--count);
}

/* Country info. The box shows how the country writes things, and the
   lines that do are written here first, into the lines of the box
   themselves: under each of two rows of characters their capitals and
   their small letters; nine in the evening, "9:00p" or "21:00"; a date
   as two letters each for the month, the day and the year, in the
   country's order; and a thousand millions, with the country's
   separator of thousands, a column further on if it has none. */
void StF09_00(void)
{
    u16 code = AsmSeg();
    u8 __far *info = AT(code, OFFSET(StInf02_S));
    u8 __far *line;
    u8 separator, first, second, third;
    u16 count;

    StF09_10(AT(code, OFFSET(StInf05_S) + 4),
             AT(code, OFFSET(StInf06_S) + 7));
    StF09_20(AT(code, OFFSET(StInf05_S) + 4),
             AT(code, OFFSET(StInf07_S) + 7));
    StF09_10(AT(code, OFFSET(StInf09_S) + 7),
             AT(code, OFFSET(StInf10_S) + 7));
    StF09_20(AT(code, OFFSET(StInf09_S) + 7),
             AT(code, OFFSET(StInf11_S) + 7));

    /* The time. */
    separator = md.Country.TimeSep[0];
    line = info + 6;
    if (md.Country.TimeFmt == 0) {
        *line++ = '9';
        *line++ = separator;
        *line++ = '0';
        *line++ = '0';
        *line = 'p';
    } else {
        *line++ = '2';
        *line++ = '1';
        *line++ = separator;
        *line++ = '0';
        *line = '0';
    }

    /* The date. */
    separator = md.Country.DateSep[0];
    first = 'M';
    second = 'D';
    third = 'Y';
    if (md.Country.DateFmt != 0) {
        first = 'D';
        second = 'M';
        if (md.Country.DateFmt != 1) {
            first = 'Y';
            third = 'D';
        }
    }
    line = info + 22;
    *line++ = first;
    *line++ = first;
    *line++ = separator;
    *line++ = second;
    *line++ = second;
    *line++ = separator;
    *line++ = third;
    *line = third;

    /* The number. */
    separator = md.Country.Sep1000[0];
    line = info + 39;
    if (separator == 0)
        line++;
    *line++ = '1';
    for (count = 3; count != 0; count--) {
        *line++ = separator;
        *line++ = '0';
        *line++ = '0';
        *line++ = '0';
    }
    Dialog(OFFSET(StInfo_B), 0, 0, 0, MAIN_SEG);
}
