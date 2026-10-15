/* The functions of the portable C's sys.h, each asked and what it
   answers written onto the screen: the toy by which sys_dos.c, the
   functions of the program, a flat one of 32 bits, was held against
   dos.c, those of the program of 16 bits that the portable C was
   before. It was one source built both ways, and is built one way now
   (build.sh):

     sys-32.exe   by wcc386, with src/portable_c/sys_dos.c and stack_dos.c, and
                  DOS/32A for its stub.

   The two had to write the same on the bench, a screen for a screen,
   and did when the 16-bit build went; asked.py runs the one that is
   left and prints its pages. What is asked is what the program asks,
   and what it could: a call that fails, a file that is not there, a
   drive that there is not, a count of none. What is said below of 16
   bits is of then.

   It writes a page at a time and then waits as the program waits, with
   the idle call, a look at the mouse and a look for a key, so each page
   is the screen of one step of a script, and the next page begins with
   what turned it: the key as KeyWaiting and KeyRead had it and what
   KeysHeld said then, or where the mouse was when a button went down.
   The steps are asked.py's STEPS, one behind each page but the last.

   An item is a name and what was answered, in hex: of a struct Answer
   the value and, behind a slash, whether it failed. What turns out
   otherwise from one run to the next is not written: a time of day is
   written as far as its minute, and of a number that follows the
   machine or the length of the program only whether it is plausible.

   The toy has no C library, as the program has none. It writes straight
   into the memory of the display, so that what the functions for the
   screen do shows beside it. It is written as the program of 16 bits
   let it be written, where the main data filled the segment that DS was
   on all but for 2.9 KB: a text of the toy is a constant, by the macros
   below, its buffers are those of the main data, and it has no pointer
   to a local variable, which was one into another segment there.

   It also looks, before each thing that it writes, whether
   ES is still on the data and the direction flag still clear, which the
   compiler takes them to be and an interrupt through the extender might
   not leave them: `strayed` and a number on the screen says that one
   was not, 400h for the flag and anything else for ES.

   The pages behind the first eight are of what a program of 32 bits has
   to do otherwise than one of 16, each in a function below that says
   what it asks: the memory in blocks and the PSP; the calls that are
   made in real mode; other programs, with a second copy of the toy
   itself, which is the toy started with something behind its name; the
   critical error, where the toy is asked from inside a call of DOS and
   a step of the script ends there; Ctrl-Break and Ctrl-C.

   The last page is of the program ended by another's hand: its end
   taken and given back, with what the PSP names meanwhile, and taken
   once more and left so where the toy ends. DOS then goes on where it
   does when another program has it end this one, and the step behind
   the toy's last asks the shell what DOS has been told of Ctrl-C there.
   harness/trial32/ended.asm has DOS end a program by another's hand,
   and prints the registers that it gets back. */
#include "vc.h"

/* The main data, which the functions keep two things in: ErrDat and
   Screen. */
struct MainDat md;

/* A place of the machine's memory, by its segment and offset in real
   mode. */
#define MACHINE(type, segment, offset) \
    ((type *)(((u32)(segment) << 4) + (offset)))
#define DISPLAY MACHINE(u16, 0xB800, 0)

/* A text of the toy; and one written out. */
#define TEXT(name, text) static const u8 name[] = text
#define SAY(text) { TEXT(said, text); Put(said); }

/* The buffers. BIG is the copies of the screen that the main data has
   one behind the other, 38,016 bytes of them. */
#define BIG      ((u8 *)md.ScrBuf)
#define BIG_SIZE 30000
#define BIGGER   0x18000                /* more than DOS takes at once */
#define SMALL    md.TmpBuf

static u16 Row, Col;

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

static void Put(const u8 *text)
{
    u32 strayed = Strayed();

    if (strayed != 0) {
        TEXT(said, "strayed ");
        const u8 *at = said;

        while (*at != 0)
            Cell(*at++);
        Digits(strayed, 4);
        Cell(' ');
    }
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
static void Quoted(const u8 *text, u16 most)
{
    Cell('\'');
    for (; *text != 0 && most != 0; text++, most--)
        Cell(*text < ' ' ? '.' : *text);
    Cell('\'');
    Cell(' ');
}

/* What a function answered, behind its name. */
static void Answer(const u8 *name, struct Answer answer)
{
    Put(name);
    Cell(' ');
    Digits(answer.value, 4);
    Cell('/');
    Hex(answer.failed, 1);
}

#define ANSWER(name, call) { TEXT(said, name); Answer(said, call); }

/* What reading or writing answered, behind its name: the count, and
   behind a slash whether it failed and the error. */
static void Moved(const u8 *name, struct Moved moved)
{
    Put(name);
    Cell(' ');
    Digits((u32)moved.count, 8);
    Cell('/');
    Hex(moved.failed, 1);
    Cell(' ');
    Digits(moved.error, 4);
    Cell(' ');
}

#define MOVED(name, call) { TEXT(said, name); Moved(said, call); }

static void Copy(u8 *to, const u8 *from, u16 count)
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
    bool was;

    SAY("MemOwn failed "); H4(Owned);
    version = SystemVersion();
    SAY("SystemVersion "); H4(version.version); H4(version.other);
    version = SystemTrueVersion();
    SAY("SystemTrueVersion "); H4(version.version); H4(version.other);
    Line();
    was = BreakCheck();
    SAY("BreakCheck "); H4(was);
    SetBreakCheck(true);
    SAY("SetBreakCheck 1: "); H4(BreakCheck());
    SetBreakCheck(false);
    SAY("0: "); H4(BreakCheck());
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

static void Place(const u8 *name, struct FilePlace place)
{
    Put(name);
    Cell(' ');
    Digits(place.place, 8);
    Cell('/');
    Hex(place.failed, 1);
}

#define PLACE(name, call) { TEXT(said, name); Place(said, call); }

static void Attr(const u8 *name, struct FileAttr attr)
{
    Put(name);
    Cell(' ');
    Digits(attr.value, 4);
    Cell('/');
    Hex(attr.failed, 1);
    H4(attr.attributes);
}

#define ATTR(name, call) { TEXT(said, name); Attr(said, call); }

static void Stamp(const u8 *name, struct FileStamp stamp)
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
    MOVED("FileWrite 30000:", FileWrite(handle, BIG, BIG_SIZE));
    PLACE("FileSeek 100:", FileSeek(handle, 100));
    PLACE("FileStep 2,0:", FileStep(handle, 2, 0));
    Line();
    PLACE("FileStep 2,-10:", FileStep(handle, 2, -10));
    PLACE("1,5:", FileStep(handle, 1, 5));
    PLACE("FileSeek 12345678h:", FileSeek(handle, 0x12345678UL));
    Line();
    PLACE("FileStep 3,89ABh:", FileStep(handle, 3, 0x89AB));
    PLACE("FileSeek, no handle:", FileSeek(99, 0x12345UL));
    PLACE("20000:", FileSeek(handle, 20000));
    Line();
    MOVED("FileWrite 0:", FileWrite(handle, BIG, 0));
    PLACE("FileStep 2,0:", FileStep(handle, 2, 0));
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
    MOVED("FileRead 30000:", FileRead(handle, BIG, BIG_SIZE));
    for (at = wrong = 0; at < 20000; at++)
        wrong += BIG[at] != Pattern(at);
    for (; at < BIG_SIZE; at++)
        wrong += BIG[at] != 0;
    SAY("wrong bytes "); H4(wrong);
    MOVED("at the end:", FileRead(handle, BIG, 100));
    Line();
    STAMP("FileTime", FileTime(handle));
    PLACE("FileSeek 300:", FileSeek(handle, 300));
    MOVED("FileReadBare 5:", FileReadBare(handle, SMALL, 5));
    H2(SMALL[0]); H2(SMALL[4]); H2(Pattern(300)); H2(Pattern(304));
    Line();
    MOVED("FileReadBare, no handle:", FileReadBare(99, SMALL, 5));
    MOVED("FileRead, no handle:", FileRead(99, SMALL, 5));
    MOVED("FileWrite, opened to read:", FileWrite(handle, SMALL, 5));
    Line();
    MOVED("FileWrite, no handle:", FileWrite(99, SMALL, 5));
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
    MOVED("FileReadBare 30:", FileReadBare(handle, SMALL, 30));
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
static void Listed(const u8 *pattern, u16 attributes, u16 stamped)
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
static void Caret(const u8 *name, u16 page)
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

/* The mouse. */
static void Where(const u8 *name)
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
    u8 *held = MACHINE(u8, 0x40, 0x18);

    SAY("KeysHeld "); H2(KeysHeld());
    *held |= 0x80;
    KeyForgetIns();
    SAY("KeyForgetIns: "); H2(*held & 0x80);
    Line();
}


TEXT(Comspec_S, "COMSPEC=");
TEXT(OnA_S, "A:\\X.TMP");
TEXT(Ran_S, "RAN.TXT");
TEXT(Echo_S, "/C ECHO ran>RAN.TXT");
TEXT(Run_S, "/C ");
TEXT(Two_S, " 2");
TEXT(Three_S, " 3");
TEXT(No_S, "");

/* The text of the environment that begins with `name`, behind that; or
   an empty one. */
static const u8 *Variable(const u8 *name)
{
    const u8 *text = ProgramEnvironment();
    u16 at;

    while (*text != 0) {
        for (at = 0; name[at] != 0 && text[at] == name[at]; at++)
            ;
        if (name[at] == 0)
            return text + at;
        while (*text++ != 0)
            ;
    }
    return text;
}

/* The path of the toy's own file, which DOS has behind the end of the
   environment. */
static const u8 *Self(void)
{
    const u8 *text = ProgramEnvironment();

    while (text[0] != 0 || text[1] != 0)
        text++;
    return text + 4;
}

/* A command tail made of three texts, in the small buffer from 128 on:
   a count, the characters, a carriage return. */
static u8 *Tail(const u8 *first, const u8 *second,
                const u8 *third)
{
    u8 *tail = SMALL + 128;
    u16 count = 0;

    while (*first != 0)
        tail[++count] = *first++;
    while (*second != 0)
        tail[++count] = *second++;
    while (*third != 0)
        tail[++count] = *third++;
    tail[0] = (u8)count;
    tail[count + 1] = '\r';
    return tail;
}

/* Other programs. The shell is run for a command that leaves a file,
   bare, as the program runs its commands, and a file that is not there
   is run both ways. Tails and commands are in the main data, as the
   program's are. */
static void Others(void)
{
    const u8 *shell = Variable(Comspec_S);
    struct Answer asked;
    u16 at;

    SAY("the shell is "); Quoted(shell, 40);
    Line();
    ANSWER("ProgramRunBare of it, for a command:",
           ProgramRunBare(shell, Tail(Echo_S, No_S, No_S)));
    for (at = 0; at < 16; at++)
        SMALL[at] = 0;
    asked = FileOpen(Ran_S, 0);
    FileRead(asked.value, SMALL, 16);
    FileClose(asked.value);
    SAY("the command wrote "); Quoted(SMALL, 16);
    if (ProgramOfFile(Ran_S) == 0)
        SAY("no program; ")
    else
        SAY("a program; ")
    ANSWER("ProgramOpenFile of that:", ProgramOpenFile(Ran_S));
    ANSWER("FileDelete of that:", FileDelete(Ran_S));
    Line();
    ANSWER("ProgramRun of a file that is not there:",
           ProgramRun(None_S, Tail(No_S, No_S, No_S)));
    ANSWER("ProgramRunBare:",
           ProgramRunBare(None_S, Tail(No_S, No_S, No_S)));
    Line();
}

/* The toy's own file run with something behind its name, by itself and
   then by the shell, as a command: each time the second copy writes a
   page of its own, waits for a key as the first does, and ends, and the
   first writes what it was answered when both have. Run by itself, the
   copy has the environment that DOS made for it of this one's; run by
   the shell, it has what the shell gave it. */
static void Copies(void)
{
    struct Answer first, second;

    first = ProgramRun(Self(), Tail(Two_S, No_S, No_S));
    second = ProgramRun(Variable(Comspec_S), Tail(Run_S, Self(), Three_S));
    Clear();
    ANSWER("ProgramRun of a second copy:", first);
    ANSWER("and of the shell for one:", second);
    Line();
}

/* What a second copy of the toy does, which is one that was started
   with something behind its name: it writes that, what its environment
   and the end of it say, and whether it has memory, waits for a key and
   ends with 5. */
static void Second(void)
{
    const u8 *tail = ProgramTail();
    struct Block block;
    u16 at;

    if (tail[1] == '4') {
        Row = 12;
        SAY("the copy: FileCreate there:");
        ANSWER("", FileCreate(OnA_S, 0));
        Line();
        ProgramEnd(4);
    }
    Clear();
    SAY("a second copy, ProgramTail '");
    for (at = 0; at < 20 && tail[at] != '\r'; at++)
        Cell(tail[at] < ' ' ? '.' : tail[at]);
    Cell('\'');
    Line();
    SAY("its shell is "); Quoted(Variable(Comspec_S), 40);
    SAY("and its own file "); Quoted(Self(), 40);
    Line();
    block = MemTake(0xFFFF);
    SAY("MemTake FFFF: "); H4(block.value); H4(block.failed);
    SAY("at least 128 KB and no more than 640 ");
    H2(block.largest >= 0x2000 && block.largest <= 0xA000);
    Line();
    Page(0);
    ProgramEnd(5);
}

TEXT(Mask_S, "TOY?.DAT");
TEXT(Toy1_S, "TOY1.DAT");
TEXT(Lower_S, "hello.txt");
TEXT(Dots_S, "..\\WORK\\.\\hello.txt");
TEXT(NoDrive_S, "Q:X.DAT");
TEXT(Drive_S, "C:TOY?.DAT");
TEXT(Star_S, "*.*");

/* The calls of DOS that the extender does not know: the full path of a
   name, the files by a file control block, the country and its
   capitals. Each buffer is filled first, so that what DOS leaves of it
   shows too. */
static void Older(void)
{
    struct Answer asked;
    u8 *fcb = SMALL + 128;
    u16 at;

    for (at = 0; at < 128; at++)
        SMALL[at] = '#';
    SMALL[127] = 0;
    ANSWER("FileTrueName", FileTrueName(Hello_S, SMALL));
    Quoted(SMALL, 20);
    ANSWER("lower case:", FileTrueName(Lower_S, SMALL));
    Quoted(SMALL, 20);
    Line();
    ANSWER("with dots:", FileTrueName(Dots_S, SMALL));
    Quoted(SMALL, 20);
    ANSWER("no such directory:", FileTrueName(NoDir_S, SMALL));
    Quoted(SMALL, 20);
    Line();
    for (at = 0; at < 127; at++)
        SMALL[at] = '#';
    ANSWER("no such drive:", FileTrueName(NoDrive_S, SMALL));
    Quoted(SMALL, 20);
    SAY("and its end "); Quoted(SMALL + 120, 8);
    Line();
    asked = FileCreate(Toy1_S, 0);
    FileClose(asked.value);
    asked = FileCreate(Toy2_S, 0);
    FileClose(asked.value);
    for (at = 0; at < 48; at++)
        fcb[at] = '#';
    SAY("FcbParse "); H4(FcbParse(Mask_S, fcb, 1));
    H2(fcb[0]); Quoted(fcb + 1, 11);
    for (at = 12; at < 20; at++)
        H2(fcb[at]);
    Line();
    SAY("a name: "); H4(FcbParse(Hello_S, fcb, 1));
    H2(fcb[0]); Quoted(fcb + 1, 11);
    SAY("with a drive: "); H4(FcbParse(Drive_S, fcb, 1));
    H2(fcb[0]); Quoted(fcb + 1, 11);
    Line();
    SAY("a drive that there is not: "); H4(FcbParse(NoDrive_S, fcb, 1));
    H2(fcb[0]); Quoted(fcb + 1, 11);
    fcb[0] = 2;
    SAY("its own drive kept: "); H4(FcbParse(Star_S, fcb, 2));
    H2(fcb[0]); Quoted(fcb + 1, 11);
    Line();
    for (at = 0; at < 48; at++)
        fcb[at] = 0;
    FcbParse(Mask_S, fcb, 1);
    ANSWER("FcbDelete", FcbDelete(fcb, 0));
    ATTR("FileAttributes of one:", FileAttributes(Toy2_S));
    ANSWER("FcbDelete again:", FcbDelete(fcb, 0));
    Line();
    ANSWER("FcbDelete with the flag:", FcbDelete(fcb, 1));
    /* An extended one, as the program deletes: with the attributes of
       what is to be deleted in front. */
    asked = FileCreate(Toy1_S, 0);
    FileClose(asked.value);
    fcb[0] = 0xFF;
    fcb[6] = 0x26;
    fcb[7] = 0;
    FcbParse(Mask_S, fcb + 7, 2);
    ANSWER("an extended one:", FcbDelete(fcb, 0));
    ATTR("FileAttributes:", FileAttributes(Toy1_S));
    Line();
    FileDelete(Toy1_S);
    FileDelete(Toy2_S);
    /* The country: 34 bytes as DOS has them, of which the structure has
       32, and the two behind it are the first of the search's record. */
    for (at = 0; at < 34; at++)
        ((u8 *)&md.Country)[at] = '#';
    CountryInfo(&md.Country);
    SAY("CountryInfo "); H4(md.Country.DateFmt);
    Quoted(md.Country.Sep1000, 2); Quoted(md.Country.DecimalSep, 2);
    Quoted(md.Country.DateSep, 2); Quoted(md.Country.TimeSep, 2);
    Line();
    for (at = 0; at < 34; at++)
        Digits(((const u8 *)&md.Country)[at], 2);
    Line();
    SAY("CaseMap "); H4(CaseMap(0x1200 | 'a')); H4(CaseMap(0x84));
    H4(CaseMap(0xA0)); H4(CaseMap(0x3481)); H4(CaseMap(0xE0));
    H4(CaseMap(0xFF)); H4(CaseMap(0x7F));
    Line();
}

/* More than DOS takes at one call, 18000h bytes, written to a file at
   once and read back at once, in a block of the memory: sys_dos.c asks
   DOS a piece at a time for that, which the program never has it do. */
static void Bigger(void)
{
    struct Block block;
    struct Answer asked;
    u8 *big;
    u16 handle;
    i32 at, wrong;

    block = MemTake(BIGGER);
    big = block.block;
    SAY("MemTake 18000h: "); H4(block.failed);
    if (block.failed) {
        Line();
        return;
    }
    for (at = 0; at < BIGGER; at++)
        big[at] = Pattern((u16)(at + at / 0x10000));
    asked = FileCreate(Toy_S, 0);
    handle = asked.value;
    ANSWER(" FileCreate", asked);
    Line();
    MOVED("FileWrite 18000h:", FileWrite(handle, big, BIGGER));
    PLACE(" FileSeek 0:", FileSeek(handle, 0));
    Line();
    for (at = 0; at < BIGGER; at++)
        big[at] = 0;
    MOVED("FileRead 18000h:", FileRead(handle, big, BIGGER));
    for (at = wrong = 0; at < BIGGER; at++)
        wrong += big[at] != Pattern((u16)(at + at / 0x10000));
    SAY(" wrong bytes "); H8((u32)wrong);
    Line();
    MOVED("FileReadBare at the end:", FileReadBare(handle, big, BIGGER));
    PLACE(" FileSeek 10000h:", FileSeek(handle, 0x10000));
    MOVED(" FileWrite 0:", FileWrite(handle, big, 0));
    Line();
    PLACE("FileStep 2,0:", FileStep(handle, 2, 0));
    ANSWER(" FileClose", FileClose(handle));
    ANSWER(" FileDelete", FileDelete(Toy_S));
    ANSWER(" MemGive", MemGive(big));
    Line();
}

/* What a block of the memory is answered with, behind its name: the
   error if it failed, and else how far what is answered is behind the
   block `from`, in bytes; and the bytes that it says. */
static void Block(const u8 *name, struct Block block, const u8 *from)
{
    Put(name);
    Cell(' ');
    Hex(block.failed ? block.value : (u32)(block.block - from), 8);
    Cell('/');
    Hex(block.failed, 4);
    H8((u32)block.largest);
}

#define BLOCK(name, call, from) { TEXT(said, name); Block(said, call, from); }

/* The memory, in blocks of the extender's that are counted in bytes, and
   what the PSP says. How much memory there is and where a block lies
   follow the machine and the extender: of the first only that it is
   plausible is written, and of the second how far a block is behind the
   first one. The first is all that there is, as the program asks for
   it, and is written to at both ends. Until the piece S this page asked
   DOS's blocks by their segments, as the functions did before F9. */
static void Memory(void)
{
    struct Block block;
    const u8 *text;
    u8 *first, *second, *third;
    i32 all;
    u16 at;

    block = MemTake(0x7FFFFFFF);
    all = block.largest;
    SAY("MemTake 7FFFFFFF: "); H4(block.value); H4(block.failed);
    SAY("at least 1 MB "); H2(all >= 0x100000);
    Line();
    block = MemTake(all);
    first = block.block;
    SAY("MemTake of all that: "); H4(block.failed);
    H2(block.largest == all);
    first[0] = 0x12;
    first[all - 1] = 0x34;
    SAY("written at its ends: "); H2(first[0]); H2(first[all - 1]);
    Line();
    block = MemResize(first, 0x1000);
    BLOCK("MemResize of it to 1000:", block, first);
    Line();
    if (!block.failed)                  /* a block may move */
        first = block.block;
    block = MemTake(0x800);
    second = block.block;
    SAY("MemTake 800: "); H4(block.failed);
    block = MemTake(0x400);
    third = block.block;
    SAY("MemTake 400: "); H4(block.failed);
    Line();
    block = MemTake(0x7FFFFFFF);
    SAY("what is left is less "); H2(block.largest < all);
    ANSWER("MemGive of the second:", MemGive(second));
    Line();
    block = MemResize(first, 0x2000);
    BLOCK("MemResize of the first to 2000:", block, first);
    if (!block.failed)
        first = block.block;
    block = MemResize(first, 0x80);
    BLOCK(" to 80:", block, first);
    if (!block.failed)
        first = block.block;
    Line();
    /* A block that is none. */
    ANSWER("MemGive of what is no block:", MemGive(first + 1));
    BLOCK("MemResize of it:", MemResize(first + 1, 1), first);
    Line();
    ANSWER("MemGive of the third:", MemGive(third));
    ANSWER("of the first:", MemGive(first));
    ANSWER("again:", MemGive(first));
    Line();
    block = MemTake(0x7FFFFFFF);
    SAY("all is left again "); H2(block.largest == all);
    Line();
    Bigger();
    MemSetSearch(0);
    MemSetUpper(0);
    SAY("MemSetSearch, MemSetUpper");
    Line();
    /* What the PSP says: the tail that the toy was started with, as far
       as its carriage return; the first text of its environment; and
       what is behind the environment's end, where the program looks for
       the path of its own file. */
    text = ProgramTail();
    SAY("ProgramTail '");
    for (at = 0; at < 20 && text[at] != '\r'; at++)
        Cell(text[at] < ' ' ? '.' : text[at]);
    Cell('\'');
    Line();
    text = ProgramEnvironment();
    SAY("ProgramEnvironment "); Quoted(text, 40);
    for (at = 0; at < 0x7FFF && (text[at] != 0 || text[at + 1] != 0); at++)
        ;
    SAY("and behind its end "); H2(text[at + 2]); H2(text[at + 3]);
    Quoted(text + at + 4, 40);
    Line();
}

/* Wait as the program does, for a key, and read it. */
static u16 Key(void)
{
    struct Keys waiting;

    for (;;) {
        Idle();
        MouseWhere();
        waiting = KeyWaiting();
        if (!waiting.none)
            return KeyRead();
    }
}

/* What the toy has the system ask about a device that fails. It does
   what the program does there, in small: it lets the machine interrupt
   again, writes what it is asked onto the screen, waits for a key as
   the program waits, by the idle call, the mouse and the keyboard, and
   answers by the key. r has the system try again. f has the call fail
   and notes that in ErrDat of the main data, as the program does when
   the user gives up, by which the call counts as failed with the error
   53h. Anything else has the call fail and notes nothing, and the
   error is then what DOS says of the call. A step of the script ends
   inside this, with the system in the middle of the call. */
static u16 Asked;

static u16 Device(u16 how, u16 error, const u8 *device)
{
    u16 key;

    Asked++;
    IntOn();
    Line();
    SAY("  asked "); H4(how); H4(error);
    if (device == 0) {
        SAY("of a drive, ");
    } else {
        SAY("of the device "); Quoted(device, 8);
    }
    key = Key();
    SAY("answered by "); H4(key);
    Line();
    if ((u8)key == 'r')
        return DEVICE_RETRY;
    if ((u8)key == 'f')
        md.ErrDat = 1;
    return DEVICE_FAIL;
}

TEXT(Com2_S, "COM2");
TEXT(Copy_S, "/C COPY HELLO.TXT A:\\X.TMP");
TEXT(Four_S, " 4");

/* What the system enters of the program: the critical error, with the
   toy asked from inside a call of DOS, and waiting there for a key.
   The three interrupts are noted and taken in the order in which the
   program does that. Each line of "asked" is a step of the script. */
static void Devices(void)
{
    struct Answer asked;

    BreakCheck();
    SetBreakCheck(0);
    CtrlCTake();
    DeviceErrorNote();
    CtrlBreakNote();
    DeviceErrorTake(Device);
    CtrlBreakTake();
    SAY("the critical error, Ctrl-Break and Ctrl-C are taken");
    Line();
    SAY("FileCreate on a diskette that cannot be written:");
    ANSWER("", FileCreate(OnA_S, 0));
    SAY("asked "); H4(Asked);
    Line();
    SAY("again, failed without a note:");
    ANSWER("", FileCreate(OnA_S, 0));
    SAY("asked "); H4(Asked);
    Line();
    /* A call that the 32-bit program makes in real mode. */
    SAY("FileSetAttributes of the shell's file:");
    ANSWER("", FileSetAttributes(Variable(Comspec_S), 0));
    SAY("asked "); H4(Asked);
    Line();
    /* A device that is no drive: the second serial port, which the
       bench has not. */
    asked = FileOpen(Com2_S, 1);
    SAY("FileWrite to COM2:");
    MOVED("", FileWrite(asked.value, Com2_S, 1));
    SAY("asked "); H4(Asked);
    FileClose(asked.value);
    Line();
}

/* The same from a program that the toy has run and that has not taken
   the question itself: the shell, for a command that writes to the
   diskette, and a second copy of the toy that makes a file there. DOS
   asks whoever has the critical error, which is the toy, in the middle
   of a call that is not the toy's. */
static void Children(void)
{
    struct Answer ran;

    CursorTo(0, 20 << 8);
    SAY("the shell run for a copy to the diskette:");
    ran = ProgramRunBare(Variable(Comspec_S), Tail(Copy_S, No_S, No_S));
    Line();
    ANSWER("it came back", ran);
    SAY("asked "); H4(Asked);
    Line();
    SAY("a second copy run that makes a file there:");
    ran = ProgramRun(Self(), Tail(Four_S, No_S, No_S));
    Line();
    ANSWER("it came back", ran);
    SAY("asked "); H4(Asked);
    Line();
}

/* Ctrl-Break, which the driver sends when the page has been written:
   the BIOS calls its interrupt, which the toy has taken, and nothing
   happens but that the BIOS has a key of no character for the next
   page. And Ctrl-C: the toy has DOS look for it at every call and
   waits until the BIOS has the key, without reading it; DOS finds it
   at the next call, calls its interrupt, which the toy has taken, and
   the toy goes on. */
static void Breaks(void)
{
    struct Keys waiting;

    SAY("Ctrl-Break came and went. Now Ctrl-C, with DOS looking for it");
    Line();
    SetBreakCheck(1);
    CursorTo(0, 10 << 8);
    do {
        Idle();
        MouseWhere();
        waiting = KeyWaiting();
    } while (waiting.none);
    SAY("the BIOS has the key "); H4(waiting.key);
    TimeOfDay();
    SAY("DOS was asked the time, and the toy goes on. BreakCheck ");
    H4(BreakCheck());
    SetBreakCheck(0);
    waiting = KeyWaiting();
    SAY("KeyWaiting "); H4(waiting.key); H4(waiting.none);
    while (!KeyWaiting().none)
        KeyRead();
    Line();
}

/* The program ended by another's hand. The system has the PSP name a
   routine of its own for where DOS goes on, at 0Ah, while the program
   has taken its end: the address there is read, and whether it is
   another or as it was is written, since the address itself follows
   the build. Taking twice takes once, and so does giving back.

   Then the end is taken once more and left so, with DOS to be told to
   look for Ctrl-C at every call, which Breaks above has switched off:
   Main ends the toy without giving its end back, DOS goes on in the
   system's routine as if another program had ended the toy, and the
   shell is asked afterwards, by the last step of the script, what DOS
   does. */
static void Ended(void)
{
    const u32 *goes = (const u32 *)(ProgramTail() - 0x81 + 0x0A);
    u32 was = *goes, taken;

    SAY("The program ended by another's hand, what the PSP names:");
    Line();
    ProgramEndedBreakCheck(0);
    ProgramEndedTake();
    taken = *goes;
    SAY("taken, another "); H2(taken != was);
    ProgramEndedBreakCheck(1);
    ProgramEndedTake();
    SAY("taken again, the same "); H2(*goes == taken);
    ProgramEndedGive();
    SAY("given back, as it was "); H2(*goes == was);
    ProgramEndedGive();
    SAY("given back again "); H2(*goes == was);
    Line();
    ProgramEndedTake();
    SAY("taken and left so, the same as before "); H2(*goes == taken);
    SAY("BreakCheck "); H4(BreakCheck());
    Line();
}

/* The pages, a step of the script behind each.

   The toy is entered where the program is, at Start of sys_dos.c, which
   calls Main. The first thing asked is what the program asks first,
   that it keep no more of the memory than it is. */
void Main(void)
{
    Owned = MemOwn().failed;
    if (*ProgramTail() != '\r')
        Second();
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
    Page(1);
    Pointer();
    Page(0);
    Keyboard();
    Page(0);
    Memory();
    Page(0);
    Older();
    Page(0);
    Others();
    Page(0);
    Copies();
    Page(0);
    Devices();
    Page(0);
    Children();
    Page(0);
    Breaks();
    Page(0);
    Ended();
    Page(0);
    /* The toy ends as the program does, but for its end, which it has
       left taken (see Ended): it gives Ctrl-Break back, and leaves the
       critical error and Ctrl-C taken, which ProgramEnd of sys_dos.c
       puts back. An interrupt that is left taken there has the extender
       write a warning under the last line. */
    CtrlBreakGive();
    SAY("ProgramEnd 7");
    Line();
    CursorTo(0, Row << 8);
    ProgramEnd(7);
}
