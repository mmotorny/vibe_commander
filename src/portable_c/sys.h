/*
 * What the program asks of the system that it runs on: files,
 * directories and drives, memory, the time, other programs, the keyboard,
 * the screen, the mouse and the timer. Each thing that it asks is a
 * function here, named for what is asked, and nothing here says how the
 * system is asked: no register, no number of a service, no interrupt.
 * dos.c is these functions for DOS on a PC, and the only file that knows
 * those; the same functions for another system are another file beside
 * it.
 *
 * What is asked for is still shaped as DOS and the BIOS of a PC have it,
 * since the rest of the program has those shapes in hundreds of places
 * and the screens show them: a drive is a letter or its number, a path
 * has backslashes and at most 67 characters, a name 12 with its dot;
 * attributes, dates and times are DOS's bits; an error is DOS's number
 * for it; a key is a character with the scan code of the key above it; a
 * cell of the screen is a character with its colour above it; a tick is
 * the 18.2th part of a second; and a block of the memory is known by its
 * segment, and counted in paragraphs of 16 bytes. Another system has to
 * answer in these.
 *
 * One function is one thing asked. Where the program asks several
 * things in a row, the row is in the program, in the order of the
 * original: what DOS is asked and in which order is behaviour, and the
 * scenarios see it.
 *
 * Errors. A function that may fail answers with struct Answer, or with
 * a structure that begins as that one does: whether it failed, and the
 * handle, the count or whatever was asked for, or the error. The
 * functions of files, directories and drives come in two kinds, and
 * each group below says which of its functions are of which:
 *
 *   - Asked with the program's errors. A device that is not ready has the
 *     program show its box and ask for another try (FatErr of main.c).
 *     If the user gives up there, the function has failed with the error
 *     53h. A failure that the system reports itself comes back as its
 *     extended error, which DOS is asked for behind the call. And the
 *     box, which stays away once a call has been given up, may come
 *     again. This is Intr21 of the original, and most functions are of
 *     this kind.
 *   - Bare: the system's answer as it is, with nothing before the call
 *     and nothing behind it. The original calls DOS itself for these.
 *     Where a function of the first kind is also asked like this, it is
 *     a function of its own, of the same name with Bare behind it.
 *
 * What comes back. `value` of an answer is what DOS leaves in AX, also
 * where nothing failed and the call has nothing to say there: some
 * callers go on with that. Where the system answers with something else
 * as well, that is a member with a name, and if the call failed it is 0
 * unless its function says otherwise: DOS leaves alone what it has
 * nothing to say in, and 0 is what dos.c hands it there. The original
 * went on with whatever it had there itself, and a caller that goes by
 * that says what it was, where it reads the answer.
 *
 * A pointer here reaches anything that the program has, in whichever of
 * its segments: FAR, which is a word of the compiler's where the memory
 * is in segments and nothing where it is not. vc.h says which, where it
 * says so of the other such words, and includes this file; the types
 * u8, u16 and u32 are vc.h's too.
 */
#ifndef SYS_H
#define SYS_H

struct DTAs;                            /* what is known of a file that
                                           was found, */
struct CNTRY;                           /* how a country writes numbers,
                                           dates and times, */
struct EPBs;                            /* and what a program is started
                                           with: DOS's records, of vc.h */

/* What the system answered to something that may fail. */
struct Answer {
    u16 value;                          /* what was asked for, a handle or
                                           a number of bytes, or the error:
                                           AX as DOS left it */
    u16 failed;                         /* not 0 if that is the error */
};

/*
 * Files.
 *
 * A file is known by its handle once it is open. A name is a path that
 * ends in a zero byte. All of these are asked with the program's errors,
 * but for the three that say Bare.
 */

/* Make a file with these attributes, or make the one that there is
   empty, and open it to write. Answers with the handle. */
struct Answer FileCreate(const u8 FAR *name, u16 attributes);

/* Open a file: the low byte of `mode` is 0 to read, 1 to write and 2 for
   both, with what others may do meanwhile in its higher bits, as DOS has
   it. Answers with the handle. */
struct Answer FileOpen(const u8 FAR *name, u16 mode);
struct Answer FileOpenBare(const u8 FAR *name, u16 mode);

/* Close a file. */
struct Answer FileClose(u16 handle);
struct Answer FileCloseBare(u16 handle);

/* Read up to `count` bytes of a file to `to`, from where the file is at.
   Answers with how many were read: fewer at the end of the file. */
struct Answer FileRead(u16 handle, u8 FAR *to, u16 count);
struct Answer FileReadBare(u16 handle, u8 FAR *to, u16 count);

/* Write `count` bytes at `from` to a file, at where the file is at.
   Answers with how many were written: fewer on a disk that is full.
   None, 0, cuts the file off there. */
struct Answer FileWrite(u16 handle, const u8 FAR *from, u16 count);

/* Go to a place of a file: `offset` bytes from its start (`from` 0),
   from where it is at (1) or from its end (2); with 1 and 2 the offset
   may be below 0. Answers with the place then, counted from the start.
   If it failed, `place` is none: its low word is the error and its high
   word the low word of `offset`. The two words are what DOS left in AX
   and DX, and some callers hold them against the place that they asked
   for without looking at `failed`. */
struct FilePlace {
    u32 place;
    u16 failed;
};
struct FilePlace FileSeek(u16 handle, u16 from, u32 offset);

/* Delete a file. */
struct Answer FileDelete(const u8 FAR *name);

/* Give a file or a directory another name, or another directory of the
   same drive. */
struct Answer FileRename(const u8 FAR *from, const u8 FAR *to);

/* The attributes of a file or a directory, and others set for it. If
   asking failed, `attributes` is 0. */
struct FileAttr {
    u16 value;                          /* AX as DOS left it, or the error */
    u16 failed;
    u16 attributes;
};
struct FileAttr FileAttributes(const u8 FAR *name);
struct Answer FileSetAttributes(const u8 FAR *name, u16 attributes);

/* The date and the time that a file was last written at, as DOS packs
   them, and others set for it. If asking failed, both are 0. */
struct FileStamp {
    u16 value;                          /* AX as DOS left it, or the error */
    u16 failed;
    u16 time;
    u16 date;
};
struct FileStamp FileTime(u16 handle);
struct Answer FileSetTime(u16 handle, u16 time, u16 date);

/* What an open handle is: bit 7 of `kind` is set for a device, which is
   no file of a disk, and its lower bits then say which. If asking
   failed, `kind` is 0. */
struct Device {
    u16 value;                          /* AX as DOS left it, or the error */
    u16 failed;
    u16 kind;
};
struct Device FileDevice(u16 handle);

/* The full path of a name, with its drive and with nothing left in it
   to work out, to `to`, which has room for 128 bytes. */
struct Answer FileTrueName(const u8 FAR *name, u8 FAR *to);

/*
 * Files by a file control block, DOS's older way, which the program
 * goes for two things that the newer one cannot do: deleting all the
 * files of a mask at once, and a label of a drive.
 */

/* Make the name at `name` the name of a file control block, at `fcb`:
   `how` is as DOS takes it. Bare. Returns what DOS leaves in AX: 1 in
   the low byte if the name has * or ?, 0 if not, FFh for a drive that
   there is not. */
u16 FcbParse(const u8 FAR *name, u8 FAR *fcb, u16 how);

/* Delete what the file control block at `fcb` names. Asked with the
   program's errors. The low byte of `value` is 0 if something was
   deleted and FFh if nothing was.

   DOS leaves the carry flag as it finds it at this call, and the
   original takes a flag that is set for an error, whatever DOS did, and
   asks DOS which: that answer then comes back, with `failed`, in place
   of the call's. `flagged` is not 0 where the original has the flag set
   when it calls. */
struct Answer FcbDelete(const u8 FAR *fcb, u16 flagged);

/*
 * Directories. Asked with the program's errors, but for FindRecord and
 * DirCurrentBare.
 */

/* What is known of a file that was found is put at `record` from now
   on. */
void FindRecord(struct DTAs FAR *record);

/* Find the first file or directory that `pattern` names, a path whose
   last name may have * and ?, among those with no other attributes than
   `attributes`, or a label for 8; and the next one of the same search.
   What is found is at the record. The error at the end of the search is
   12h. */
struct Answer FindFirst(const u8 FAR *pattern, u16 attributes);
struct Answer FindNext(void);

/* Make a directory, remove one that is empty, make one the current one
   of its drive. */
struct Answer DirMake(const u8 FAR *name);
struct Answer DirRemove(const u8 FAR *name);
struct Answer DirChange(const u8 FAR *name);

/* The current directory of a drive, 1 for A: and 0 for the current one,
   in the low byte of `drive`: its path without the drive and the first
   backslash, to `to`, which has room for 64 bytes. */
struct Answer DirCurrent(u16 drive, u8 FAR *to);
struct Answer DirCurrentBare(u16 drive, u8 FAR *to);

/*
 * Drives. Where it is not said otherwise a drive is 0 for A:, and only
 * the low byte of it counts. All Bare, but DriveSpace.
 */

/* The current drive, and another made the current one. */
u16 DriveCurrent(void);
void DriveSelect(u16 drive);

/* How much room a drive has, 1 for A: and 0 for the current one. A
   drive that there is not is no failure: `value` is FFFFh for it, and
   the other members are 0. Asked with the program's errors.

   DOS leaves the carry flag as it finds it at this call too: see
   FcbDelete for `flagged`. */
struct Space {
    u16 value;                          /* the sectors of a cluster: AX as
                                           DOS left it, or the error */
    u16 failed;
    u16 free;                           /* the clusters that are free */
    u16 sector;                         /* the bytes of a sector */
    u16 total;                          /* the clusters of the drive */
};
struct Space DriveSpace(u16 drive, u16 flagged);

/* A drive that shares its unit with another, as A: and B: share one
   floppy drive, 1 for A:. DriveLetter answers in the low byte of `value`
   with the drive that the unit goes by now, or with 0 for a drive that
   shares none; DriveSetLetter has the unit go by `drive`, and answers
   with that. */
struct Answer DriveLetter(u16 drive);
struct Answer DriveSetLetter(u16 drive);

/* Is a drive, 1 for A:, of a network? Bit 12 of `kind` is set if it
   is. And can its disk be taken out? The low byte of `value` is 1 if it
   cannot. */
struct Device DriveRemote(u16 drive);
struct Answer DriveFixed(u16 drive);

/* Is a drive a CD-ROM? 1 or 0. */
u16 DriveCd(u16 drive);

/*
 * Memory, in blocks that are known by their segment and counted in
 * paragraphs. All Bare.
 */

/* What MemTake and MemResize answer with. */
struct Block {
    u16 value;                          /* the block, for MemTake; else AX
                                           as DOS left it, or the error */
    u16 failed;
    u16 largest;                        /* if it failed for want of memory,
                                           how many paragraphs there are:
                                           else `paragraphs` as it came */
};

/* Take a block of so many paragraphs; give one back; make one longer or
   shorter. */
struct Block MemTake(u16 paragraphs);
struct Answer MemGive(u16 block);
struct Block MemResize(u16 block, u16 paragraphs);

/* Where the system looks for a block that it is asked for: `how` is as
   DOS has it, 0 for the lowest block that fits, which is how DOS looks
   when it has started. And whether the upper memory, beyond the 640 KB
   that every PC has, is among what it looks through: 1 or 0. Neither
   answers, and a system that does not know the question goes on looking
   as it did. */
void MemSetSearch(u16 how);
void MemSetUpper(u16 on);

/*
 * The time, the country, the system itself. All Bare.
 */

/* The time of day. */
struct Time {
    u16 clock;                          /* the hour in the high byte and
                                           the minute in the low one */
    u16 seconds;                        /* the second in the high byte and
                                           the hundredth in the low one */
};
struct Time TimeOfDay(void);

/* How the country writes numbers, dates and times, to `to`. */
void CountryInfo(struct CNTRY FAR *to);

/* The capital of a letter as the country has it: only letters beyond
   ASCII change. The low byte is the letter, and the high byte comes
   back as it went. CountryInfo has to have been asked. */
u16 CaseMap(u16 letter);

/* Which version the system is. SystemVersion: the major number in the
   low byte of `version` and the minor one in the high byte, and who
   made it in the high byte of `other`. SystemTrueVersion, which a
   program cannot be told otherwise: the numbers likewise, and what DOS
   left in AX in `other`; a system that does not know the question
   answers with the version 0. */
struct SysVersion {
    u16 version;
    u16 other;
};
struct SysVersion SystemVersion(void);
struct SysVersion SystemTrueVersion(void);

/* Whether the system looks for Ctrl-C whatever it is asked, 1 or 0, and
   that set. */
u16 BreakCheck(void);
void SetBreakCheck(u16 on);

/* Tell the system that the program is idle: once, each time that it
   looks for a key and finds none. */
void Idle(void);

/*
 * Other programs, and the end of this one. ProgramRun is asked with the
 * program's errors.
 */

/* Run the program in the file `name` and come back when it has ended,
   with the command line and what else `block` names. */
struct Answer ProgramRun(const u8 FAR *name, const struct EPBs FAR *block);
struct Answer ProgramRunBare(const u8 FAR *name,
                             const struct EPBs FAR *block);

/* End the program, with a number for whoever started it. Does not
   return. */
void ProgramEnd(u16 code);

/* What DOS keeps of the program in front of it, its PSP, by its segment:
   256 bytes that have the command line which the program was started
   with, the segment of its environment, where its memory ends, and where
   the program that started it goes on when this one has ended. The
   program's block of the memory is known by the same segment. */
u16 ProgramPrefix(void);

/*
 * The console: what the system writes where the screen has its cursor,
 * with the lines moving up at the bottom. Bare.
 */

/* Write a character; and a text, as far as its dollar sign. */
void ConsoleChar(u16 character);
void ConsoleText(const u8 FAR *text);

/*
 * The keyboard.
 */

/* What KeyWaiting says. */
struct Keys {
    u16 key;                            /* the key that waits: the
                                           character, and the scan code in
                                           the high byte */
    u16 none;                           /* 1 if no key waits */
};

/* Does a key wait, and which? It stays where it is. And the key that
   waits, taken: KeyRead waits for one if there is none. */
struct Keys KeyWaiting(void);
u16 KeyRead(void);

/* Which of Shift, Ctrl and Alt are held, and which locks are on: the
   right Shift is 1, the left one 2, Ctrl 4, Alt 8, Scroll Lock 10h, Num
   Lock 20h, Caps Lock 40h and Ins 80h. */
u16 KeysHeld(void);

/* Take the Ins key for let go, so that it counts again while it is held
   down. */
void KeyForgetIns(void);

/*
 * The screen: 80 columns of text, in one of its pages. A place has the
 * row in its high byte and the column in its low one; a cell is a
 * character with its colour in the high byte.
 */

/* How the screen is set. */
struct Screen {
    u16 mode;                           /* 2 and 3 are text in colour, 7 is
                                           text in one colour; its highest
                                           bit says that the screen was not
                                           cleared for it */
    u16 columns;
    u16 page;                           /* the page that is shown */
};
struct Screen ScreenState(void);

/* Set a mode, which clears the screen. */
void ScreenSetMode(u16 mode);

/* The cursor of a page: where it is, and what shape it has, the first of
   its lines in the high byte and the last in the low one. */
struct CursorAt {
    u16 place;
    u16 shape;
};
struct CursorAt CursorWhere(u16 page);
void CursorTo(u16 page, u16 place);
void CursorShape(u16 shape);

/* Move what a window of the screen shows a number of lines up, or down
   if `down` is not 0, with empty lines of the colour `color` where it
   came from: `upper` is the upper left corner of the window and `lower`
   the lower right one. No lines, 0, empties the window. */
void ScreenScroll(u16 down, u16 lines, u16 color, u16 upper, u16 lower);

/* The cell at the cursor of a page. And `count` cells from the cursor
   on, written as this character in this colour; the cursor stays. */
u16 ScreenCell(u16 page);
void ScreenFill(u16 page, u16 character, u16 color, u16 count);

/* Write a character at the cursor as a terminal does: the cursor moves
   on, a line feed moves the lines up at the bottom, and 7 sounds the
   bell. The colour stays what the cell has. `page` and `color` are what
   the BIOS is handed for the page and for a colour that only a mode of
   graphics has: the original hands it what it happens to have. */
void ScreenType(u16 character, u16 page, u16 color);

/* Is the display an EGA or better, which can have more lines than 25? 1
   or 0. */
u16 ScreenEga(void);

/* The font of such a display: how many lines of dots a character has,
   and the last row of the screen with it. A display that does not say
   has 16 and 0. */
struct Font {
    u16 height;
    u16 last;
};
struct Font ScreenFont(void);

/* Give such a display the font of 8 lines, which makes 43 rows of an
   EGA's screen and 50 of a VGA's. */
void ScreenSmallFont(void);

/* How far into the memory of the display the page begins that is shown,
   in bytes. And where the cells of the screen are from now on, for
   ScreenPut and ScreenGet: those of the display that the mode `mode` is
   of, from `start` bytes into its memory. */
u16 ScreenPageStart(void);
void ScreenMemory(u16 mode, u16 start);

/* The cells of the screen themselves, 80 to a row: `count` of them from
   the cell `cell` on, written from `from` or read to `to`. The first
   cell is 0. ScreenMemory has to have said where they are. */
void ScreenPut(u16 cell, const u16 FAR *from, u16 count);
void ScreenGet(u16 cell, u16 FAR *to, u16 count);

/*
 * The mouse. It counts in dots, eight to a cell of the text screen.
 */

/* Is there a mouse? FFFFh if there is. MouseFind leaves it as it is set,
   and comes back with 21h from a driver that does not know the question;
   MouseReset sets it anew. */
u16 MouseFind(void);
u16 MouseReset(void);

/* Show the pointer, and hide it: hidden twice, it wants showing twice. */
void MouseShow(void);
void MouseHide(void);

/* Where the pointer is and which buttons are held. */
struct MouseAt {
    u16 buttons;                        /* the left one 1, the right one 2,
                                           the middle one 4 */
    u16 x, y;                           /* in dots */
};
struct MouseAt MouseWhere(void);

/* Put the pointer somewhere; and say which rows of dots it may go to. */
void MouseTo(u16 x, u16 y);
void MouseRows(u16 first, u16 last);

/* What the pointer looks like: of the cell that it is on, the bits of
   `keep` stay, and those of `turn` are turned round then. */
void MousePointer(u16 keep, u16 turn);

/*
 * The timer.
 */

/* The ticks since midnight, 18.2 to the second. */
u32 Ticks(void);

#endif
