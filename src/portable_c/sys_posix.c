/*
 * The functions of sys.h for Linux: step 6 of the portable C's road
 * (CLAUDE.md, "The portable C", the road, steps 3 and 6), the piece L3a
 * (TODO.md, "L3a, the program runs on Linux"). The program goes on
 * asking what it asks DOS and the BIOS, and this answers as they would,
 * emulating them on the host at the level of their services.
 *
 * What files, directories and drives are is the emulation of the pieces
 * L1 and L2, files_posix.c with emulate_posix.h: each function of those
 * groups is a line here that calls its function there with the one
 * struct Dos that there is. The rest is here:
 *
 *   - The screen is a grid of cells in the memory, as the display's
 *     memory of a PC is, 80 to a row and 25 rows shown, and the calls of
 *     the BIOS for it answer as the bench's BIOS does (harness/trial32/
 *     asked.py): mode 3, 80 columns, page 0, a VGA with a font of 16
 *     lines whose last row is 24. Nothing of it is drawn in a terminal
 *     yet: that is the piece L4. The console of DOS writes into the same
 *     grid, as DOS has the BIOS do.
 *   - The keys come from the test channel below, as the BIOS's words; a
 *     terminal is the piece L5. No mouse: L5 too.
 *   - The timer's ticks are the host's monotonic clock at DOS's 18.2 to
 *     the second, counted from the time of day when the program started;
 *     the time of day is the host's local time.
 *   - The memory is the C library's, malloc and realloc, with what
 *     sys_dos.c answers: a block refused for want of memory says how long
 *     the largest could be, which is the machine's memory, as the user
 *     decided of the extender's, less what the program holds, and no more
 *     than a count of 32 bits with a sign holds.
 *   - The country is that of the United States with the code page 437,
 *     always, on the user's word of 14 October 2026, as the bench's DOS
 *     has it; the capitals are DOS's of the code page (CodePageUpper).
 *   - The program's tail and its environment are those of the host,
 *     argv and environ, in the code page; behind the environment, where
 *     DOS has the path of the program's file, is the path of the
 *     directory of the setup file (SetupFile of paths_posix.c) with the
 *     program's name, VC, so that the program reads its setup file, its
 *     menu and its extension file there, as the original reads them
 *     beside its file.
 *   - What the system asks the program never comes: the host has no
 *     critical error, and Ctrl-Break and Ctrl-C are keys of a terminal,
 *     L5. Another program run fails for now: that is L6.
 *
 * The drives are R:, the root of the host's file system, and H:, the
 * home directory, as $HOME names it, where that is a directory other
 * than the root (DosBegin of files_posix.c). Where the program starts is
 * the host's current directory, on H: where that is under the home
 * directory and on R: otherwise; where DOS's path of it is longer than
 * DOS holds, the deepest directory above it whose path is not (StartIn).
 */
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "emulate_posix.h"

extern char **environ;

/* DOS's errors that are answered here. */
#define INVALID_FUNCTION 1
#define NO_MEMORY 8
#define NO_BLOCK 9

/* The one DOS that the program has. */
static struct Dos Emulated;

/*
 * Files, files by a file control block, directories and drives: the
 * emulation's, a line each. The Bare functions are the same as the
 * others, since the host has no device that the program is asked about.
 */

struct Answer FileCreate(const u8 *name, u16 attributes)
{
    return DosFileCreate(&Emulated, name, attributes);
}

struct Answer FileOpen(const u8 *name, u16 mode)
{
    return DosFileOpen(&Emulated, name, mode);
}

struct Answer FileOpenBare(const u8 *name, u16 mode)
{
    return DosFileOpen(&Emulated, name, mode);
}

struct Answer FileClose(u16 handle)
{
    return DosFileClose(&Emulated, handle);
}

struct Answer FileCloseBare(u16 handle)
{
    return DosFileClose(&Emulated, handle);
}

struct Moved FileRead(u16 handle, u8 *to, i32 count)
{
    return DosFileRead(&Emulated, handle, to, count);
}

struct Moved FileReadBare(u16 handle, u8 *to, i32 count)
{
    return DosFileRead(&Emulated, handle, to, count);
}

struct Moved FileWrite(u16 handle, const u8 *from, i32 count)
{
    return DosFileWrite(&Emulated, handle, from, count);
}

struct FilePlace FileSeek(u16 handle, u32 place)
{
    return DosFileSeek(&Emulated, handle, place);
}

struct FilePlace FileStep(u16 handle, u16 from, i32 step)
{
    return DosFileStep(&Emulated, handle, from, step);
}

struct Answer FileDelete(const u8 *name)
{
    return DosFileDelete(&Emulated, name);
}

struct Answer FileRename(const u8 *from, const u8 *to)
{
    return DosFileRename(&Emulated, from, to);
}

struct FileAttr FileAttributes(const u8 *name)
{
    return DosFileAttributes(&Emulated, name);
}

struct Answer FileSetAttributes(const u8 *name, u16 attributes)
{
    return DosFileSetAttributes(&Emulated, name, attributes);
}

struct FileStamp FileTime(u16 handle)
{
    return DosFileTime(&Emulated, handle);
}

struct Answer FileSetTime(u16 handle, u16 time, u16 date)
{
    return DosFileSetTime(&Emulated, handle, time, date);
}

struct Device FileDevice(u16 handle)
{
    return DosFileDevice(&Emulated, handle);
}

struct Answer FileTrueName(const u8 *name, u8 *to)
{
    return DosFileTrueName(&Emulated, name, to);
}

u16 FcbParse(const u8 *name, u8 *fcb, u16 how)
{
    return DosFcbParse(&Emulated, name, fcb, how);
}

struct Answer FcbDelete(const u8 *fcb, bool flagged)
{
    return DosFcbDelete(&Emulated, fcb, flagged);
}

void FindRecord(struct DTAs *record)
{
    DosFindRecord(&Emulated, record);
}

struct Answer FindFirst(const u8 *pattern, u16 attributes)
{
    return DosFindFirst(&Emulated, pattern, attributes);
}

struct Answer FindNext(void)
{
    return DosFindNext(&Emulated);
}

struct Answer DirMake(const u8 *name)
{
    return DosDirMake(&Emulated, name);
}

struct Answer DirRemove(const u8 *name)
{
    return DosDirRemove(&Emulated, name);
}

struct Answer DirChange(const u8 *name)
{
    return DosDirChange(&Emulated, name);
}

struct Answer DirCurrent(u16 drive, u8 *to)
{
    return DosDirCurrent(&Emulated, drive, to);
}

struct Answer DirCurrentBare(u16 drive, u8 *to)
{
    return DosDirCurrent(&Emulated, drive, to);
}

u16 DriveCurrent(void)
{
    return DosDriveCurrent(&Emulated);
}

void DriveSelect(u16 drive)
{
    DosDriveSelect(&Emulated, drive);
}

struct Space DriveSpace(u16 drive, bool flagged)
{
    return DosDriveSpace(&Emulated, drive, flagged);
}

struct Answer DriveLetter(u16 drive)
{
    return DosDriveLetter(&Emulated, drive);
}

struct Answer DriveSetLetter(u16 drive)
{
    return DosDriveSetLetter(&Emulated, drive);
}

struct Device DriveRemote(u16 drive)
{
    return DosDriveRemote(&Emulated, drive);
}

struct Answer DriveFixed(u16 drive)
{
    return DosDriveFixed(&Emulated, drive);
}

bool DriveCd(u16 drive)
{
    return DosDriveCd(&Emulated, drive);
}

/*
 * Memory: the C library's. What is kept of each block that the program
 * holds is in a table, in the order in which the blocks were taken, as
 * sys_dos.c keeps it of the extender's.
 */

#define BLOCKS_MOST 8                   /* as many as the table has room
                                           for: the program has three */
#define BYTES_MOST 0x7FFFFFF0           /* the most that a count of 32
                                           bits with a sign holds, in
                                           paragraphs */

struct Held {
    u8 *block;
    i32 bytes;
};

static struct Held Blocks[BLOCKS_MOST];
static i32 BlocksHeld = 0;
static int64_t MemoryAll = 0;               /* the machine's, at the start */

/* How many bytes a block could have that is taken now: the machine's
   memory less what the program holds, and no more than BYTES_MOST. */
static i32 Largest(void)
{
    int64_t left = MemoryAll;
    i32 i;

    for (i = 0; i < BlocksHeld; i++)
        left -= Blocks[i].bytes;
    if (left < 0)
        return 0;
    if (left > BYTES_MOST)
        return BYTES_MOST;
    return (i32)left;
}

/* Which block of the table `block` is: BlocksHeld if it is none. */
static i32 Known(const u8 *block)
{
    i32 i;

    for (i = 0; i < BlocksHeld; i++)
        if (Blocks[i].block == block)
            break;
    return i;
}

static struct Block Refused(u16 error, i32 largest)
{
    struct Block block;

    block.block = 0;
    block.value = error;
    block.failed = true;
    block.largest = largest;
    return block;
}

static struct Block Given(u8 *block, i32 bytes)
{
    struct Block given;

    given.block = block;
    given.value = 0;
    given.failed = false;
    given.largest = bytes;
    return given;
}

struct Block MemTake(i32 bytes)
{
    i32 largest = Largest();
    u8 *block;

    if (bytes <= 0 || bytes > largest || BlocksHeld == BLOCKS_MOST)
        return Refused(NO_MEMORY, largest);
    block = malloc((size_t)bytes);
    if (block == 0)
        return Refused(NO_MEMORY, 0);
    Blocks[BlocksHeld].block = block;
    Blocks[BlocksHeld].bytes = bytes;
    BlocksHeld++;
    return Given(block, bytes);
}

struct Answer MemGive(u8 *block)
{
    struct Answer answer;
    i32 at = Known(block), i;

    answer.value = NO_BLOCK;
    answer.failed = true;
    if (at == BlocksHeld)
        return answer;
    free(Blocks[at].block);
    BlocksHeld--;
    for (i = at; i < BlocksHeld; i++)
        Blocks[i] = Blocks[i + 1];
    answer.value = 0;
    answer.failed = false;
    return answer;
}

/* A block that is not made as long as it is asked answers how long it
   is, which it keeps, as the extender's does in sys_dos.c. */
struct Block MemResize(u8 *block, i32 bytes)
{
    struct Held *held;
    u8 *made;
    i32 at = Known(block);

    if (at == BlocksHeld)
        return Refused(NO_BLOCK, 0);
    held = &Blocks[at];
    if (bytes <= 0 || bytes - held->bytes > Largest())
        return Refused(NO_MEMORY, held->bytes);
    made = realloc(held->block, (size_t)bytes);
    if (made == 0)
        return Refused(NO_MEMORY, held->bytes);
    held->block = made;
    held->bytes = bytes;
    return Given(made, bytes);
}

/* The program's own memory is the host's to give it, and it has nothing
   to make shorter. */
struct Block MemOwn(void)
{
    return Given(0, 0);
}

/* How DOS looks through its memory: nothing of the host's. */
void MemSetSearch(u16 how)
{
    (void)how;
}

void MemSetUpper(bool on)
{
    (void)on;
}

/* Every block that the program still holds, given back as it ends, as
   DOS takes a program's memory back. */
static void GiveAll(void)
{
    while (BlocksHeld > 0)
        MemGive(Blocks[0].block);
}

/*
 * The time, the country, the system itself.
 */

/* The milliseconds since midnight of the local time now. */
static int64_t TodayMs(void)
{
    struct timespec now;
    struct tm local;

    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &local);
    return ((int64_t)local.tm_hour * 3600 + local.tm_min * 60 + local.tm_sec)
           * 1000 + now.tv_nsec / 1000000;
}

struct Time TimeOfDay(void)
{
    struct timespec now;
    struct tm local;
    struct Time time;

    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &local);
    time.clock = (u16)(local.tm_hour << 8 | local.tm_min);
    time.seconds = (u16)(local.tm_sec << 8
                         | (int)(now.tv_nsec / 10000000));
    return time;
}

/* The United States, as the bench's DOS writes its 24 bytes of the
   country (harness/trial32/asked.py): the date as month, day, year with
   -, the time of 12 hours with :, a comma for thousands, a point for the
   decimals, the dollar in front with two digits. The address of DOS's
   routine for capitals, which is one of real mode on the bench, is none:
   CaseMap is that. What DOS does not write it leaves as it is. */
void CountryInfo(struct CNTRY *to)
{
    to->DateFmt = 0;
    memcpy(to->Currency, "$\0\0\0\0", sizeof to->Currency);
    memcpy(to->Sep1000, ",", sizeof to->Sep1000);
    memcpy(to->DecimalSep, ".", sizeof to->DecimalSep);
    memcpy(to->DateSep, "-", sizeof to->DateSep);
    memcpy(to->TimeSep, ":", sizeof to->TimeSep);
    to->CurrencyFmt = 0;
    to->CurrencyDigits = 2;
    to->TimeFmt = 0;
    to->CaseMap = 0;
    memcpy(to->DataSep, ",", sizeof to->DataSep);
}

u16 CaseMap(u16 letter)
{
    return (u16)((letter & 0xFF00) | CodePageUpper((u8)letter));
}

/* What the bench's DOS answers, FreeDOS: 7.10, made by FDh. */
struct SysVersion SystemVersion(void)
{
    struct SysVersion version;

    version.version = 0x0A07;
    version.other = 0xFD00;
    return version;
}

struct SysVersion SystemTrueVersion(void)
{
    struct SysVersion version;

    version.version = 0x0A07;
    version.other = 0x3306;             /* AX as DOS leaves it */
    return version;
}

/* Kept, and nothing looks for Ctrl-C by it: the bench has it on. */
static bool BreakOn = true;

bool BreakCheck(void)
{
    return BreakOn;
}

void SetBreakCheck(bool on)
{
    BreakOn = on;
}

/* The milliseconds of the host's monotonic clock since the program
   started. */
static struct timespec StartedAt;

static int64_t Elapsed(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return ((int64_t)now.tv_sec - StartedAt.tv_sec) * 1000
           + (now.tv_nsec - StartedAt.tv_nsec) / 1000000;
}

static int64_t StartMs = 0;             /* since midnight, at the start */

/*
 * The screen: the display's memory, 32 KB as a VGA has it for text, of
 * which a page of 80 by 25 cells is shown, and what the BIOS keeps of it.
 */

#define COLUMNS 80
#define ROWS 25
#define CELLS_ALL 0x4000                /* the display's memory, in cells */
#define BLANK 0x0720                    /* a space, grey on black */
#define CURSOR_SHAPE 0x0607             /* the cursor of a mode of colour */

static u16 Display[CELLS_ALL];
static i32 CellsStart = 0;              /* where ScreenPut and ScreenGet
                                           begin, of ScreenMemory */
static u16 Mode = 3;
static u16 CursorPlace = 0;             /* the row above the column */
static u16 Shape = CURSOR_SHAPE;

/* A stretch of cells that the program reads or writes in the display's
   memory: nothing on DOS stops one that goes beyond, which would be in
   another page or in nothing. Here that is the program's fault, and it
   ends. */
static u16 *Cells(i32 cell, i32 count)
{
    if (cell < 0 || count < 0 || cell > CELLS_ALL - count) {
        fprintf(stderr, "vc: cells %d to %d of the screen, which has %d\n",
                cell, cell + count, CELLS_ALL);
        abort();
    }
    return Display + cell;
}

static i32 Row(void)
{
    return CursorPlace >> 8;
}

static i32 Column(void)
{
    return CursorPlace & 0xFF;
}

static void Place(i32 row, i32 column)
{
    CursorPlace = (u16)(row << 8 | column);
}

struct Screen ScreenState(void)
{
    struct Screen screen;

    screen.mode = Mode;
    screen.columns = COLUMNS;
    screen.page = 0;
    return screen;
}

/* A mode with its highest bit set leaves the cells as they are. */
void ScreenSetMode(u16 mode)
{
    i32 i;

    Mode = mode;
    if ((mode & 0x80) == 0)
        for (i = 0; i < CELLS_ALL; i++)
            Display[i] = BLANK;
    CursorPlace = 0;
    Shape = CURSOR_SHAPE;
}

struct CursorAt CursorWhere(u16 page)
{
    struct CursorAt cursor;

    (void)page;
    cursor.place = CursorPlace;
    cursor.shape = Shape;
    return cursor;
}

void CursorTo(u16 page, u16 place)
{
    (void)page;
    CursorPlace = place;
}

void CursorShape(u16 shape)
{
    Shape = shape;
}

void ScreenScroll(bool down, i32 lines, u16 color, u16 upper, u16 lower)
{
    i32 top = upper >> 8, left = upper & 0xFF;
    i32 bottom = lower >> 8, right = lower & 0xFF;
    i32 height, row, column, from;
    u16 blank = (u16)((color & 0xFF) << 8 | ' ');

    if (bottom >= ROWS)
        bottom = ROWS - 1;
    if (right >= COLUMNS)
        right = COLUMNS - 1;
    if (top > bottom || left > right)
        return;
    height = bottom - top + 1;
    if (lines <= 0 || lines > height)
        lines = height;
    for (row = 0; row < height; row++) {
        /* The rows from the far end first, so that none is written
           before it has been moved. */
        i32 to = down ? bottom - row : top + row;

        from = down ? to - lines : to + lines;
        for (column = left; column <= right; column++)
            Display[to * COLUMNS + column] =
                from >= top && from <= bottom
                    ? Display[from * COLUMNS + column] : blank;
    }
}

u16 ScreenCell(u16 page)
{
    (void)page;
    return Display[Row() * COLUMNS + Column()];
}

void ScreenFill(u16 page, u16 character, u16 color, i32 count)
{
    u16 cell = (u16)((color & 0xFF) << 8 | (character & 0xFF));
    i32 at = Row() * COLUMNS + Column(), i;

    (void)page;
    if (count > CELLS_ALL - at)
        count = CELLS_ALL - at;
    for (i = 0; i < count; i++)
        Display[at + i] = cell;
}

/* The lines move up a row when the cursor goes beyond the last, the
   new one in the colour of the cell at the cursor, as the BIOS has it. */
static void LineFeed(void)
{
    if (Row() < ROWS - 1) {
        Place(Row() + 1, Column());
        return;
    }
    ScreenScroll(false, 1, (u16)(ScreenCell(0) >> 8), 0,
                 (ROWS - 1) << 8 | (COLUMNS - 1));
}

/* What the BIOS does with a character that it types: a carriage return,
   a line feed, a step back and the bell are no characters; any other is
   written in the colour that the cell has, and the cursor goes on, to
   the next line behind the last column. */
void ScreenType(u16 character, u16 page, u16 color)
{
    u16 *cell;
    u8 c = (u8)character;

    (void)page;
    (void)color;
    switch (c) {
    case '\r':
        Place(Row(), 0);
        return;
    case '\n':
        LineFeed();
        return;
    case '\b':
        if (Column() > 0)
            Place(Row(), Column() - 1);
        return;
    case 7:
        return;
    default:
        break;
    }
    cell = &Display[Row() * COLUMNS + Column()];
    *cell = (u16)((*cell & 0xFF00) | c);
    if (Column() < COLUMNS - 1) {
        Place(Row(), Column() + 1);
        return;
    }
    Place(Row(), 0);
    LineFeed();
}

bool ScreenEga(void)
{
    return true;
}

struct Font ScreenFont(void)
{
    struct Font font;

    font.height = 16;
    font.last = ROWS - 1;
    return font;
}

u16 ScreenPageStart(void)
{
    return 0;
}

/* One memory for every mode: the start is in bytes. */
void ScreenMemory(u16 mode, u16 start)
{
    (void)mode;
    CellsStart = start / 2;
}

void ScreenPut(i32 cell, const u16 *from, i32 count)
{
    memcpy(Cells(CellsStart + cell, count), from,
           (size_t)count * sizeof *from);
}

void ScreenGet(i32 cell, u16 *to, i32 count)
{
    memcpy(to, Cells(CellsStart + cell, count),
           (size_t)count * sizeof *to);
}

/*
 * The console: DOS's, which writes as the BIOS types, with a tab made
 * blanks as far as the next column of eight.
 */

void ConsoleChar(u16 character)
{
    if ((character & 0xFF) != '\t') {
        ScreenType(character, 0, 0);
        return;
    }
    do
        ScreenType(' ', 0, 0);
    while (Column() % 8 != 0);
}

void ConsoleText(const u8 *text)
{
    for (; *text != '$'; text++)
        ConsoleChar(*text);
}

/*
 * The test channel. Told so by the variable VC_TEST of the environment,
 * which names a directory of the host, the program takes its keys from
 * the file `keys` there and writes its screens to `screens` there, made
 * anew. It stands where the driver of the tests stands on the bench,
 * harness/DRIVER.ASM, and does what that does, but from inside: a key
 * is due when the program is idle and none waits, one at a time, and a
 * step has ended when the program has gone round idle twice with nothing
 * left to take and nothing that it does by itself after a while, AutoTim
 * of the main data. Then the screen is written.
 *
 * `keys` is text, a line for each step, each event of a step a word of
 * the line, in the order in which they are due:
 *
 *     XXXX     a key, the BIOS's word in four hex digits: the scan code
 *              above the character
 *     held=XX  what KeysHeld answers from then on, two hex digits
 *     wait=N   nothing until the timer has ticked N times more
 *
 * A line that begins with # is none. Every other line is a step, an
 * empty one too, which has the program go round and its screen written.
 *
 * `screens` is a record of 4016 bytes for each screen: four letters,
 * "step" for a step that has ended or "end " where the program ends; a
 * number of 32 bits, low byte first, the step's, counted from 1, or the
 * code that the program ends with; the 2000 cells of the page that is
 * shown, each a character and its colour; and the column and the row of
 * the cursor, the first and the last line of its shape, the mode, and
 * three zeros. The cursor's and the mode's bytes are as the bench's
 * BIOS has them in its data, which is what the harness reads there.
 *
 * Where the steps are all done and the program waits for a key all the
 * same, it ends with 2, and says so.
 */

#define TEST_VARIABLE "VC_TEST"
#define RECORD_KIND 4

enum Event { EVENT_KEY, EVENT_HELD, EVENT_WAIT };

struct Due {
    enum Event event;
    u32 value;
};

struct Step {
    i32 first;                          /* its first event */
    i32 count;
};

static struct Due *Events = 0;
static i32 EventsAll = 0;
static i32 EventsRoom = 0;
static struct Step *Steps = 0;
static i32 StepsAll = 0;
static i32 StepsRoom = 0;
static i32 StepNow = 0;                 /* the step that is going on */
static i32 EventNow = 0;                /* its next event, of its own */
static int ScreensFd = -1;
static bool KeyThere = false;           /* a key waits: the BIOS's buffer */
static u16 KeyWord = 0;
static u16 Held = 0;
static bool Waiting = false;            /* for the timer, by wait= */
static int64_t WaitUntil = 0;           /* by Elapsed */
static i32 IdleRounds = 0;              /* with nothing left to take */
static bool AutoSeen = false;           /* when AutoTim was first seen */
static int64_t AutoSince = 0;

/* A wait of the program's that is no busy one: a millisecond. */
static void Pause(void)
{
    struct timespec pause = { 0, 1000000 };

    nanosleep(&pause, 0);
}

static void Fail(const char *what)
{
    fprintf(stderr, "vc: %s: %s\n", what, strerror(errno));
    exit(1);
}

static void Write(int fd, const u8 *from, size_t count)
{
    ssize_t written;

    while (count > 0) {
        written = write(fd, from, count);
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            Fail("the test's screens");
        from += written;
        count -= (size_t)written;
    }
}

/* A record of the screen as it is. */
static void Record(const char *kind, u32 number)
{
    u8 record[RECORD_KIND + 4 + COLUMNS * ROWS * 2 + 8];
    u8 *at = record;
    i32 i;

    if (ScreensFd < 0)
        return;
    memcpy(at, kind, RECORD_KIND);
    at += RECORD_KIND;
    for (i = 0; i < 4; i++)
        *at++ = (u8)(number >> 8 * i);
    for (i = 0; i < COLUMNS * ROWS; i++) {
        *at++ = (u8)Display[i];
        *at++ = (u8)(Display[i] >> 8);
    }
    *at++ = (u8)Column();
    *at++ = (u8)Row();
    *at++ = (u8)(Shape >> 8);
    *at++ = (u8)Shape;
    *at++ = (u8)Mode;
    for (i = 0; i < 3; i++)
        *at++ = 0;
    Write(ScreensFd, record, sizeof record);
}

/* A word of a line of `keys` made an event. False if it is none. */
static bool Event(const char *word, struct Due *due)
{
    char *end;
    unsigned long value;

    due->event = EVENT_KEY;
    if (strncmp(word, "held=", 5) == 0) {
        due->event = EVENT_HELD;
        word += 5;
    } else if (strncmp(word, "wait=", 5) == 0) {
        due->event = EVENT_WAIT;
        value = strtoul(word + 5, &end, 10);
        due->value = (u32)value;
        return end != word + 5 && *end == 0 && value <= 0xFFFFFF;
    }
    value = strtoul(word, &end, 16);
    due->value = (u32)value;
    return end != word && *end == 0 && value <= 0xFFFF
           && (due->event != EVENT_HELD || value <= 0xFF);
}

/* An event more, of the last step; and a step more, without events. */
static void AddEvent(struct Due due)
{
    struct Due *more;

    if (EventsAll == EventsRoom) {
        EventsRoom = EventsRoom * 2 + 64;
        more = realloc(Events, (size_t)EventsRoom * sizeof *Events);
        if (more == 0)
            Fail("the test's keys");
        Events = more;
    }
    Events[EventsAll++] = due;
    Steps[StepsAll - 1].count++;
}

static void AddStep(void)
{
    struct Step *more;

    if (StepsAll == StepsRoom) {
        StepsRoom = StepsRoom * 2 + 64;
        more = realloc(Steps, (size_t)StepsRoom * sizeof *Steps);
        if (more == 0)
            Fail("the test's keys");
        Steps = more;
    }
    Steps[StepsAll].first = EventsAll;
    Steps[StepsAll].count = 0;
    StepsAll++;
}

/* A line of `keys`, of the file at `path`. */
static void ReadLine(const char *path, char *line)
{
    char *word, *rest;
    struct Due due;

    if (*line == '#')
        return;
    AddStep();
    for (word = strtok_r(line, " \t\r", &rest); word != 0;
         word = strtok_r(0, " \t\r", &rest)) {
        if (!Event(word, &due)) {
            fprintf(stderr, "vc: %s: no event: %s\n", path, word);
            exit(1);
        }
        AddEvent(due);
    }
}

/* Read the steps of `keys`. */
static void ReadKeys(const char *path)
{
    FILE *file = fopen(path, "r");
    char line[4096];
    size_t length = 0;
    int c;

    if (file == 0)
        Fail(path);
    while ((c = getc(file)) != EOF) {
        if (c == '\n') {
            line[length] = 0;
            ReadLine(path, line);
            length = 0;
        } else if (length < sizeof line - 1) {
            line[length++] = (char)c;
        } else {
            fprintf(stderr, "vc: %s: a line is too long\n", path);
            exit(1);
        }
    }
    if (ferror(file))
        Fail(path);
    if (length > 0) {
        line[length] = 0;
        ReadLine(path, line);
    }
    fclose(file);
}

/* Begin the channel, from the directory that VC_TEST names. False where
   the variable is not there. */
static bool TestBegin(void)
{
    const char *directory = getenv(TEST_VARIABLE);
    char path[4096];

    if (directory == 0)
        return false;
    snprintf(path, sizeof path, "%s/keys", directory);
    ReadKeys(path);
    snprintf(path, sizeof path, "%s/screens", directory);
    ScreensFd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (ScreensFd < 0)
        Fail(path);
    return true;
}

/* Whether the program has said that it does something by itself at a
   time that has not come: AutoTim, the hour and the minute in its low
   word and the second above, or FFFFh there for none. Believed for 2
   seconds from when it is first seen, as the driver believes it. */
static bool AutoPending(void)
{
    u16 clock = (u16)md.AutoTim, seconds = (u16)(md.AutoTim >> 16);
    struct Time now;

    if (clock == 0xFFFF) {
        AutoSeen = false;
        return false;
    }
    now = TimeOfDay();
    if (now.clock > clock || (now.clock == clock && now.seconds >= seconds))
        return false;
    if (!AutoSeen) {
        AutoSeen = true;
        AutoSince = Elapsed();
    }
    return Elapsed() - AutoSince < 2000;
}

/* The program is idle with no key waiting: the next event is due, or
   the step ends. */
static void Due(void)
{
    struct Due *due;

    if (KeyThere)
        return;
    if (Waiting) {
        if (Elapsed() < WaitUntil) {
            Pause();
            return;
        }
        Waiting = false;
    }
    while (StepNow < StepsAll && EventNow < Steps[StepNow].count) {
        due = &Events[Steps[StepNow].first + EventNow++];
        IdleRounds = 0;
        switch (due->event) {
        case EVENT_KEY:
            KeyWord = (u16)due->value;
            KeyThere = true;
            return;
        case EVENT_HELD:
            Held = (u16)due->value;
            break;
        case EVENT_WAIT:
            Waiting = true;
            WaitUntil = Elapsed() + (int64_t)due->value * 65536000 / 1193180;
            return;
        }
    }
    if (StepNow == StepsAll) {
        fprintf(stderr, "vc: the test's keys have run out\n");
        Record("end ", 2);
        GiveAll();
        DosEnd(&Emulated);
        exit(2);
    }
    if (AutoPending()) {
        Pause();
        return;
    }
    if (++IdleRounds < 2)
        return;
    IdleRounds = 0;
    StepNow++;
    EventNow = 0;
    Record("step", (u32)StepNow);
}

void Idle(void)
{
    Due();
}

/*
 * The keyboard: what the test channel has made due. KeyRead waits for a
 * key, which is idle as Idle is.
 */

struct Keys KeyWaiting(void)
{
    struct Keys keys;

    keys.key = KeyThere ? KeyWord : 0;
    keys.none = !KeyThere;
    return keys;
}

u16 KeyRead(void)
{
    while (!KeyThere)
        Due();
    KeyThere = false;
    return KeyWord;
}

u16 KeysHeld(void)
{
    return Held;
}

void KeyForgetIns(void)
{
    Held &= 0x7F;
}

/*
 * The mouse: there is none, as a driver that does not answer says.
 */

u16 MouseFind(void)
{
    return 0;
}

u16 MouseReset(void)
{
    return 0;
}

void MouseShow(void)
{
}

void MouseHide(void)
{
}

struct MouseAt MouseWhere(void)
{
    struct MouseAt at;

    at.buttons = 0;
    at.x = 0;
    at.y = 0;
    return at;
}

void MouseTo(u16 x, u16 y)
{
    (void)x;
    (void)y;
}

void MouseRows(u16 first, u16 last)
{
    (void)first;
    (void)last;
}

void MousePointer(u16 keep, u16 turn)
{
    (void)keep;
    (void)turn;
}

/*
 * The timer: the BIOS's ticks since midnight, 1,193,180 / 65,536 to the
 * second, of the time of day when the program started and the host's
 * monotonic clock since; the BIOS counts 1800B0h of them to a day.
 */

#define TICKS_A_DAY 0x1800B0
#define MS_A_DAY (24 * 3600 * 1000)

u32 Ticks(void)
{
    return (u32)((StartMs + Elapsed()) % MS_A_DAY * 1193180 / 65536000
                 % TICKS_A_DAY);
}

/*
 * Other programs, and the end of this one.
 */

/* Not yet: the piece L6. */
struct Answer ProgramRun(const u8 *name, const u8 *tail)
{
    struct Answer answer;

    (void)name;
    (void)tail;
    answer.value = INVALID_FUNCTION;
    answer.failed = true;
    return answer;
}

struct Answer ProgramRunBare(const u8 *name, const u8 *tail)
{
    return ProgramRun(name, tail);
}

void ProgramEnd(u16 code)
{
    Record("end ", code & 0xFF);
    GiveAll();
    DosEnd(&Emulated);
    exit(code & 0xFF);
}

/* A system that cannot be that old. */
void ProgramEndOld(void)
{
}

/* The tail, as DOS has it in the program's prefix from 81h: the
   arguments, each behind a blank, in the code page, at most 126
   characters, and a carriage return. */
#define TAIL_MOST 126

static u8 Tail[TAIL_MOST + 1];

const u8 *ProgramTail(void)
{
    return Tail;
}

/* The environment, as DOS has one: at most 32 KB of texts, then a zero,
   two bytes, the path of the program's file, and room behind it, since
   start.c reads 68 bytes of that path whatever it is. */
#define ENVIRONMENT_MOST 0x8000
#define PATH_READ 68

static u8 *Environment = 0;

const u8 *ProgramEnvironment(void)
{
    return Environment;
}

/* Of a text of the host, in UTF-8 by custom, as many bytes of the code
   page as there is room for at `to`, a character that the code page has
   not, or a byte that is no UTF-8, being _. Answers how many. */
static i32 InCodePage(const char *text, u8 *to, i32 room)
{
    i32 length = (i32)strlen(text), made = 0, at = 0, step, byte;
    u32 character;

    while (at < length && made < room) {
        step = Utf8Decode(text + at, length - at, &character);
        byte = '_';
        if (step == 0)
            step = 1;
        else if (CodePageFromUnicode(character) >= 0)
            byte = CodePageFromUnicode(character);
        to[made++] = (u8)byte;
        at += step;
    }
    return made;
}

static void TailOf(int count, char **arguments)
{
    i32 length = 0;
    int i;

    for (i = 1; i < count && length < TAIL_MOST; i++) {
        Tail[length++] = ' ';
        length += InCodePage(arguments[i], Tail + length,
                             TAIL_MOST - length);
    }
    Tail[length] = '\r';
}

/* The environment of the host, a text after another as far as they fit
   with the zero behind the last, and the program's path behind them. */
static void EnvironmentOf(const u8 *program)
{
    size_t length = strlen((const char *)program);
    i32 at = 0, room = ENVIRONMENT_MOST - 1, made;
    char **text;

    Environment = calloc(ENVIRONMENT_MOST + 2 + length + PATH_READ, 1);
    if (Environment == 0)
        Fail("the environment");
    for (text = environ; *text != 0; text++) {
        made = InCodePage(*text, Environment + at, room - at - 1);
        if (made < (i32)strlen(*text) && at + made + 1 >= room) {
            memset(Environment + at, 0, (size_t)made);
            break;
        }
        at += made + 1;
    }
    Environment[at++] = 0;
    Environment[at++] = 1;              /* one string behind it */
    Environment[at++] = 0;
    memcpy(Environment + at, program, length);
}

/* Each directory of `path` that the host has not, made, as the XDG Base
   Directory Specification has it made, for the owner alone. */
static void MakeDirectories(char *path)
{
    char *slash;

    for (slash = strchr(path + 1, '/'); slash != 0;
         slash = strchr(slash + 1, '/')) {
        *slash = 0;
        if (mkdir(path, 0700) != 0 && errno != EEXIST)
            Fail(path);
        *slash = '/';
    }
    if (mkdir(path, 0700) != 0 && errno != EEXIST)
        Fail(path);
}

/* The path of the program's file, as DOS has it behind the environment:
   VC in the directory of the setup file, which is made where the host
   has it not. */
static void ProgramPath(u8 *to)
{
    char file[4096], real[PATH_MAX];
    char *slash;
    struct Answer made;

    if (!SetupFile(getenv("XDG_CONFIG_HOME"), getenv("HOME"), file,
                   (i32)sizeof file)) {
        fprintf(stderr, "vc: no directory for the setup file: HOME is not "
                        "set\n");
        exit(1);
    }
    slash = strrchr(file, '/');
    *slash = 0;
    MakeDirectories(file);
    if (realpath(file, real) == 0)
        Fail(file);
    made = DosPathOfHost(&Emulated.directories, &Emulated.drives, real, to);
    if (made.failed) {
        fprintf(stderr, "vc: the directory of the setup file, %s, is not "
                        "one that DOS can name\n", real);
        exit(1);
    }
    memcpy(to + strlen((const char *)to), "\\VC", sizeof "\\VC");
}

/*
 * What the system asks the program: nothing, here. The host has no
 * critical error, and the keys that stop a program are a terminal's.
 */

void DeviceErrorNote(void)
{
}

void DeviceErrorTake(DeviceErrorFn *ask)
{
    (void)ask;
}

void DeviceErrorGive(void)
{
}

void IntOn(void)
{
}

void CtrlBreakNote(void)
{
}

void CtrlBreakTake(void)
{
}

void CtrlBreakGive(void)
{
}

void CtrlCTake(void)
{
}

void ProgramEndedBreakCheck(bool on)
{
    (void)on;
}

void ProgramEndedTake(void)
{
}

void ProgramEndedGive(void)
{
}

/*
 * Where the host starts the program.
 */

/* The host's current directory made the current drive's, H: under the
   home directory and R: elsewhere, or where DOS cannot name that, for a
   path that is longer than DOS holds, the deepest directory above it
   that DOS can: the home directory at the least on H:, which is H:\,
   and the root on R:. DOS's 67 characters of a path stay, on the user's
   word of 15 October 2026 (CLAUDE.md, step 6). */
static void StartIn(char *here)
{
    char *slash;

    while (DosStartIn(&Emulated, here).failed) {
        slash = strrchr(here, '/');
        if (slash == 0 || slash == here) {
            DosStartIn(&Emulated, "/");
            return;
        }
        *slash = 0;
    }
}

int main(int count, char **arguments)
{
    char here[4096];
    u8 program[DOS_PATH_ROOM + 4];
    long pages = sysconf(_SC_PHYS_PAGES), page = sysconf(_SC_PAGESIZE);

    if (!TestBegin()) {
        fprintf(stderr, "vc: there is no terminal yet: the program runs "
                        "with %s set, for its tests\n", TEST_VARIABLE);
        return 1;
    }
    MemoryAll = pages > 0 && page > 0 ? (int64_t)pages * page : BYTES_MOST;
    StartMs = TodayMs();
    clock_gettime(CLOCK_MONOTONIC, &StartedAt);
    DosBegin(&Emulated, getenv("HOME"));
    if (getcwd(here, sizeof here) == 0)
        Fail("the current directory");
    StartIn(here);
    ProgramPath(program);
    EnvironmentOf(program);
    TailOf(count, arguments);
    ScreenSetMode(3);
    Main();
    return 0;
}
