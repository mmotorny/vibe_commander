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
 *     lines whose last row is 24. The console of DOS writes into the
 *     same grid, as DOS has the BIOS do. The grid is drawn in the
 *     terminal that the program is started in, term_posix.c, the piece
 *     L4, but where the test channel below is told to drive it.
 *   - The keys come from the terminal, made the BIOS's words there, by
 *     the kitty protocol of the keyboard where it has that, with Shift,
 *     Ctrl and Alt held; or from the test channel.
 *   - The mouse is the terminal's, or the test channel's, and answers as
 *     the bench's mouse driver does, with its pointer drawn in the grid,
 *     a cell whose colours are turned round: "The mouse" below.
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
 *     L5.
 *   - A program run is a command of the host's shell, $SHELL -c, which
 *     COMSPEC names, on the terminal's own screen, which Ctrl-O shows:
 *     "A command" below, the piece L6.
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
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

static bool TestClock(void);
static struct Time TestTime(void);

struct Time TimeOfDay(void)
{
    struct timespec now;
    struct tm local;
    struct Time time;

    if (TestClock())
        return TestTime();
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

/*
 * The mouse's pointer, as the bench's mouse driver draws it in the text
 * mode: the cell that it is on kept, and in its place the cell with the
 * bits of PointerKeep kept and those of PointerTurn turned round, while
 * the pointer is shown. The driver puts the cell back as it kept it when
 * the pointer moves or is hidden, where the cell still has what it drew
 * there: one that has been written over is left as it is, as CuteMouse's
 * notes say of it (DOC/CTMOUSE/TECHNOTE.TXT of its package), and as the
 * screen shows where the BIOS moves the lines up under the pointer for an
 * error's beep (view-keys). Places are in dots, eight to a cell, as the driver has them,
 * and always at the top left of a cell in the text mode.
 */

#define DOTS 8
#define X_MOST (COLUMNS * DOTS - 1)
#define Y_MOST (ROWS * DOTS - 1)

static i32 MouseX = 0, MouseY = 0;
static i32 MouseTop = 0, MouseBottom = Y_MOST;  /* the rows it may go to */
static u16 MouseButtons = 0;
static i32 PointerHidden = 1;           /* shown at 0, as the driver counts */
static u16 PointerKeep = 0x77FF, PointerTurn = 0x7700;
static i32 PointerAt = -1;              /* the cell drawn on, or -1 */
static u16 PointerUnder = 0;            /* what that cell had */

static void PointerOff(void)
{
    if (PointerAt >= 0
        && Display[PointerAt]
               == (u16)((PointerUnder & PointerKeep) ^ PointerTurn))
        Display[PointerAt] = PointerUnder;
    PointerAt = -1;
}

static void PointerOn(void)
{
    if (PointerHidden != 0)
        return;
    PointerAt = MouseY / DOTS * COLUMNS + MouseX / DOTS;
    if (PointerAt >= CELLS_ALL) {       /* rows beyond the memory */
        PointerAt = -1;
        return;
    }
    PointerUnder = Display[PointerAt];
    Display[PointerAt] = (u16)((PointerUnder & PointerKeep) ^ PointerTurn);
}

/* The pointer to a place, kept to the screen and to the rows that it may
   go to, and drawn there. */
static void MousePlace(i32 x, i32 y)
{
    MouseX = (x < 0 ? 0 : x > X_MOST ? X_MOST : x) & ~(DOTS - 1);
    MouseY = (y < MouseTop ? MouseTop : y > MouseBottom ? MouseBottom : y)
             & ~(DOTS - 1);
    PointerOff();
    PointerOn();
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
    if ((mode & 0x80) == 0) {
        for (i = 0; i < CELLS_ALL; i++)
            Display[i] = BLANK;
        PointerAt = -1;                 /* what it kept is gone */
        PointerOn();
    }
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

/* What the program has typed by the BIOS since the terminal last drew
   the screen, and where the first of it went: what a command run writes
   on the terminal's own screen in front of what the command writes
   (Typed below, L6). */
#define TYPED_ROOM 1024

static u8 TypedBytes[TYPED_ROOM];
static i32 TypedCount = 0;
static i32 TypedRow = -1, TypedColumn = 0;

static void Type(u8 c)
{
    if (TypedCount == 0) {
        TypedRow = Row();
        TypedColumn = Column();
    }
    if (TypedCount < TYPED_ROOM)
        TypedBytes[TypedCount++] = c;
}

/* The lines move up a row when the cursor goes beyond the last, the
   new one in the colour of the cell at the cursor, as the BIOS has it. */
static void LineFeed(void)
{
    if (Row() < ROWS - 1) {
        Place(Row() + 1, Column());
        return;
    }
    if (TypedRow > 0)
        TypedRow--;
    ScreenScroll(false, 1, (u16)(ScreenCell(0) >> 8), 0,
                 (ROWS - 1) << 8 | (COLUMNS - 1));
}

/* What the BIOS does with a character that it types: a carriage return,
   a line feed, a step back and the bell are no characters; any other is
   written in the colour that the cell has, and the cursor goes on, to
   the next line behind the last column. And whatever it types, with the
   cursor below the last row, as the program has it where it hides it
   (Cursor of window.c), the BIOS takes it a row up and moves the lines
   up a row, as for a line feed there: the bench's BIOS does, and an
   error's beep moves the screen so (CLAUDE.md, the rules of a part).
   The new row has the colour of the first cell of the last, which no
   scenario tells from another: the ones there are have 07h in both. */
void ScreenType(u16 character, u16 page, u16 color)
{
    u16 *cell;
    u8 c = (u8)character;

    (void)page;
    (void)color;
    Type(c);
    switch (c) {
    case '\r':
        Place(Row(), 0);
        break;
    case '\n':
        LineFeed();
        return;
    case '\b':
        if (Column() > 0)
            Place(Row(), Column() - 1);
        break;
    case 7:
        break;
    default:
        if (Row() >= ROWS)
            break;
        cell = &Display[Row() * COLUMNS + Column()];
        *cell = (u16)((*cell & 0xFF00) | c);
        if (Column() < COLUMNS - 1) {
            Place(Row(), Column() + 1);
            return;
        }
        Place(Row(), 0);
        LineFeed();
        return;
    }
    if (Row() >= ROWS) {
        Place(Row() - 1, Column());
        ScreenScroll(false, 1, (u16)(Display[(ROWS - 1) * COLUMNS] >> 8), 0,
                     (ROWS - 1) << 8 | (COLUMNS - 1));
    }
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
 *     XXXX+    the same, with the next key put into the BIOS's buffer
 *              in the same round, as the driver puts the keys of one
 *              chord: a program that empties the buffer loses it
 *     held=XX  what KeysHeld answers from then on, two hex digits
 *     wait=N   nothing until the timer has ticked N times more
 *     mouse=C,R,B  the mouse moved to column C of row R, in decimal from
 *              0, with the buttons B held, a hex digit: the left one 1,
 *              the right one 2, the middle one 4; the pointer goes to
 *              the top left of the cell, kept to the rows that it may go
 *              to, as the bench's driver takes it there
 *     asks=N   nothing until the program has asked N times where the
 *              mouse is (MouseWhere), N from 1 to 255: at the last of
 *              them the next event is taken before the program is
 *              answered, as the driver of the bench takes it
 *
 * Each event is taken when the program is idle with no key waiting, one
 * at a time, as the driver takes its events: a key, a change of what is
 * held, a move of the mouse, each in a round of its own, so that the
 * program sees Alt held before it sees it let go. A move followed by
 * asks= takes both at once. While asks= is counting nothing else is
 * taken, but where the program waits for a key, which ends the count.
 * A scenario's "Click@3,5*4>3,2" is "mouse=3,5,1 asks=4 mouse=3,2,1
 * asks=20 mouse=3,2,0", "Move@40,12" is "mouse=40,12,0", and a click
 * without a count has 20, as harness/driver.py has them.
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
 *
 * With VC_TEST_CLOCK set as well, to a date and a time of the form
 * 2000-06-16T12:00:00, the program's clock is the test's, as the
 * machine's clock is on the bench, counted in what the machine executes:
 * it starts at that time and goes on only while the test waits, a tick
 * at every round that the program goes idle, by wait= or while AutoTim
 * is believed, and while the program waits for the next tick by itself
 * (Ticks), and a file that the program has written is stamped with
 * it when it is closed, as DOS stamps it. The time of day and the timer
 * are of it, and a user's run never has it.
 */

#define TEST_VARIABLE "VC_TEST"
#define CLOCK_VARIABLE "VC_TEST_CLOCK"
#define TICKS_A_DAY 0x1800B0            /* the BIOS's, of the timer */
#define RECORD_KIND 4

enum Event { EVENT_KEY, EVENT_HELD, EVENT_WAIT, EVENT_MOUSE, EVENT_ASKS };

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
#define KEY_WITH_NEXT 0x10000           /* a key word's +, in its value */
#define KEYS_ROOM 15                    /* the BIOS's buffer */
static u16 KeysWaiting[KEYS_ROOM];      /* the keys that wait, the first
                                           first */
static i32 KeysCount = 0;
static u16 Held = 0;
static bool Waiting = false;            /* for the timer, by wait= */
static int64_t WaitUntil = 0;           /* by Elapsed */
static i32 IdleRounds = 0;              /* with nothing left to take */
static i32 Asks = 0;                    /* asks= that are left */
static bool AutoSeen = false;           /* when AutoTim was first seen */
static int64_t AutoSince = 0;
static bool Clocked = false;            /* VC_TEST_CLOCK: the test's clock */
static u32 TestTicks = 0;               /* since its day's midnight */
static struct tm TestDay;               /* the date it began on */
static u32 WaitTicks = 0;               /* by TestTicks */
static u32 AutoTick = 0;                /* AutoSince, by TestTicks */
static i32 Polled = 0;                  /* reads of Ticks since the program
                                           was last idle */

/* The ticks for which AutoTim is believed, as DRIVER.ASM has them. */
#define SOON 37

static bool TestClock(void)
{
    return Clocked;
}

/* The time of day of the test's clock, as DOS makes it of the BIOS's
   ticks. */
static struct Time TestTime(void)
{
    int64_t centis = (int64_t)(TestTicks % TICKS_A_DAY) * 6553600 / 1193180;
    struct Time time;

    time.clock = (u16)(centis / 360000 << 8 | centis / 6000 % 60);
    time.seconds = (u16)(centis / 100 % 60 << 8 | centis % 100);
    return time;
}

/* The stamp of DOS of the test's clock, for a file that is closed. */
static struct DosStamp TestStamp(void)
{
    struct tm local = TestDay;
    int64_t seconds = (int64_t)TestTicks * 65536 / 1193180;

    local.tm_sec += (int)seconds;
    mktime(&local);
    return DosStampOfLocal(&local);
}

/* A number of the test's clock, of `digits` digits at `*at`, and the
   character that is to follow it, or none for the end: -1 where it is
   not so. */
static long ClockPart(const char **at, int digits, char behind)
{
    char *end;
    long value;

    if (**at < '0' || **at > '9')
        return -1;
    errno = 0;
    value = strtol(*at, &end, 10);
    if (errno != 0 || end - *at != digits || *end != behind)
        return -1;
    *at = behind == 0 ? end : end + 1;
    return value;
}

/* The test's clock from VC_TEST_CLOCK, if that is set. */
static void ClockBegin(void)
{
    const char *given = getenv(CLOCK_VARIABLE);
    const char *at = given;
    long year, month, day, hour, minute, second;

    if (given == 0)
        return;
    year = ClockPart(&at, 4, '-');
    month = year < 0 ? -1 : ClockPart(&at, 2, '-');
    day = month < 0 ? -1 : ClockPart(&at, 2, 'T');
    hour = day < 0 ? -1 : ClockPart(&at, 2, ':');
    minute = hour < 0 ? -1 : ClockPart(&at, 2, ':');
    second = minute < 0 ? -1 : ClockPart(&at, 2, 0);
    if (second < 0 || month < 1 || month > 12 || day < 1 || day > 31
        || hour > 23 || minute > 59 || second > 59) {
        fprintf(stderr, "vc: %s: no date and time: %s\n", CLOCK_VARIABLE,
                given);
        exit(1);
    }
    memset(&TestDay, 0, sizeof TestDay);
    TestDay.tm_year = (int)year - 1900;
    TestDay.tm_mon = (int)month - 1;
    TestDay.tm_mday = (int)day;
    TestDay.tm_isdst = -1;
    /* The first tick at the time or after it: of the tick before, DOS
       makes a time of a second before, 11:59:59 of 12:00:00. */
    TestTicks = (u32)(((hour * 3600 + minute * 60 + second) * 1193180
                       + 65535) / 65536);
    Clocked = true;
}

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

/* The C,R,B of mouse=, made an event: the column in the low byte of its
   value, the row in the next and the buttons above. */
static bool MouseEvent(const char *word, struct Due *due)
{
    unsigned long column, row, buttons;
    char *end;

    due->event = EVENT_MOUSE;
    column = strtoul(word, &end, 10);
    if (end == word || *end != ',' || column > 255)
        return false;
    word = end + 1;
    row = strtoul(word, &end, 10);
    if (end == word || *end != ',' || row > 255)
        return false;
    word = end + 1;
    buttons = strtoul(word, &end, 16);
    if (end == word || *end != 0 || buttons > 7)
        return false;
    due->value = (u32)(column | row << 8 | buttons << 16);
    return true;
}

/* A word of a line of `keys` made an event. False if it is none. */
static bool Event(const char *word, struct Due *due)
{
    char *end;
    unsigned long value;

    due->event = EVENT_KEY;
    if (strncmp(word, "mouse=", 6) == 0)
        return MouseEvent(word + 6, due);
    if (strncmp(word, "asks=", 5) == 0) {
        due->event = EVENT_ASKS;
        value = strtoul(word + 5, &end, 10);
        due->value = (u32)value;
        return end != word + 5 && *end == 0 && value >= 1 && value <= 255;
    }
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
    if (due->event == EVENT_KEY && end != word && *end == '+'
        && end[1] == 0) {
        due->value |= KEY_WITH_NEXT;
        end++;
    }
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
    ClockBegin();
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
    if (Clocked) {
        if (!AutoSeen) {
            AutoSeen = true;
            AutoTick = TestTicks;
        }
        if (TestTicks - AutoTick >= SOON)
            return false;
        TestTicks++;
        return true;
    }
    if (!AutoSeen) {
        AutoSeen = true;
        AutoSince = Elapsed();
    }
    return Elapsed() - AutoSince < 2000;
}

/* The next event of the step, if one is due: a key is not while one
   waits, nor anything while the timer is waited for or the asks are
   counted, unless `reading`, where the program waits for a key, which
   ends both as the driver ends them. False where the step has no event
   left. */
static bool TakeEvent(bool reading)
{
    struct Due *due;

    Polled = 0;
    if (KeysCount > 0)
        return true;
    if (reading)
        Asks = 0;
    if (Asks > 0)
        return true;
    if (Waiting && Clocked) {
        if (TestTicks < WaitTicks) {
            TestTicks++;
            return true;
        }
        Waiting = false;
    } else if (Waiting) {
        if (Elapsed() < WaitUntil) {
            Pause();
            return true;
        }
        Waiting = false;
    }
    if (StepNow == StepsAll || EventNow == Steps[StepNow].count)
        return false;
    due = &Events[Steps[StepNow].first + EventNow++];
    IdleRounds = 0;
    switch (due->event) {
    case EVENT_KEY:
        /* A key that has the next with it, a word with +, has it put
           into the buffer in the same round, as the driver puts the
           keys of one chord. */
        for (;;) {
            if (KeysCount < KEYS_ROOM)
                KeysWaiting[KeysCount++] = (u16)due->value;
            if ((due->value & KEY_WITH_NEXT) == 0
                || EventNow == Steps[StepNow].count
                || Events[Steps[StepNow].first + EventNow].event
                   != EVENT_KEY)
                break;
            due = &Events[Steps[StepNow].first + EventNow++];
        }
        break;
    case EVENT_HELD:
        /* The program goes round once with each change of what is
           held, as the driver has it on the bench. */
        Held = (u16)due->value;
        break;
    case EVENT_WAIT:
        Waiting = true;
        WaitUntil = Elapsed() + (int64_t)due->value * 65536000 / 1193180;
        WaitTicks = TestTicks + due->value;
        break;
    case EVENT_MOUSE:
        MouseButtons = (u16)(due->value >> 16);
        MousePlace((i32)(due->value & 0xFF) * DOTS,
                   (i32)(due->value >> 8 & 0xFF) * DOTS);
        if (EventNow < Steps[StepNow].count
            && Events[Steps[StepNow].first + EventNow].event == EVENT_ASKS)
            Asks = (i32)Events[Steps[StepNow].first + EventNow++].value;
        break;
    case EVENT_ASKS:
        Asks = (i32)due->value;
        break;
    }
    return true;
}

/* The program is idle with no key waiting: the next event is due, or
   the step ends. */
static void Due(bool reading)
{
    if (TakeEvent(reading))
        return;
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

/*
 * The terminal, where the program is not tested: term_posix.c, which
 * draws the page that is shown, the first 2,000 cells, whenever the
 * program asks for a key or is idle, and makes what is typed keys.
 */

#define IDLE_WAIT 50                    /* milliseconds, under a tick */

static bool OnTerminal = false;
static bool CommandShown = false;              /* to a command, which has run: the
                                           terminal's own screen shows */

/* The screen drawn, the terminal taken again first where a command had
   it. */
static void Show(void)
{
    if (CommandShown) {
        CommandShown = false;
        TermTake();
    }
    TypedCount = 0;
    TermDraw(Display, CursorPlace, Shape);
}

/* Ctrl-O typed at the terminal, as many times as it waits: the
   terminal's own screen, which has what the commands wrote, shown until
   the next key, which is dropped, and the program's again. The program
   does not get the key (TODO.md, "L6, commands"). */
#define CTRL_O 0x180F

static void OwnScreen(void)
{
    struct Keys keys = TermKeyWaiting();

    while (!keys.none && keys.key == CTRL_O) {
        TermKeyTake();
        TermOwnScreen();
        Show();
        TermInput(0);
        keys = TermKeyWaiting();
    }
}

/* The mouse as the terminal has reported it, taken as the driver takes
   a mouse's motion, where it has changed since it was last taken.
   `looked` where the program asks where the mouse is. */
static u32 PointerReports = 0;

static void MouseFromTerminal(bool looked)
{
    struct TermPointer pointer = TermMouse(looked);

    if (pointer.count == PointerReports)
        return;
    PointerReports = pointer.count;
    MouseButtons = pointer.buttons;
    MousePlace(pointer.column * DOTS, pointer.row * DOTS);
}

void Idle(void)
{
    if (OnTerminal) {
        MouseFromTerminal(false);
        Show();
        TermInput(IDLE_WAIT);
        MouseFromTerminal(false);
        return;
    }
    Due(false);
}

/*
 * The trace: told so by the variable VC_TRACE of the environment, which
 * names a file of the host, the program writes there what it is answered
 * of the keyboard and the mouse, a line for each: "key XXXX" for a key
 * read, "held XX" where KeysHeld answers otherwise than it last did, and
 * "mouse B X Y" where MouseWhere does, the buttons and the place in dots,
 * in hex. It is for the tests, harness/terminal.py, which hold what a
 * terminal types to what the program receives, beside the test channel
 * or without it; nothing else reads it.
 */

#define TRACE_VARIABLE "VC_TRACE"

static FILE *Trace = 0;
static bool TraceBegun = false;
static i32 TracedHeld = -1;
static i32 TracedMouse = -1;

static void Traced(const char *line)
{
    const char *path;

    if (!TraceBegun) {
        TraceBegun = true;
        path = getenv(TRACE_VARIABLE);
        if (path != 0) {
            Trace = fopen(path, "w");
            if (Trace == 0)
                Fail(path);
        }
    }
    if (Trace == 0)
        return;
    fputs(line, Trace);
    fflush(Trace);
}

/*
 * The keyboard: what the test channel has made due, or the terminal.
 * KeyRead waits for a key, which is idle as Idle is.
 */

/* Asked with no key there, the program is not idle: the bench's driver
   takes only INT 16h 00h and 10h and INT 28h for the program's idle
   call, not 01h, and makes no key due for it. */
struct Keys KeyWaiting(void)
{
    struct Keys keys;

    if (OnTerminal) {
        Show();
        TermInput(0);
        MouseFromTerminal(false);
        OwnScreen();
        return TermKeyWaiting();
    }
    keys.key = KeysCount > 0 ? KeysWaiting[0] : 0;
    keys.none = KeysCount == 0;
    return keys;
}

u16 KeyRead(void)
{
    char line[16];
    u16 key;

    if (OnTerminal) {
        for (;;) {
            OwnScreen();
            if (!TermKeyWaiting().none)
                break;
            MouseFromTerminal(false);
            Show();
            TermInput(-1);
        }
        MouseFromTerminal(false);
        key = TermKeyTake();
    } else {
        while (KeysCount == 0)
            Due(true);
        key = KeysWaiting[0];
        memmove(KeysWaiting, KeysWaiting + 1,
                (size_t)--KeysCount * sizeof *KeysWaiting);
    }
    snprintf(line, sizeof line, "key %04X\n", key);
    Traced(line);
    return key;
}

u16 KeysHeld(void)
{
    u16 held = OnTerminal ? TermKeysHeld() : Held;
    char line[16];

    if (held != TracedHeld) {
        TracedHeld = held;
        snprintf(line, sizeof line, "held %02X\n", held);
        Traced(line);
    }
    return held;
}

void KeyForgetIns(void)
{
    Held &= 0x7F;
}

/*
 * The mouse, as the bench's mouse driver answers INT 33h, which is
 * FreeDOS's, CuteMouse, loaded with /3 for the middle button: read there
 * on 15 October 2026 by the toy of harness/trial32/asked.py. Asked
 * whether there is one, by 21h or by a reset, it says FFFFh, and sets
 * itself up: the pointer hidden, in the middle of the screen, 320 by 96,
 * the rows those of the screen. A place that the program puts it at is
 * kept to the screen and to the rows, and made the top left of its cell:
 * 7FFFh by 7FFFh is 632 by 192. Hidden twice, it wants showing twice;
 * shown, it is the cell with the bits of `keep` kept and those of `turn`
 * turned round, 1E4Dh made 694Dh by FFFFh and 7700h, and drawn anew when
 * those change. The pointer's look before the program says it is the
 * driver's own, which the toy did not show, since the program always
 * says it before it shows the pointer.
 */

static u16 MouseSetUp(void)
{
    PointerOff();
    PointerHidden = 1;
    MouseTop = 0;
    MouseBottom = Y_MOST;
    PointerKeep = 0x77FF;
    PointerTurn = 0x7700;
    MousePlace((X_MOST + 1) / 2, (Y_MOST + 1) / 2);
    return 0xFFFF;
}

u16 MouseFind(void)
{
    return MouseSetUp();
}

u16 MouseReset(void)
{
    return MouseSetUp();
}

void MouseShow(void)
{
    if (PointerHidden == 0)
        return;
    PointerHidden--;
    if (PointerHidden == 0)
        PointerOn();
}

void MouseHide(void)
{
    if (PointerHidden++ == 0)
        PointerOff();
}

struct MouseAt MouseWhere(void)
{
    struct MouseAt at;
    char line[32];
    i32 traced;

    if (OnTerminal)
        MouseFromTerminal(true);
    else if (Asks > 0) {
        Asks--;
        if (Asks == 0)
            TakeEvent(false);
    }
    at.buttons = MouseButtons;
    at.x = (u16)MouseX;
    at.y = (u16)MouseY;
    traced = MouseButtons << 24 | MouseX << 12 | MouseY;
    if (traced != TracedMouse) {
        TracedMouse = traced;
        snprintf(line, sizeof line, "mouse %X %X %X\n", MouseButtons,
                 (unsigned)MouseX, (unsigned)MouseY);
        Traced(line);
    }
    return at;
}

void MouseTo(u16 x, u16 y)
{
    MousePlace(x, y);
}

void MouseRows(u16 first, u16 last)
{
    MouseTop = first < last ? first : last;
    MouseBottom = first < last ? last : first;
    MousePlace(MouseX, MouseY);
}

void MousePointer(u16 keep, u16 turn)
{
    PointerOff();
    PointerKeep = keep;
    PointerTurn = turn;
    PointerOn();
}

/*
 * The timer: the BIOS's ticks since midnight, 1,193,180 / 65,536 to the
 * second, of the time of day when the program started and the host's
 * monotonic clock since; the BIOS counts 1800B0h of them to a day.
 */

#define MS_A_DAY (24 * 3600 * 1000)

u32 Ticks(void)
{
    if (TestClock()) {
        /* A second read with no round idle between is the program
           waiting for the next tick by itself, as the zooming boxes do,
           which the machine's clock on the bench gets to by what the
           machine executes meanwhile: the test's clock ticks then. */
        if (++Polled > 1)
            TestTicks++;
        return TestTicks % TICKS_A_DAY;
    }
    return (u32)((StartMs + Elapsed()) % MS_A_DAY * 1193180 / 65536000
                 % TICKS_A_DAY);
}

/*
 * Other programs, and the end of this one.
 */

/* A command, the piece L6 (TODO.md, "L6, commands"). What the program
   runs is COMMAND.COM, by the name that COMSPEC has in its environment,
   with /C and the command for its tail: COMSPEC names the host's shell,
   $SHELL or /bin/sh where that is not set (ShellOf), and the shell is
   run with -c and the command made one of the host's
   (DosCommandToHost). Any other program is run by the shell too, by its
   path of the host and its tail.

   On the terminal, the command has it as the user has it, with its
   own screen and its modes as they were before the program took it, in
   the foreground, so that Ctrl-C and Ctrl-Z are its signals and not the
   program's; in front of what it writes, what DOS would have shown
   there: the line of the prompt and the command, which the program has
   on its screen, and what it has typed by the BIOS since (Typed). The
   terminal stays so until the program next draws its screen, so that
   commands run one behind the other are on one screen, as on DOS. A
   command that is stopped, Ctrl-Z, stays so when the program goes on,
   as behind a shell without job control. Under the test channel the
   command has nothing of standard input and writes nowhere.

   DOS's answer is that the program ran, whatever it ended with: the
   shell's 127 for a command that it did not find is a run, as COMMAND.COM
   that says "Bad command or file name" is. `value` is the code that the
   command ended with, which DOS would answer to 4Dh. The shell that
   cannot be run is DOS's 2, file not found, as a COMSPEC that names no
   file is. */
#define FILE_NOT_FOUND 2
#define SHELL_DEFAULT "/bin/sh"
#define COMMAND_ROOM 0x10000
#define TAIL_ROOM 256                   /* a count of a byte, and what it
                                           counts */

static char Shell[HOST_PATH_ROOM];      /* of the host */
static u8 ShellDos[DOS_PATH_ROOM] = "COMMAND.COM";  /* what COMSPEC names */

/* The shell: $SHELL where it is a path from the root, else /bin/sh; and
   the path of DOS's that COMSPEC names it by, or COMMAND.COM where DOS
   cannot name it. */
static void ShellOf(void)
{
    const char *shell = getenv("SHELL");
    char real[PATH_MAX];

    if (shell == 0 || shell[0] != '/' || strlen(shell) >= sizeof Shell)
        shell = SHELL_DEFAULT;
    memcpy(Shell, shell, strlen(shell) + 1);
    if (realpath(Shell, real) != 0)
        DosPathOfHost(&Emulated.directories, &Emulated.drives, real,
                      ShellDos);
}

/* Whether two names of DOS's are one, by their capitals. */
static bool SameName(const u8 *one, const u8 *other)
{
    for (; *one != 0 && *other != 0; one++, other++)
        if (CodePageUpper(*one) != CodePageUpper(*other))
            return false;
    return *one == *other;
}

/* What the program has typed, written on the terminal's own screen, and
   forgotten: in front of the first command, the row that the typing
   began in, as far as it began, or all of it where the typing began
   with a new line, which is the prompt and the command that the program
   drew there. */
static void Typed(bool first)
{
    char text[TYPED_ROOM * 4 + COLUMNS * 4];
    i32 length = 0, end = COLUMNS, i;
    bool whole = true;
    u8 c;

    if (first && TypedCount > 0 && TypedRow >= 0) {
        for (i = 0; i < TypedCount && TypedBytes[i] != '\n'; i++)
            if (TypedBytes[i] >= ' ')
                whole = false;
        if (!whole)
            end = TypedColumn;
        while (end > 0 && (u8)Display[TypedRow * COLUMNS + end - 1] == ' ')
            end--;
        for (i = 0; i < end; i++)
            length += Utf8Encode(TermGlyph((u8)Display[TypedRow * COLUMNS
                                                       + i]),
                                 text + length);
    }
    for (i = 0; i < TypedCount; i++) {
        c = TypedBytes[i];
        if (c == '\r' || c == '\n' || c == '\b')
            text[length++] = (char)c;
        else if (c != 7)
            length += Utf8Encode(TermGlyph(c), text + length);
    }
    TypedCount = 0;
    TermWrite(text, length);
}

/* The command run, in UTF-8, in the directory of the host that DOS's
   current one is: how it ended, the code in `value`. */
static struct Answer Run(const char *command)
{
    char directory[HOST_PATH_ROOM];
    struct Answer answer;
    sigset_t none;
    pid_t child;
    int status = 0, null;

    answer.failed = false;
    answer.value = 0;
    if (DosCurrentHost(&Emulated, directory, (i32)sizeof directory).failed)
        memcpy(directory, "/", 2);
    if (OnTerminal) {
        if (!CommandShown) {
            CommandShown = true;
            TermGive();
            Typed(true);
        } else {
            Typed(false);
        }
    }
    if (Trace != 0)
        fflush(Trace);
    child = fork();
    if (child == 0) {
        if (OnTerminal) {
            /* Its group, which the terminal is handed to; and what it
               says of errors on the terminal too, whatever the program's
               own standard error is. */
            setpgid(0, 0);
            TermHandTo(getpid());
            dup2(1, 2);
        } else {
            null = open("/dev/null", O_RDWR);
            if (null >= 0) {
                dup2(null, 0);
                dup2(null, 1);
                dup2(null, 2);
            }
        }
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, 0);
        if (chdir(directory) != 0 && chdir("/") != 0)
            _exit(127);
        execl(Shell, Shell, "-c", command, (char *)0);
        _exit(127);
    }
    if (child < 0) {
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    if (OnTerminal) {
        setpgid(child, child);
        TermHandTo(child);
    }
    while (waitpid(child, &status, WUNTRACED) < 0 && errno == EINTR)
        ;
    if (OnTerminal)
        TermHandBack();
    if (WIFEXITED(status))
        answer.value = (u16)WEXITSTATUS(status);
    return answer;
}

struct Answer ProgramRun(const u8 *name, const u8 *tail)
{
    u8 dos[DOS_PATH_ROOM + 1 + TAIL_ROOM + 1], full[DOS_PATH_ROOM];
    char host[HOST_PATH_ROOM];
    char *command;
    struct Answer answer;
    i32 length = 0, start = 0, count = tail[0], made;

    /* Under the test channel the step that asks for it is said on
       standard error, for the harness. */
    if (ScreensFd >= 0)
        fprintf(stderr, "vc: a program is run at step %d\n",
                (int)StepNow);
    answer.failed = true;
    answer.value = FILE_NOT_FOUND;
    if (access(Shell, X_OK) != 0)
        return answer;
    if (SameName(name, ShellDos)) {
        while (length < count && tail[1 + length] != 0)
            length++;
        if (length < 2 || tail[1] != '/' || CodePageUpper(tail[2]) != 'C')
            return answer;              /* the shell is run for /C alone */
        start = 3;
        if (start < length && tail[1 + start] == ' ')
            start++;
        memcpy(dos, tail + 1 + start, (size_t)(length - start));
        length -= start;
    } else {
        /* Another program, by its full path, which leads to a file of the
           host that the user may run. */
        if (DosPathFull(name, &Emulated.drives, full).failed)
            return answer;
        answer = HostPathOfDos(&Emulated.directories, &Emulated.drives,
                               full, host, (i32)sizeof host);
        if (answer.failed || answer.value != 1 || access(host, X_OK) != 0) {
            answer.failed = true;
            answer.value = FILE_NOT_FOUND;
            return answer;
        }
        length = (i32)strlen((const char *)full);
        memcpy(dos, full, (size_t)length);
        while (start < count && tail[1 + start] != 0)
            dos[length++] = tail[1 + start++];
    }
    command = malloc(COMMAND_ROOM);
    if (command == 0) {
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    made = DosCommandToHost(&Emulated, dos, length, command, COMMAND_ROOM);
    if (made < 0) {
        free(command);
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    answer = Run(command);
    free(command);
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
    /* COMSPEC first, the host's shell (ShellOf); one that the host has
       is left out. */
    made = (i32)strlen("COMSPEC=") + (i32)strlen((const char *)ShellDos);
    memcpy(Environment, "COMSPEC=", strlen("COMSPEC="));
    memcpy(Environment + strlen("COMSPEC="), ShellDos,
           strlen((const char *)ShellDos));
    at = made + 1;
    for (text = environ; *text != 0; text++) {
        if (strncmp(*text, "COMSPEC=", strlen("COMSPEC=")) == 0)
            continue;
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
        if (!TermBegin()) {
            fprintf(stderr, "vc: standard input and output are to be a "
                            "terminal\n");
            return 1;
        }
        OnTerminal = true;
    }
    MemoryAll = pages > 0 && page > 0 ? (int64_t)pages * page : BYTES_MOST;
    StartMs = TodayMs();
    clock_gettime(CLOCK_MONOTONIC, &StartedAt);
    DosBegin(&Emulated, getenv("HOME"));
    /* DosBegin has the host's clock for what is written; the test's
       clock, which TestBegin has set, takes its place. */
    if (Clocked)
        Emulated.clock = TestStamp;
    if (getcwd(here, sizeof here) == 0)
        Fail("the current directory");
    StartIn(here);
    ProgramPath(program);
    ShellOf();
    EnvironmentOf(program);
    TailOf(count, arguments);
    ScreenSetMode(3);
    Main();
    return 0;
}
