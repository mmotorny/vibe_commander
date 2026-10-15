/*
 * Numbers, sizes, dates and times as text.
 *
 * Translated from VCSUB1.INC (Number) and VCSUB2.INC (the rest).
 */
#include "vc.h"

/* The country's date formats; anything above is year, month, day. */
#define DATE_MDY 0
#define DATE_DMY 1

#define TIME_12_HOURS 0                 /* the country's time format */

/* A date as PutDate() takes it apart. Each field is two characters, the
   first in the low byte, as PUT16() writes them. The assembly's PutDate
   returns this in AX, BX, DX and CX. */
struct DateText {
    u16 century;
    u16 first, second, third;           /* in the country's order */
};

/* A time as PutTime() takes it apart: BX, DX, AX and CH in the assembly. */
struct TimeText {
    u16 hour, minute, second;
    u8  unused;
    u8  suffix;                         /* 'a' or 'p', or a space */
};

u16 HexDec(i32 number);
void PutDate(u16 date, struct DateText *text);
u8 *BufDate(const struct DateText *text, u8 *buffer);
void PutTime(u16 time, struct TimeText *text);
u8 *BufTime(const struct TimeText *text, u8 *buffer);

/* Number and SzDtTm are in vc.h. */

static const u32 PowersOfTen[9] = {
    1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL,
    1000UL, 100UL, 10UL
};

/* What the size column shows for a directory: the parent, and any other. */
static const u8 DirMarker[2][9] = {
    {0x10, 'U', 'P', '-', '-', 'D', 'I', 'R', 0x11},
    {0x10, 'S', 'U', 'B', '-', 'D', 'I', 'R', 0x11}
};

/* The two decimal digits of a number below 100, as characters: the tens in
   the low byte. Only the low byte of `number` counts. */
u16 HexDec(i32 number)
{
    i32 tens;

    number &= 0xFF;
    tens = number / 10;
    return (u16)(tens + '0' + ((number - tens * 10 + '0') << 8));
}

/* Write a zero or a separator of a number. Before the first digit there is
   none; in a padded number a space takes its place. */
static u8 *PutFill(u8 *buffer, u16 character, bool started, u16 mode)
{
    if (started)
        *buffer++ = (u8)character;
    else if (mode & NUM_PADDED)
        *buffer++ = ' ';
    return buffer;
}

/* Write a number, given as its two halves, in decimal to `buffer` and end
   it with a zero byte. Returns the address of that zero. Only the low byte
   of `mode` counts.

   With NUM_SEPARATED the digits are grouped in threes by the country's
   separator. With NUM_PADDED the number is right-aligned in a field as
   wide as the longest number, 10 digits and any separators. A country
   without a separator gets none; padded, the field is as wide all the
   same. */
u8 *Number(u8 *buffer, u16 high, u16 low, u16 mode)
{
    u32 number = (u32)high << 16 | low;
    bool started = false;               /* a digit has been written */
    i32 i;
    u16 digit;

    mode &= 0xFF;
    if (md.Country.Sep1000[0] == 0 && (mode & NUM_SEPARATED)) {
        mode &= ~NUM_SEPARATED;
        if (mode) {
            *buffer++ = ' ';
            *buffer++ = ' ';
            *buffer++ = ' ';
        }
    }
    for (i = 0; i < 9; i++) {
        digit = '0';
        while (number >= PowersOfTen[i]) {
            number -= PowersOfTen[i];
            digit++;
        }
        if (digit != '0') {
            started = true;
            *buffer++ = (u8)digit;
        } else {
            buffer = PutFill(buffer, '0', started, mode);
        }
        /* After the billions, the millions and the thousands. */
        if ((mode & NUM_SEPARATED) && (i == 0 || i == 3 || i == 6))
            buffer = PutFill(buffer, md.Country.Sep1000[0], started, mode);
    }
    *buffer++ = '0' + (u8)number;
    *buffer = 0;
    return buffer;
}

/* Take a DOS date apart into `text`, in the country's order. Where the
   year does not come first, a leading zero of the first field becomes a
   space. */
void PutDate(u16 date, struct DateText *text)
{
    u16 day = HexDec(date & 0x1F);
    u16 month = HexDec(date >> 5 & 0x0F);
    i32 year = (date >> 9 & 0x7F) + 80;         /* since 1900 */
    u16 year2 = HexDec(year % 100);

    text->century = HexDec(year / 100 + 19);
    text->second = month;
    if (md.Country.DateFmt > DATE_DMY) {
        text->first = year2;
        text->third = day;
        return;
    }
    text->third = year2;
    if (md.Country.DateFmt == DATE_MDY) {
        text->first = month;
        text->second = day;
    } else {
        text->first = day;
    }
    if ((u8)text->first == '0')
        *(u8 *)&text->first = ' ';
}

/* Write a date from PutDate() to `buffer`, with the century: 10 characters.
   Returns the address behind them. */
u8 *BufDate(const struct DateText *text, u8 *buffer)
{
    if (md.Country.DateFmt > DATE_DMY)
        PUT16(buffer, text->century);
    PUT16(buffer, text->first);
    *buffer++ = md.Country.DateSep[0];
    PUT16(buffer, text->second);
    *buffer++ = md.Country.DateSep[0];
    if (md.Country.DateFmt <= DATE_DMY)
        PUT16(buffer, text->century);
    PUT16(buffer, text->third);
    return buffer;
}

/* Take a DOS time apart into `text`. A leading zero of the hour becomes a
   space. A country with a 12-hour clock gets a suffix, 'a' or 'p'. */
void PutTime(u16 time, struct TimeText *text)
{
    i32 hour = time >> 11;

    text->second = HexDec((time & 0x1F) * 2);
    text->minute = HexDec(time >> 5 & 0x3F);
    text->suffix = ' ';
    if (md.Country.TimeFmt == TIME_12_HOURS) {
        text->suffix = 'a';
        if (hour >= 12)
            text->suffix = 'p';
        hour %= 12;
        if (hour == 0)
            hour = 12;
    }
    text->hour = HexDec(hour);
    if ((u8)text->hour == '0')
        *(u8 *)&text->hour = ' ';
}

/* Write a time from PutTime() to `buffer`, with seconds and the suffix:
   9 characters. Returns the address behind them. */
u8 *BufTime(const struct TimeText *text, u8 *buffer)
{
    PUT16(buffer, text->hour);
    *buffer++ = md.Country.TimeSep[0];
    PUT16(buffer, text->minute);
    *buffer++ = md.Country.TimeSep[0];
    PUT16(buffer, text->second);
    *buffer++ = text->suffix;
    return buffer;
}

/* The routines below write to the screen buffer: a word for each cell,
   the character in its low byte and the colour in its high byte. They
   take the colour as the assembly passes it, in the high byte of `color`
   with a low byte of any value. */

/* Put the two characters of `pair` on the screen. */
static u16 *PutPair(u16 *screen, u16 pair, u16 color)
{
    color &= 0xFF00;
    *screen++ = color | (pair & 0xFF);
    *screen++ = color | pair >> 8;
    return screen;
}

/* Put a file's size on the screen, in 9 cells. A directory shows a marker
   instead, unless its panel, `wcb`, has had the sizes of directories
   worked out. The assembly has the panel in DS, where the file is; here
   it is handed on, since a pointer to a file says which panel it is in
   only where it has a segment. A size of 10 digits shows as nine nines. */
static u16 *PutSize(const struct WCBdef *wcb,
                    const struct FILENT *file, u16 *screen, u16 color)
{
    i32 i, marker;

    color &= 0xFF00;
    if (file->AttrF & ATTR_DIRECTORY) {
        marker = file->NameF[0] != '.';
        if (!marker || wcb->SizDir == 0) {
            for (i = 0; i < 9; i++)
                *screen++ = color | DirMarker[marker][i];
            return screen;
        }
    }
    Number(md.TmpBuf, (u16)(file->SizeF >> 16), (u16)file->SizeF, NUM_PADDED);
    if (md.TmpBuf[0] != ' ')
        for (i = 0; i < 10; i++)
            md.TmpBuf[i] = '9';
    for (i = 1; i < 10; i++)
        *screen++ = color | md.TmpBuf[i];
    return screen;
}

/* Put a file's date on the screen, without the century: 8 cells. */
static u16 *PutDat0(const struct FILENT *file, u16 *screen, u16 color)
{
    struct DateText text;
    u16 separator = (color & 0xFF00) | md.Country.DateSep[0];

    PutDate(file->DateF, &text);
    screen = PutPair(screen, text.first, color);
    *screen++ = separator;
    screen = PutPair(screen, text.second, color);
    *screen++ = separator;
    return PutPair(screen, text.third, color);
}

/* Put a file's time on the screen, without the seconds: 6 cells. The
   suffix of a 12-hour clock takes the place of a leading space. */
static u16 *PutTim0(const struct FILENT *file, u16 *screen, u16 color)
{
    struct TimeText text;

    color &= 0xFF00;
    PutTime(file->TimeF, &text);
    if (text.suffix == ' ')
        *screen++ = color | ' ';
    screen = PutPair(screen, text.hour, color);
    *screen++ = color | md.Country.TimeSep[0];
    screen = PutPair(screen, text.minute, color);
    if (text.suffix != ' ')
        *screen++ = color | text.suffix;
    return screen;
}

/* Colour the cell at `screen`, which separates two columns and keeps its
   character. */
#define PutGap(screen, color) \
    (((u8 *)(screen))[1] = (u8)((color) >> 8), (screen) + 1)

/* Put the size, date and time of a file of the panel `wcb` on the screen,
   each after a cell that separates the columns: 26 cells. Returns the
   address behind them. */
u16 *SzDtTm(const struct WCBdef *wcb, const struct FILENT *file,
            u16 *screen, u16 color)
{
    screen = PutGap(screen, color);
    screen = PutSize(wcb, file, screen, color);
    screen = PutGap(screen, color);
    screen = PutDat0(file, screen, color);
    screen = PutGap(screen, color);
    return PutTim0(file, screen, color);
}
