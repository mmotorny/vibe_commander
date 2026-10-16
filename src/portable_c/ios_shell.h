/*
 * What the program for iOS and its shell ask of each other: step 8 of
 * the portable C's road (CLAUDE.md, "The app for iOS"), the piece I2
 * (TODO.md, "Step 8, the app for iOS"). The program is sys_ios.c with
 * the common code and the emulation of DOS of *_posix.c behind it, and
 * runs on a thread of its own; the shell is the app's, in SwiftUI on
 * the device, and on a host a stand-in in C (harness/ios_stand_in.c),
 * which the tests run. Plain C on both sides, with no type of the
 * program's, so that Swift reads it as it is.
 *
 * Each function is said with the thread that it is called on. The
 * shell's thread is any one thread of the shell's, the main thread of
 * the app on the device.
 */
#ifndef IOS_SHELL_H
#define IOS_SHELL_H

#include <stdint.h>

/* The screen: 80 by 25 cells. */
#define IOS_COLUMNS 80
#define IOS_ROWS 25
#define IOS_CELLS (IOS_COLUMNS * IOS_ROWS)

/*
 * What the shell calls.
 */

/* Start the program on a thread of its own, with its two drives: C:,
   labelled VIBE, the directory `drive_c`, where the program's own files
   are, in its directory vc; and D:, labelled DEMO, the directory
   `drive_d`, or none where that is NULL. The program starts in the
   process's current directory where a drive has it, else at the root of
   C:. Called once, and again after ios_shell_ended to start the program
   anew. Answers 0, or -1 where the program could not be started: a
   drive that is no directory, or no thread. On the shell's thread. */
int ios_start(const char *drive_c, const char *drive_d);

/* Bytes of the keyboard and of the mouse, as a terminal with the kitty
   protocol of the keyboard sends them (keyboard_posix.c reads them), the
   mouse in the reports of SGR's mode 1006 with motions, columns and rows
   from 1: queued for the program, which takes them when it next looks.
   What does not fit in the queue is dropped. On the shell's thread. */
void ios_input(const char *bytes, int count);

/* The screen as the program last handed it, after ios_shell_draw: its
   2,000 cells, a character of the code page 437 in the low byte and its
   colour in the high one, row after row, copied to `cells`; the cursor's
   row and column, from 0, at `row` and `column`, and its shape as the
   BIOS has it, its first and last scan line in the high and the low
   byte, hidden where bit 13 of it is set, at `shape`. Answers how many
   screens the program has handed so far, 0 where none, so that a screen
   that was taken once need not be drawn again. On the shell's thread. */
uint32_t ios_screen(uint16_t cells[IOS_CELLS], int *row, int *column,
                    uint16_t *shape);

/*
 * What the shell has, which the program calls.
 */

/* A screen has been handed, to be taken by ios_screen on the shell's
   own thread. On the program's thread, which goes on at once. */
void ios_shell_draw(void);

/* Show the file of the host `path`, a regular file, by QuickLook: Enter
   on a file that is no program. On the program's thread, which goes on
   at once: `path` is the program's only for the call. */
void ios_shell_open_file(const char *path);

/* The program has ended, with `code`: F10. The shell keeps its last
   screen, and a tap starts the program anew by ios_start. On the
   program's thread, which then ends. */
void ios_shell_ended(int code);

/* The bytes of memory that the system lets the app have now, of which
   the program takes half at the most, and no more than 256 MB, for the
   room of its editor and viewer (CLAUDE.md, "The app for iOS"): on the
   device os_proc_available_memory(). On the program's thread, once at
   each start. */
uint64_t ios_shell_memory(void);

#endif
