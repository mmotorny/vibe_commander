/*
 * The functions of sys.h for a system that has nothing: step 5 of the
 * portable C's road, a second implementation that builds and is not
 * meant to run. It is the program's proof that the common code asks the
 * system for nothing but what sys.h says, since it links with this and
 * nothing else; and it is the build that clang's warnings, clang-tidy
 * and the sanitizers read (CMakeLists.txt, portable_c-stub).
 *
 * Everything that can fail fails, with DOS's error 1, "invalid
 * function", and with nothing found where a failure says what there
 * is. A question has the answer of a system that has nothing to say:
 * no key waits, no mouse, no time, an empty command tail; the screen is
 * in mode 3, text of 80 columns by 25 rows, and every cell a blank. What
 * hands something over to the system, or takes something of it, does
 * nothing. The one thing that it says is a version of DOS, 5.0, so that
 * the program gets past its first test, which ends it on an older DOS,
 * to where it asks for memory, which ends it here: that is the run that
 * the sanitizers see.
 *
 * Only the low byte of a drive counts, by sys.h; none is looked at here.
 */
#include <stdlib.h>
#include <string.h>

#include "vc.h"

/* DOS's error 1: what every call that fails answers with. */
#define INVALID_FUNCTION 1

/* A blank cell: a space, grey on black. */
#define BLANK 0x0720

static struct Answer Fails(void)
{
    struct Answer answer;

    answer.value = INVALID_FUNCTION;
    answer.failed = true;
    return answer;
}

static struct Moved Moves(void)
{
    struct Moved moved;

    moved.count = 0;
    moved.error = INVALID_FUNCTION;
    moved.failed = true;
    return moved;
}

static struct FilePlace Seeks(void)
{
    struct FilePlace place;

    place.place = INVALID_FUNCTION;     /* the error in the low word */
    place.failed = true;
    return place;
}

static struct Device Devices(void)
{
    struct Device device;

    device.value = INVALID_FUNCTION;
    device.failed = true;
    device.kind = 0;
    return device;
}

static struct Block Blocks(void)
{
    struct Block block;

    block.block = 0;
    block.value = INVALID_FUNCTION;
    block.failed = true;
    block.largest = 0;                  /* nothing to give */
    return block;
}

/* Files. */

struct Answer FileCreate(const u8 *name, u16 attributes)
{
    (void)name;
    (void)attributes;
    return Fails();
}

struct Answer FileOpen(const u8 *name, u16 mode)
{
    (void)name;
    (void)mode;
    return Fails();
}

struct Answer FileOpenBare(const u8 *name, u16 mode)
{
    return FileOpen(name, mode);
}

struct Answer FileClose(u16 handle)
{
    (void)handle;
    return Fails();
}

struct Answer FileCloseBare(u16 handle)
{
    return FileClose(handle);
}

struct Moved FileRead(u16 handle, u8 *to, i32 count)
{
    (void)handle;
    (void)to;
    (void)count;
    return Moves();
}

struct Moved FileReadBare(u16 handle, u8 *to, i32 count)
{
    return FileRead(handle, to, count);
}

struct Moved FileWrite(u16 handle, const u8 *from, i32 count)
{
    (void)handle;
    (void)from;
    (void)count;
    return Moves();
}

struct FilePlace FileSeek(u16 handle, u32 place)
{
    (void)handle;
    (void)place;
    return Seeks();
}

struct FilePlace FileStep(u16 handle, u16 from, i32 step)
{
    (void)handle;
    (void)from;
    (void)step;
    return Seeks();
}

struct Answer FileDelete(const u8 *name)
{
    (void)name;
    return Fails();
}

struct Answer FileRename(const u8 *from, const u8 *to)
{
    (void)from;
    (void)to;
    return Fails();
}

struct FileAttr FileAttributes(const u8 *name)
{
    struct FileAttr attr;

    (void)name;
    attr.value = INVALID_FUNCTION;
    attr.failed = true;
    attr.attributes = 0;
    return attr;
}

struct Answer FileSetAttributes(const u8 *name, u16 attributes)
{
    (void)name;
    (void)attributes;
    return Fails();
}

struct FileStamp FileTime(u16 handle)
{
    struct FileStamp stamp;

    (void)handle;
    stamp.value = INVALID_FUNCTION;
    stamp.failed = true;
    stamp.time = 0;
    stamp.date = 0;
    return stamp;
}

struct Answer FileSetTime(u16 handle, u16 time, u16 date)
{
    (void)handle;
    (void)time;
    (void)date;
    return Fails();
}

struct Device FileDevice(u16 handle)
{
    (void)handle;
    return Devices();
}

struct Answer FileTrueName(const u8 *name, u8 *to)
{
    (void)name;
    (void)to;
    return Fails();
}

/* Files by a file control block. */

u16 FcbParse(const u8 *name, u8 *fcb, u16 how)
{
    (void)name;
    (void)fcb;
    (void)how;
    return 0xFF;                        /* a drive that there is not */
}

struct Answer FcbDelete(const u8 *fcb, bool flagged)
{
    (void)fcb;
    (void)flagged;
    return Fails();
}

/* Directories. */

void FindRecord(struct DTAs *record)
{
    (void)record;
}

struct Answer FindFirst(const u8 *pattern, u16 attributes)
{
    (void)pattern;
    (void)attributes;
    return Fails();
}

struct Answer FindNext(void)
{
    return Fails();
}

struct Answer DirMake(const u8 *name)
{
    (void)name;
    return Fails();
}

struct Answer DirRemove(const u8 *name)
{
    (void)name;
    return Fails();
}

struct Answer DirChange(const u8 *name)
{
    (void)name;
    return Fails();
}

struct Answer DirCurrent(u16 drive, u8 *to)
{
    (void)drive;
    (void)to;
    return Fails();
}

struct Answer DirCurrentBare(u16 drive, u8 *to)
{
    return DirCurrent(drive, to);
}

/* Drives. */

u16 DriveCurrent(void)
{
    return 2;                           /* C: */
}

void DriveSelect(u16 drive)
{
    (void)drive;
}

struct Space DriveSpace(u16 drive, bool flagged)
{
    struct Space space;

    (void)drive;
    (void)flagged;
    space.value = INVALID_FUNCTION;
    space.failed = true;
    space.free = 0;
    space.sector = 0;
    space.total = 0;
    return space;
}

struct Answer DriveLetter(u16 drive)
{
    (void)drive;
    return Fails();
}

struct Answer DriveSetLetter(u16 drive)
{
    (void)drive;
    return Fails();
}

struct Device DriveRemote(u16 drive)
{
    (void)drive;
    return Devices();
}

struct Answer DriveFixed(u16 drive)
{
    (void)drive;
    return Fails();
}

bool DriveCd(u16 drive)
{
    (void)drive;
    return false;
}

/* Memory: none to take, and the program's own is as long as it is. */

struct Block MemTake(i32 bytes)
{
    (void)bytes;
    return Blocks();
}

struct Answer MemGive(u8 *block)
{
    (void)block;
    return Fails();
}

struct Block MemResize(u8 *block, i32 bytes)
{
    (void)block;
    (void)bytes;
    return Blocks();
}

void MemSetSearch(u16 how)
{
    (void)how;
}

void MemSetUpper(bool on)
{
    (void)on;
}

struct Block MemOwn(void)
{
    struct Block block;

    block.block = 0;
    block.value = 0;
    block.failed = false;
    block.largest = 0;
    return block;
}

/* The time, the country, the system itself. */

struct Time TimeOfDay(void)
{
    struct Time time;

    time.clock = 0;
    time.seconds = 0;
    return time;
}

void CountryInfo(struct CNTRY *to)
{
    memset(to, 0, sizeof *to);
}

u16 CaseMap(u16 letter)
{
    return letter;                      /* no letter beyond ASCII changes */
}

struct SysVersion SystemVersion(void)
{
    struct SysVersion system;

    system.version = 5;                 /* 5.0: the major number low */
    system.other = 0;
    return system;
}

struct SysVersion SystemTrueVersion(void)
{
    return SystemVersion();
}

bool BreakCheck(void)
{
    return false;
}

void SetBreakCheck(bool on)
{
    (void)on;
}

void Idle(void)
{
}

/* Other programs, and the end of this one. */

struct Answer ProgramRun(const u8 *name, const u8 *tail)
{
    (void)name;
    (void)tail;
    return Fails();
}

struct Answer ProgramRunBare(const u8 *name, const u8 *tail)
{
    return ProgramRun(name, tail);
}

void ProgramEnd(u16 code)
{
    _Exit(code);
}

void ProgramEndOld(void)
{
}

const u8 *ProgramTail(void)
{
    static const u8 tail[] = "\r";

    return tail;
}

/* The environment: one text, the shell's, the zero behind the last, two
   bytes, and the path of the program's file, with room behind it for
   the 68 bytes that start.c reads of that path whatever it is. */
const u8 *ProgramEnvironment(void)
{
    static const u8 environment[128] =
        "COMSPEC=C:\\COMMAND.COM\0" "\0" "\1\0" "C:\\VC.EXE";

    return environment;
}

/* What the system asks the program: nothing, here. */

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

/* The console. */

void ConsoleChar(u16 character)
{
    (void)character;
}

void ConsoleText(const u8 *text)
{
    (void)text;
}

/* The keyboard. */

struct Keys KeyWaiting(void)
{
    struct Keys keys;

    keys.key = 0;
    keys.none = true;
    return keys;
}

u16 KeyRead(void)
{
    return 0;
}

u16 KeysHeld(void)
{
    return 0;
}

void KeyForgetIns(void)
{
}

/* The screen. */

struct Screen ScreenState(void)
{
    struct Screen screen;

    screen.mode = 3;
    screen.columns = 80;
    screen.page = 0;
    return screen;
}

void ScreenSetMode(u16 mode)
{
    (void)mode;
}

struct CursorAt CursorWhere(u16 page)
{
    struct CursorAt cursor;

    (void)page;
    cursor.place = 0;
    cursor.shape = 0x0607;              /* the cursor of a colour screen */
    return cursor;
}

void CursorTo(u16 page, u16 place)
{
    (void)page;
    (void)place;
}

void CursorShape(u16 shape)
{
    (void)shape;
}

void ScreenScroll(bool down, i32 lines, u16 color, u16 upper, u16 lower)
{
    (void)down;
    (void)lines;
    (void)color;
    (void)upper;
    (void)lower;
}

u16 ScreenCell(u16 page)
{
    (void)page;
    return BLANK;
}

void ScreenFill(u16 page, u16 character, u16 color, i32 count)
{
    (void)page;
    (void)character;
    (void)color;
    (void)count;
}

void ScreenType(u16 character, u16 page, u16 color)
{
    (void)character;
    (void)page;
    (void)color;
}

bool ScreenEga(void)
{
    return false;
}

struct Font ScreenFont(void)
{
    struct Font font;

    font.height = 16;                   /* a display that does not say */
    font.last = 0;
    return font;
}

u16 ScreenPageStart(void)
{
    return 0;
}

void ScreenMemory(u16 mode, u16 start)
{
    (void)mode;
    (void)start;
}

void ScreenPut(i32 cell, const u16 *from, i32 count)
{
    (void)cell;
    (void)from;
    (void)count;
}

void ScreenGet(i32 cell, u16 *to, i32 count)
{
    i32 i;

    (void)cell;
    for (i = 0; i < count; i++)
        to[i] = BLANK;
}

/* The mouse: there is none. */

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

/* The timer. */

u32 Ticks(void)
{
    return 0;
}

/* Where the system starts the program. */

int main(void)
{
    Main();
    return 0;
}
