/* The functions of the portable C's sys.h, each asked and what it
   answers written onto the screen: the toy by which dos32.c, the
   functions for the program of 32 bits, is held against dos.c, those of
   the program of 16 bits. It is one source built both ways (build.sh):

     sys-16.exe   by wcc, with the dos.obj and the stack.obj that the
                  project's build has made for the program;
     sys-32.exe   by wcc386, with src/portable_c/dos32.c and stack.c, and
                  DOS/32A for its stub.

   The two have to write the same on the bench, a screen for a screen:
   both.py runs them there and says where they do not. What is asked is
   what the program asks, and what it could: a call that fails, a file
   that is not there, a drive that there is not, a count of none.

   It writes a page at a time and then waits as the program waits, with
   the idle call, a look at the mouse and a look for a key, so each page
   is the screen of one step of a script, and the next page begins with
   what turned it: the key as KeyWaiting and KeyRead had it and what
   KeysHeld said then, or where the mouse was when a button went down.
   The steps are both.py's STEPS, ten of them for the eleven pages.

   An item is a name and what was answered, in hex: of a struct Answer
   the value and, behind a slash, whether it failed. What turns out
   otherwise from one run to the next is not written: a time of day is
   written as far as its minute, and of a number that follows the
   machine or the length of the program only whether it is plausible.

   The toy has no C library, as the program has none. It writes straight
   into the memory of the display, so that what the functions for the
   screen do shows beside it. On DOS in 16 bits the main data fills the
   segment that DS is on all but for 2.9 KB, so a text of the toy is a
   constant of the code segment, CODE, by the macros below, and its
   buffers are those of the main data; and a pointer to a local variable
   would be one into another segment, the stack's, so there is none.

   In 32 bits it also looks, before each thing that it writes, whether
   ES is still on the data and the direction flag still clear, which the
   compiler takes them to be and an interrupt through the extender might
   not leave them: `strayed` and a number on the screen says that one
   was not, 400h for the flag and anything else for ES.

   The last page asks the functions that dos32.c does not make yet
   (both.py, NOT_MADE): there the two differ until it does. ProgramRun
   and ProgramRunBare are not asked at all: a program that the toy could
   run, and what shows that it ran, come with the piece that makes them
   (TODO.md, "The portable C in 32 bits: the plan", D2). And the seven
   by which the program takes the critical error, Ctrl-Break and Ctrl-C
   are called and no more: a device that fails, and what the program is
   asked then, come with that piece too. */
#include "vc.h"

/* The main data, which the functions keep two things in: ErrDat and
   Screen. On DOS in 16 bits it is the first thing of the segment that
   DS is on, as main.c of the program has it. */
#ifdef __I86__
#pragma data_seg("MAINDATA", "MAIN")
#endif
struct MainDat md;
#ifdef __I86__
#pragma data_seg()
#endif

/* A place of the machine's memory, by its segment and offset in real
   mode. */
#ifdef __I86__
#define MACHINE(type, segment, offset) \
    ((type __far *)((u32)(segment) << 16 | (offset)))
#else
#define MACHINE(type, segment, offset) \
    ((type *)(((u32)(segment) << 4) + (offset)))
#endif
#define DISPLAY MACHINE(u16, 0xB800, 0)

/* A text of the toy, in the code segment where the memory has segments;
   and one written out. */
#define TEXT(name, text) static const u8 CODE name[] = text
#define SAY(text) { TEXT(said, text); Put(said); }

/* The buffers. BIG is the copies of the screen that the main data has
   one behind the other, 38,016 bytes of them. */
#define BIG      ((u8 *)md.ScrBuf)
#define BIG_SIZE 30000
#define SMALL    md.TmpBuf

static u16 Row, Col;

#ifndef __I86__
static u32 Strayed(void);
#pragma aux Strayed = \
    "mov ax,es" \
    "mov dx,ds" \
    "sub ax,dx" \
    "movzx eax,ax" \
    "pushfd" \
    "pop edx" \
    "and edx,400h" \
    "or eax,edx" \
    "push ds" \
    "pop es" \
    "cld" \
    value [eax] modify exact [eax edx]
#endif

static void Cell(u16 character)
{
    if (Col >= 80) {
        Col = 0;
        Row++;
    }
    if (Row >= 25) {                    /* more than a page holds */
        DISPLAY[24 * 80 + 79] = 0x4F00 | '+';
        return;
    }
    DISPLAY[Row * 80 + Col] = 0x0700 | character;
    Col++;
}

static void Digits(u32 value, u16 digits)
{
    TEXT(hex, "0123456789ABCDEF");

    while (digits-- != 0)
        Cell(hex[(u16)(value >> (digits << 2)) & 15]);
}

static void Put(const u8 FAR *text)
{
#ifndef __I86__
    u32 strayed = Strayed();

    if (strayed != 0) {
        TEXT(said, "strayed ");
        const u8 *at = said;

        while (*at != 0)
            Cell(*at++);
        Digits(strayed, 4);
        Cell(' ');
    }
#endif
    while (*text != 0)
        Cell(*text++);
}

static void Line(void)
{
    if (Col != 0) {
        Col = 0;
        Row++;
    }
}

static void Clear(void)
{
    u16 at;

    for (at = 0; at < 25 * 80; at++)
        DISPLAY[at] = 0x0720;
    Row = Col = 0;
}

/* A number, as so many hex digits with a space behind. */
static void Hex(u32 value, u16 digits)
{
    Digits(value, digits);
    Cell(' ');
}

#define H2(value) Hex((u8)(value), 2)
#define H4(value) Hex((u16)(value), 4)
#define H8(value) Hex(value, 8)

/* A text of the memory between quotes, as far as its zero byte or so
   many characters, with a dot for what is no character. */
static void Quoted(const u8 FAR *text, u16 most)
{
    Cell('\'');
    for (; *text != 0 && most != 0; text++, most--)
        Cell(*text < ' ' ? '.' : *text);
    Cell('\'');
    Cell(' ');
}

/* What a function answered, behind its name. */
static void Answer(const u8 FAR *name, struct Answer answer)
{
    Put(name);
    Cell(' ');
    Digits(answer.value, 4);
    Cell('/');
    Hex(answer.failed, 1);
}

#define ANSWER(name, call) { TEXT(said, name); Answer(said, call); }

static void Copy(u8 *to, const u8 FAR *from, u16 count)
{
    while (count-- != 0)
        *to++ = *from++;
}

/* What a byte of the big buffer is written as, by its place. */
static u8 Pattern(u16 at)
{
    return (u8)(at * 7 + (at >> 8));
}

/* What turned the last page: a key, or a button of the mouse. */
static struct Keys Waited;
static u16 Read, Held;
static struct MouseAt Clicked;

/* Wait as the program does, and begin the next page with what ended the
   wait: with the mode of the screen set first, which clears it, if
   `mode` says so. */
static void Page(u16 mode)
{
    struct MouseAt at;
    u16 down = 0;

    for (;;) {
        Idle();
        at = MouseWhere();
        if (at.buttons & 7) {
            Clicked = at;
            down = 1;
            continue;
        }
        if (down)
            break;
        Waited = KeyWaiting();
        if (Waited.none)
            continue;
        Read = KeyRead();
        Held = KeysHeld();
        break;
    }
    if (mode)
        ScreenSetMode(3);
    Clear();
    if (down) {
        SAY("turned by a button of the mouse: MouseWhere ");
        H4(Clicked.buttons); H4(Clicked.x); H4(Clicked.y);
    } else {
        SAY("turned by a key: KeyWaiting ");
        H4(Waited.key); H4(Waited.none);
        SAY("KeyRead "); H4(Read);
        SAY("KeysHeld "); H2(Held);
    }
    Line();
}

/* The names that the toy asks for. C:\WORK is the current directory on
   the bench, and harness/setups.py says what it and C:\STAMPS hold. */
TEXT(Toy_S, "TOY.DAT");
TEXT(Toy2_S, "TOY2.DAT");
TEXT(Hello_S, "HELLO.TXT");
TEXT(None_S, "NONE.DAT");
TEXT(NoDir_S, "C:\\NODIR\\X.DAT");
TEXT(Docs_S, "DOCS");
TEXT(NewDir_S, "NEWDIR");
TEXT(Up_S, "..");
TEXT(All_S, "*.*");
TEXT(Txt_S, "*.TXT");
TEXT(Stamps_S, "C:\\STAMPS\\*.DAT");
TEXT(NoneAll_S, "NONE.*");
TEXT(NoDirAll_S, "C:\\NODIR\\*.*");
TEXT(Root_S, "C:\\*.*");
TEXT(Here_S, "C:\\WORK\\NEWDIR");
TEXT(Full_S, "C:\\STAMPS");

/* The system, the time and the drives. */
static u16 Owned;                       /* whether MemOwn failed, which
                                           Main asked first */

static void System(void)
{
    struct SysVersion version;
    struct Time time;
    struct Space space;
    struct Device device;
    u32 ticks;
    u16 was;

    SAY("MemOwn failed "); H4(Owned);
    version = SystemVersion();
    SAY("SystemVersion "); H4(version.version); H4(version.other);
    version = SystemTrueVersion();
    SAY("SystemTrueVersion "); H4(version.version); H4(version.other);
    Line();
    was = BreakCheck();
    SAY("BreakCheck "); H4(was);
    SetBreakCheck(1);
    SAY("SetBreakCheck 1: "); H4(BreakCheck());
    SetBreakCheck(0);
    SAY("0: "); H4(BreakCheck());
    SetBreakCheck(0x0301);              /* the low byte counts */
    SAY("0301: "); H4(BreakCheck());
    SetBreakCheck(was);
    Line();
    time = TimeOfDay();
    IntOn();                            /* nothing shows of it: the toy
                                           is never kept from being
                                           interrupted */
    ticks = Ticks();
    SAY("TimeOfDay "); H4(time.clock);
    SAY("seconds in range "); H2((time.seconds >> 8) < 60
                                 && (time.seconds & 0xFF) < 100);
    /* 5,462 ticks are five minutes. */
    SAY("Ticks: the minute by them is the clock's ");
    H2((u16)(ticks * 5 / 5462)
       == (time.clock >> 8) * 60 + (time.clock & 0xFF));
    Line();
    SAY("DriveCurrent "); H4(DriveCurrent());
    DriveSelect(3);
    SAY("DriveSelect 3: "); H4(DriveCurrent());
    DriveSelect(25);                    /* Z:, which there is not */
    SAY("25: "); H4(DriveCurrent());
    DriveSelect(0x0102);                /* the low byte counts */
    SAY("0102: "); H4(DriveCurrent());
    Line();
    space = DriveSpace(0, 0);
    SAY("DriveSpace 0,0: "); H4(space.value); H4(space.failed);
    H4(space.free); H4(space.sector); H4(space.total);
    space = DriveSpace(3, 1);           /* with the carry flag set */
    SAY("3,1: "); H4(space.value); H4(space.failed);
    H4(space.free); H4(space.sector); H4(space.total);
    Line();
    space = DriveSpace(26, 0);          /* Z: again */
    SAY("DriveSpace 26,0: "); H4(space.value); H4(space.failed);
    H4(space.free); H4(space.sector); H4(space.total);
    space = DriveSpace(26, 1);
    SAY("26,1: "); H4(space.value); H4(space.failed);
    H4(space.free); H4(space.sector); H4(space.total);
    Line();
    /* D: has the program on it, which is not as long in both: how much
       of the drive is free is not written. */
    space = DriveSpace(4, 0);
    SAY("DriveSpace 4,0: "); H4(space.value); H4(space.failed);
    H4(space.sector); H4(space.total);
    Line();
    ANSWER("DriveLetter 1:", DriveLetter(1));
    ANSWER("3:", DriveLetter(3));
    ANSWER("26:", DriveLetter(26));
    ANSWER("DriveSetLetter 1:", DriveSetLetter(1));
    ANSWER("3:", DriveSetLetter(3));
    ANSWER("26:", DriveSetLetter(26));
    Line();
    device = DriveRemote(1);
    SAY("DriveRemote 1: "); H4(device.value); H4(device.failed);
    H4(device.kind);
    device = DriveRemote(3);
    SAY("3: "); H4(device.value); H4(device.failed); H4(device.kind);
    device = DriveRemote(26);
    SAY("26: "); H4(device.value); H4(device.failed); H4(device.kind);
    Line();
    ANSWER("DriveFixed 1:", DriveFixed(1));
    ANSWER("3:", DriveFixed(3));
    ANSWER("26:", DriveFixed(26));
    SAY("DriveCd 0: "); H4(DriveCd(0));
    SAY("2: "); H4(DriveCd(2));
    SAY("25: "); H4(DriveCd(25));
    Line();
    device = FileDevice(0);
    SAY("FileDevice 0: "); H4(device.value); H4(device.failed);
    H4(device.kind);
    device = FileDevice(1);
    SAY("1: "); H4(device.value); H4(device.failed); H4(device.kind);
    device = FileDevice(4);
    SAY("4: "); H4(device.value); H4(device.failed); H4(device.kind);
    device = FileDevice(99);            /* no handle */
    SAY("99: "); H4(device.value); H4(device.failed); H4(device.kind);
    Line();
}

static void Place(const u8 FAR *name, struct FilePlace place)
{
    Put(name);
    Cell(' ');
    Digits(place.place, 8);
    Cell('/');
    Hex(place.failed, 1);
}

#define PLACE(name, call) { TEXT(said, name); Place(said, call); }

static void Attr(const u8 FAR *name, struct FileAttr attr)
{
    Put(name);
    Cell(' ');
    Digits(attr.value, 4);
    Cell('/');
    Hex(attr.failed, 1);
    H4(attr.attributes);
}

#define ATTR(name, call) { TEXT(said, name); Attr(said, call); }

static void Stamp(const u8 FAR *name, struct FileStamp stamp)
{
    Put(name);
    Cell(' ');
    Digits(stamp.value, 4);
    Cell('/');
    Hex(stamp.failed, 1);
    H4(stamp.time);
    H4(stamp.date);
}

#define STAMP(name, call) { TEXT(said, name); Stamp(said, call); }

/* Files: one made, written, cut off, read back, renamed and deleted. */
static void Files(void)
{
    struct Answer asked;
    struct Device device;
    u16 handle, at, wrong;

    for (at = 0; at < BIG_SIZE; at++)
        BIG[at] = Pattern(at);
    asked = FileCreate(Toy_S, 0);
    ANSWER("FileCreate", asked);
    handle = asked.value;
    ANSWER("FileWrite 30000:", FileWrite(handle, BIG, BIG_SIZE));
    PLACE("FileSeek 0,100:", FileSeek(handle, 0, 100));
    PLACE("2,0:", FileSeek(handle, 2, 0));
    Line();
    PLACE("FileSeek 2,-10:", FileSeek(handle, 2, 0xFFFFFFF6UL));
    PLACE("1,5:", FileSeek(handle, 1, 5));
    PLACE("0,12345678h:", FileSeek(handle, 0, 0x12345678UL));
    Line();
    PLACE("FileSeek 3,89ABh:", FileSeek(handle, 3, 0x89ABUL));
    PLACE("no handle:", FileSeek(99, 0, 0x12345UL));
    PLACE("0,20000:", FileSeek(handle, 0, 20000));
    Line();
    ANSWER("FileWrite 0:", FileWrite(handle, BIG, 0));
    PLACE("FileSeek 2,0:", FileSeek(handle, 2, 0));
    ANSWER("FileSetTime", FileSetTime(handle, 0x6000, 0x28D0));
    Line();
    STAMP("FileTime", FileTime(handle));
    STAMP("no handle:", FileTime(99));
    ANSWER("FileSetTime, no handle:", FileSetTime(99, 0x6000, 0x28D0));
    Line();
    device = FileDevice(handle);
    SAY("FileDevice "); H4(device.value); H4(device.failed);
    H4(device.kind & 0xFFC0);           /* the drive is in the low bits */
    ANSWER("FileClose", FileClose(handle));
    ANSWER("again:", FileClose(handle));
    ANSWER("FileCloseBare, no handle:", FileCloseBare(99));
    Line();
    asked = FileOpen(Toy_S, 0);
    ANSWER("FileOpen", asked);
    handle = asked.value;
    for (at = 0; at < BIG_SIZE; at++)
        BIG[at] = 0;
    ANSWER("FileRead 30000:", FileRead(handle, BIG, BIG_SIZE));
    for (at = wrong = 0; at < 20000; at++)
        wrong += BIG[at] != Pattern(at);
    for (; at < BIG_SIZE; at++)
        wrong += BIG[at] != 0;
    SAY("wrong bytes "); H4(wrong);
    ANSWER("at the end:", FileRead(handle, BIG, 100));
    Line();
    STAMP("FileTime", FileTime(handle));
    PLACE("FileSeek 0,300:", FileSeek(handle, 0, 300));
    ANSWER("FileReadBare 5:", FileReadBare(handle, SMALL, 5));
    H2(SMALL[0]); H2(SMALL[4]); H2(Pattern(300)); H2(Pattern(304));
    Line();
    ANSWER("FileReadBare, no handle:", FileReadBare(99, SMALL, 5));
    ANSWER("FileRead, no handle:", FileRead(99, SMALL, 5));
    ANSWER("FileWrite, opened to read:", FileWrite(handle, SMALL, 5));
    Line();
    ANSWER("FileWrite, no handle:", FileWrite(99, SMALL, 5));
    ANSWER("FileCloseBare", FileCloseBare(handle));
    ANSWER("FileOpen, none:", FileOpen(None_S, 0));
    ANSWER("no directory:", FileOpen(NoDir_S, 0));
    Line();
}

/* More of files: what fails, attributes, another name, and the end. */
static void Names(void)
{
    struct Answer asked;
    u16 handle, at;

    ANSWER("FileOpenBare, none:", FileOpenBare(None_S, 0));
    ANSWER("no directory:", FileOpenBare(NoDir_S, 0));
    asked = FileOpenBare(Hello_S, 0x0102);      /* the low byte counts */
    ANSWER("to read and write:", asked);
    handle = asked.value;
    Line();
    for (at = 0; at < 32; at++)
        SMALL[at] = 0;
    ANSWER("FileReadBare 30:", FileReadBare(handle, SMALL, 30));
    Quoted(SMALL, 30);
    STAMP("FileTime", FileTime(handle));
    ANSWER("FileClose", FileClose(handle));
    Line();
    ATTR("FileAttributes", FileAttributes(Toy_S));
    ATTR("a directory:", FileAttributes(Docs_S));
    ATTR("none:", FileAttributes(None_S));
    ATTR("no directory:", FileAttributes(NoDir_S));
    Line();
    ANSWER("FileSetAttributes 1:", FileSetAttributes(Toy_S, 1));
    ATTR("FileAttributes", FileAttributes(Toy_S));
    ANSWER("FileDelete, read only:", FileDelete(Toy_S));
    ANSWER("FileOpen to write:", FileOpen(Toy_S, 1));
    Line();
    ANSWER("FileSetAttributes 0:", FileSetAttributes(Toy_S, 0));
    ANSWER("none:", FileSetAttributes(None_S, 0));
    ANSWER("FileRename", FileRename(Toy_S, Toy2_S));
    ANSWER("again:", FileRename(Toy_S, Toy2_S));
    Line();
    ANSWER("FileRename onto another:", FileRename(Toy2_S, Hello_S));
    ANSWER("to no directory:", FileRename(Toy2_S, NoDir_S));
    ATTR("FileAttributes", FileAttributes(Toy2_S));
    Line();
    ANSWER("FileDelete", FileDelete(Toy2_S));
    ANSWER("again:", FileDelete(Toy2_S));
    ANSWER("no directory:", FileDelete(NoDir_S));
    ANSWER("FileCreate, no directory:", FileCreate(NoDir_S, 0));
    Line();
    asked = FileCreate(Toy_S, 2);       /* hidden */
    ANSWER("FileCreate hidden:", asked);
    ANSWER("FileClose", FileClose(asked.value));
    ATTR("FileAttributes", FileAttributes(Toy_S));
    ANSWER("FileCreate over it:", FileCreate(Toy_S, 0));
    Line();
    ANSWER("FileSetAttributes 0:", FileSetAttributes(Toy_S, 0));
    ANSWER("FileDelete", FileDelete(Toy_S));
    ANSWER("FileCreate over a directory:", FileCreate(Docs_S, 0));
    Line();
}

/* What a search found, of the record: the name, the attributes and the
   size, and the time and the date if `stamped`. */
static void Found(u16 stamped)
{
    Quoted(md.DTA.FilName, 13);
    H2(md.DTA.FilAttr);
    Hex(md.DTA.FilSize, 8);
    if (stamped) {
        H4(md.DTA.FilTime);
        H4(md.DTA.FilDate);
    }
}

/* A search to its end, each thing found written, and the error that
   ended it. */
static void Listed(const u8 FAR *pattern, u16 attributes, u16 stamped)
{
    struct Answer found;

    found = FindFirst(pattern, attributes);
    while (!found.failed) {
        Found(stamped);
        found = FindNext();
    }
    ANSWER("ended", found);
    Line();
}

/* Directories: the current one, and one made and removed. */
static void Directories(void)
{
    u16 at;

    for (at = 0; at < 80; at++)
        SMALL[at] = 0;
    ANSWER("DirCurrent 0:", DirCurrent(0, SMALL));
    Quoted(SMALL, 64);
    ANSWER("4:", DirCurrent(4, SMALL));
    Quoted(SMALL, 64);
    ANSWER("26:", DirCurrent(26, SMALL));
    Line();
    ANSWER("DirCurrentBare 0303h:", DirCurrentBare(0x0303, SMALL));
    Quoted(SMALL, 64);
    ANSWER("26:", DirCurrentBare(26, SMALL));
    Line();
    ANSWER("DirMake", DirMake(NewDir_S));
    ANSWER("again:", DirMake(NewDir_S));
    ANSWER("no directory:", DirMake(NoDir_S));
    ANSWER("DirChange", DirChange(NewDir_S));
    Line();
    ANSWER("DirCurrent", DirCurrent(0, SMALL));
    Quoted(SMALL, 64);
    ANSWER("DirRemove, the current one:", DirRemove(Here_S));
    ANSWER("DirChange ..:", DirChange(Up_S));
    Line();
    ANSWER("DirRemove", DirRemove(NewDir_S));
    ANSWER("again:", DirRemove(NewDir_S));
    ANSWER("DirChange, none:", DirChange(NewDir_S));
    ANSWER("DirRemove, with files:", DirRemove(Full_S));
    Line();
}

/* What is in a directory. */
static void Finds(void)
{
    struct Answer asked;

    FindRecord(&md.DTA);
    SAY("FindRecord, FindFirst, FindNext"); Line();
    Listed(All_S, 0x37, 0);
    Listed(Stamps_S, 0, 1);
    Listed(All_S, 0, 0);
    Listed(NoneAll_S, 0x37, 0);
    Listed(NoDirAll_S, 0x37, 0);
    Listed(Root_S, 8, 1);               /* the label of the drive */
    /* Other things asked between two finds of one search, as the
       program asks them. */
    ANSWER("FindFirst", FindFirst(Txt_S, 0));
    Found(0);
    asked = FileOpen(Hello_S, 0);
    FileRead(asked.value, BIG, 20);
    FileClose(asked.value);
    DirCurrent(0, SMALL + 100);
    FileAttributes(Docs_S);
    ANSWER("FindNext behind other calls:", FindNext());
    Found(0);
    Line();
    /* The record of one search kept while another is made, and put
       back. */
    FindFirst(Txt_S, 0);
    Copy(SMALL, (const u8 *)&md.DTA, sizeof md.DTA);
    ANSWER("FindFirst of another:", FindFirst(Stamps_S, 0));
    Found(0);
    Copy((u8 *)&md.DTA, SMALL, sizeof md.DTA);
    ANSWER("FindNext with the record put back:", FindNext());
    Found(0);
    Line();
    /* The record elsewhere, with what it held. */
    FindFirst(Txt_S, 0);
    Copy(SMALL, (const u8 *)&md.DTA, sizeof md.DTA);
    Copy((u8 *)&md.DTA, Stamps_S, 16);
    FindRecord((struct DTAs *)SMALL);
    ANSWER("FindNext with the record moved:", FindNext());
    Quoted(((struct DTAs *)SMALL)->FilName, 13);
    Quoted((const u8 *)&md.DTA, 16);
    FindRecord(&md.DTA);
    Line();
}

/* A cursor written out, and cells of the screen. */
static void Caret(const u8 FAR *name, u16 page)
{
    struct CursorAt cursor;

    cursor = CursorWhere(page);
    Put(name);
    Cell(' ');
    H4(cursor.place);
    H4(cursor.shape);
}

#define CARET(name, page) { TEXT(said, name); Caret(said, page); }

static void Cells(u16 cell, u16 count)
{
    u16 at;

    ScreenGet(cell, (u16 *)SMALL, count);
    for (at = 0; at < count; at++)
        H4(((u16 *)SMALL)[at]);
}

TEXT(Console_S, "a text for DOS, as far as its dollar$ and no further");

/* The screen and the console. What is written by the functions is on
   rows of its own, and what they answered on the rows between. */
static void Display(void)
{
    struct Screen screen;
    struct Font font;
    u16 page, at;

    screen = ScreenState();
    page = screen.page;
    SAY("ScreenSetMode 3, ScreenState ");
    H4(screen.mode); H4(screen.columns); H4(screen.page);
    SAY("ScreenEga "); H4(ScreenEga());
    font = ScreenFont();
    SAY("ScreenFont "); H4(font.height); H4(font.last);
    Line();
    SAY("ScreenPageStart "); H4(ScreenPageStart());
    CARET("CursorWhere", page);
    CursorTo(page, 0x0A05);
    CARET("CursorTo 0A05:", page);
    Line();
    CursorShape(0x0307);
    CARET("CursorShape 0307:", page);
    ScreenMemory(screen.mode, ScreenPageStart());
    for (at = 0; at < 6; at++)
        ((u16 *)SMALL)[at] = 0x1E41 + at;       /* ABCDEF, yellow on blue */
    ScreenPut(6 * 80 + 70, (const u16 *)SMALL, 6);
    Line();
    SAY("ScreenPut, ScreenGet "); Cells(6 * 80 + 69, 8);
    Line();
    CursorTo(page, 0x0700);
    ScreenFill(page, 'x', 0x2F, 3);
    SAY("ScreenFill "); Cells(7 * 80, 4);
    SAY("ScreenCell "); H4(ScreenCell(page));
    CARET("the cursor stays:", page);
    Row = 8;
    Col = 0;
    ScreenFill(page, 0x01FE, 0x0370, 0);        /* the low bytes count */
    SAY("ScreenFill of none "); Cells(7 * 80, 1);
    CursorTo(page, 0x0903);
    ScreenType('T', page, 0x4E);
    ScreenType('y', page, 0x4E);
    CARET("ScreenType:", page);
    Cells(9 * 80 + 3, 2);
    ScreenType('\r', page, 0);
    ScreenType('\n', page, 0);
    CARET("a new line:", page);
    ConsoleText(Console_S);             /* on row 10 */
    ConsoleChar('!');
    ConsoleChar(0x0100 | '?');          /* the low byte counts */
    Row = 11;
    Col = 0;
    CARET("ConsoleText, ConsoleChar:", page);
    /* Three rows of letters, moved up by one with a colour for the row
       that comes in; down by two; and emptied. */
    for (at = 0; at < 24; at++)
        ((u16 *)SMALL)[at] = 0x0761 + at;
    ScreenPut(12 * 80, (const u16 *)SMALL, 8);
    ScreenPut(13 * 80, (const u16 *)SMALL + 8, 8);
    ScreenPut(14 * 80, (const u16 *)SMALL + 16, 8);
    ScreenScroll(0, 1, 0x4E, 0x0C01, 0x0E06);
    Row = 16;
    Col = 0;
    SAY("ScreenScroll up "); Cells(12 * 80, 3); Cells(14 * 80, 3);
    Cells(14 * 80 + 6, 2);
    Line();
    ScreenScroll(1, 2, 0x17, 0x0C01, 0x0E06);
    SAY("down "); Cells(12 * 80, 3); Cells(14 * 80, 3);
    ScreenScroll(0, 0, 0x60, 0x0C02, 0x0D03);
    SAY("none "); Cells(12 * 80 + 1, 3); Cells(14 * 80 + 1, 3);
    Line();
    ScreenSetMode(0x83);                /* without clearing the screen */
    screen = ScreenState();
    SAY("ScreenSetMode 83, ScreenState ");
    H4(screen.mode); H4(screen.columns); H4(screen.page);
    Cells(6 * 80 + 70, 2);
    CARET("CursorWhere", page);
    Line();
    ScreenMemory(7, 0x0010);            /* the display of one colour */
    SAY("ScreenMemory 7,10: "); H8(md.Screen);
    ScreenMemory(3, 160);               /* from the second row on */
    Cells(5 * 80 + 70, 2);
    ScreenMemory(3, 0);
    Line();
}

/* The font of 8 lines, which gives the screen 50 rows. */
static void Small(void)
{
    struct Screen screen;
    struct Font font;

    ScreenSmallFont();
    font = ScreenFont();
    SAY("ScreenSmallFont, ScreenFont "); H4(font.height); H4(font.last);
    screen = ScreenState();
    SAY("ScreenState "); H4(screen.mode); H4(screen.columns);
    H4(screen.page);
    ScreenPrintAll();
    SAY("ScreenPrintAll");
    CARET(", CursorWhere", screen.page);
    Line();
    CursorTo(screen.page, 0x3000);      /* row 48 */
    CARET("CursorTo 3000:", screen.page);
    Line();
}

/* The mouse. */
static void Where(const u8 FAR *name)
{
    struct MouseAt at;

    at = MouseWhere();
    Put(name);
    Cell(' ');
    H4(at.buttons);
    H4(at.x);
    H4(at.y);
}

#define WHERE(name) { TEXT(said, name); Where(said); }

static void Pointer(void)
{
    struct Font font;

    font = ScreenFont();
    SAY("ScreenSetMode 3, ScreenFont "); H4(font.height); H4(font.last);
    Line();
    SAY("MouseReset "); H4(MouseReset());
    SAY("MouseFind "); H4(MouseFind());
    WHERE("MouseWhere");
    MouseRows(0, 199);
    MouseTo(320, 96);
    WHERE("MouseRows 0,199, MouseTo 320,96:");
    Line();
    MouseTo(0x7FFF, 0x7FFF);
    WHERE("MouseTo 7FFF,7FFF:");
    MouseRows(16, 31);
    WHERE("MouseRows 16,31:");
    MouseTo(0, 0);
    WHERE("MouseTo 0,0:");
    Line();
    MouseRows(0, 199);
    MouseTo(79 * 8, 20 * 8);
    ScreenMemory(3, 0);
    DISPLAY[20 * 80 + 79] = 0x1E4D;
    MousePointer(0xFFFF, 0x7700);
    MouseShow();
    SAY("MousePointer FFFF,7700, MouseShow "); Cells(20 * 80 + 79, 1);
    MouseHide();
    SAY("MouseHide "); Cells(20 * 80 + 79, 1);
    MouseHide();
    MouseShow();
    SAY("hidden twice and shown once "); Cells(20 * 80 + 79, 1);
    Line();
    MouseShow();
    SAY("shown twice "); Cells(20 * 80 + 79, 1);
    MousePointer(0xFF00, 0x7700 | '#');
    SAY("MousePointer FF00,7723: "); Cells(20 * 80 + 79, 1);
    MouseHide();
    SAY("MouseHide "); Cells(20 * 80 + 79, 1);
    MousePointer(0xFFFF, 0x7700);
    Line();
}

/* The keys that are held, and Ins taken for let go. */
static void Keyboard(void)
{
    u8 FAR *held = MACHINE(u8, 0x40, 0x18);

    SAY("KeysHeld "); H2(KeysHeld());
    *held |= 0x80;
    KeyForgetIns();
    SAY("KeyForgetIns: "); H2(*held & 0x80);
    Line();
}

TEXT(Mask_S, "TOY?.DAT");
TEXT(Toy1_S, "TOY1.DAT");
TEXT(Lower_S, "hello.txt");

/* What the toy has the system ask about a device that fails: it has
   the call fail. No device of the toy's does. */
static u16 Fail(u16 how, u16 error, const u8 FAR *device)
{
    (void)how;
    (void)error;
    (void)device;
    return DEVICE_FAIL;
}

/* The functions that dos32.c does not make yet: both.py knows the page
   by its first line. */
static void Later(void)
{
    struct Block block;
    struct Answer asked;
    const u8 FAR *tail;
    u16 at, taken;

    SAY("Not made in dos32.c yet:");
    Line();
    for (at = 0; at < 200; at++)
        SMALL[at] = 0;
    ANSWER("FileTrueName", FileTrueName(Hello_S, SMALL));
    Quoted(SMALL, 64);
    ANSWER("FileTrueName, lower case:", FileTrueName(Lower_S, SMALL));
    Quoted(SMALL, 64);
    Line();
    asked = FileCreate(Toy1_S, 0);
    FileClose(asked.value);
    asked = FileCreate(Toy2_S, 0);
    FileClose(asked.value);
    SAY("FcbParse "); H4(FcbParse(Mask_S, SMALL + 128, 1));
    H2(SMALL[128]); Quoted(SMALL + 129, 11);
    SAY("FcbParse, a name: "); H4(FcbParse(Hello_S, SMALL + 128, 1));
    H2(SMALL[128]); Quoted(SMALL + 129, 11);
    Line();
    FcbParse(Mask_S, SMALL + 128, 1);
    ANSWER("FcbDelete", FcbDelete(SMALL + 128, 0));
    ATTR("FileAttributes of one:", FileAttributes(Toy2_S));
    ANSWER("FcbDelete again:", FcbDelete(SMALL + 128, 0));
    Line();
    ANSWER("FcbDelete with the flag:", FcbDelete(SMALL + 128, 1));
    FileDelete(Toy1_S);
    FileDelete(Toy2_S);
    Line();
    for (at = 0; at < sizeof md.Country; at++)
        ((u8 *)&md.Country)[at] = 0;
    CountryInfo(&md.Country);
    SAY("CountryInfo "); H4(md.Country.DateFmt);
    Quoted(md.Country.Sep1000, 2); Quoted(md.Country.DecimalSep, 2);
    Quoted(md.Country.DateSep, 2); Quoted(md.Country.TimeSep, 2);
    SAY("CaseMap "); H4(CaseMap(0x1200 | 'a')); H4(CaseMap(0x84));
    H4(CaseMap(0xA0)); H4(CaseMap(0x3481));
    Line();
    /* How much memory there is follows the program's length: only that
       it is plausible is written. */
    block = MemTake(0xFFFF);
    SAY("MemTake FFFF: "); H4(block.value); H4(block.failed);
    SAY("at least 256 KB and no more than 640 ");
    H2(block.largest >= 0x4000 && block.largest <= 0xA000);
    Line();
    block = MemTake(0x100);
    taken = block.value;
    SAY("MemTake 100: it is given "); H2(block.failed == 0 && taken != 0);
    H4(block.failed); H4(block.largest);
    block = MemResize(taken, 0x200);
    SAY("MemResize 200: "); H4(block.failed); H4(block.largest);
    block = MemResize(taken, 0xFFFF);
    SAY("FFFF: "); H4(block.value); H4(block.failed);
    Line();
    ANSWER("MemGive", MemGive(taken));
    ANSWER("again:", MemGive(taken));
    MemSetSearch(0);
    MemSetUpper(0);
    SAY("MemSetSearch, MemSetUpper");
    Line();
    /* What the PSP says: the tail that the toy was started with, as
       far as its carriage return; and the first text of its
       environment. */
    tail = ProgramTail();
    SAY("ProgramTail '");
    for (at = 0; at < 20 && tail[at] != '\r'; at++)
        Cell(tail[at] < ' ' ? '.' : tail[at]);
    Cell('\'');
    Line();
    SAY("ProgramEnvironment "); Quoted(ProgramEnvironment(), 40);
    Line();
    /* What the system enters of the program, noted, taken and given
       back in the order in which the program asks for that. Nothing
       shows of it until a device fails or a key is pressed, which is
       for a toy of the piece that makes them in dos32.c. */
    DeviceErrorNote();
    CtrlBreakNote();
    DeviceErrorTake(Fail);
    CtrlBreakTake();
    CtrlCTake();
    DeviceErrorGive();
    CtrlBreakGive();
    SAY("DeviceError and CtrlBreak noted, taken, given back; CtrlCTake");
    Line();
}

/* The pages, a step of the script behind each.

   The toy is entered where the program is, at Alloc0 of dos.c in 16
   bits and at Start of dos32.c in 32, each of which calls Main. The
   first thing asked is what the program asks first, that it keep no
   more of the memory than it is: in 16 bits there is none to ask for
   otherwise. */
void Main(void)
{
    Owned = MemOwn().failed;
    Clear();
    System();
    Page(0);
    Files();
    Page(0);
    Names();
    Page(0);
    Directories();
    Page(0);
    Finds();
    Page(1);
    Display();
    Page(0);
    Small();
    Page(1);
    Pointer();
    Page(0);
    Keyboard();
    Page(0);
    Later();
    Page(0);
    SAY("ProgramEnd 7");
    Line();
    CursorTo(0, Row << 8);
    ProgramEnd(7);
}
