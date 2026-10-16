/*
 * A stand-in for the shell of the app for iOS, on a host: the piece I2
 * of step 8 (TODO.md, "Step 8, the app for iOS"). The program is
 * src/portable_c/sys_ios.c with the common code and the emulation of
 * DOS behind it, as the app has it; what the app's shell in SwiftUI
 * does around it, by src/portable_c/ios_shell.h, this file does in C, so
 * that the program can be taken through the scenarios on Linux, by the
 * test channel (VC_TEST), as the tree "ios-host" of the harness
 * (harness/posixrun.py). The rule on test builds allows it: a part of
 * the program itself, the shell, is replaced.
 *
 * Its drives are those of the web page's tests: C: the directory that
 * VC_DRIVE_C names, D: that of VC_DRIVE_D, and the program's own files
 * on the drive of VC_PROGRAM_DRIVE (sys_ios.c reads that). It takes
 * every screen that the program hands, on its own thread as the app's
 * shell does, and counts them; a file to show is shown nowhere; and the
 * system lets the app have a fixed 1 GB, of which the program takes 256
 * MB, the web page's room.
 *
 * Without the test channel it takes the keys that VC_IOS_KEYS has, as
 * the bytes that the shell would hand on, \e for Esc, once the first
 * screen is there and again at each start; starts the program anew when
 * it has ended, as many times as VC_IOS_STARTS says, 1 where it says
 * nothing; and writes the last row of each last screen on standard
 * output, with the code that the program ended with: a trial by hand of
 * the way that the tests do not take, the condition variable, the end
 * and a start anew.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ios_shell.h"

#define AVAILABLE ((uint64_t)1024 * 1024 * 1024)

static pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t Changed = PTHREAD_COND_INITIALIZER;
static int Drawn = 0;                   /* screens told of, not taken */
static int Ended = 0;                   /* ends told of, not taken */
static int EndedWith = 0;

void ios_shell_draw(void)
{
    pthread_mutex_lock(&Lock);
    Drawn = 1;
    pthread_cond_signal(&Changed);
    pthread_mutex_unlock(&Lock);
}

void ios_shell_open_file(const char *path)
{
    (void)path;
}

void ios_shell_ended(int code)
{
    pthread_mutex_lock(&Lock);
    Ended = 1;
    EndedWith = code;
    pthread_cond_signal(&Changed);
    pthread_mutex_unlock(&Lock);
}

uint64_t ios_shell_memory(void)
{
    return AVAILABLE;
}

/* The keys of VC_IOS_KEYS, \e made Esc, handed on. */
static void Type(const char *keys)
{
    char bytes[1024];
    int count = 0;

    for (; *keys != 0 && count < (int)sizeof bytes; keys++) {
        if (keys[0] == '\\' && keys[1] == 'e') {
            bytes[count++] = '\033';
            keys++;
        } else {
            bytes[count++] = *keys;
        }
    }
    ios_input(bytes, count);
}

int main(void)
{
    static uint16_t cells[IOS_CELLS];
    const char *keys = getenv("VC_IOS_KEYS");
    const char *starts_said = getenv("VC_IOS_STARTS");
    int starts = starts_said != 0 ? atoi(starts_said) : 1;
    int row, column, i, typed = 0;
    uint16_t shape;
    uint32_t handed = 0;

    if (ios_start(getenv("VC_DRIVE_C"), getenv("VC_DRIVE_D")) != 0) {
        fprintf(stderr, "vc: the program could not be started\n");
        return 1;
    }
    for (;;) {
        pthread_mutex_lock(&Lock);
        while (!Drawn && !Ended)
            pthread_cond_wait(&Changed, &Lock);
        if (Drawn) {
            Drawn = 0;
            pthread_mutex_unlock(&Lock);
            handed = ios_screen(cells, &row, &column, &shape);
            if (!typed && keys != 0) {
                typed = 1;
                Type(keys);
            }
            continue;
        }
        Ended = 0;
        pthread_mutex_unlock(&Lock);
        handed = ios_screen(cells, &row, &column, &shape);
        if (keys == 0 && starts_said == 0)
            return EndedWith;           /* a test's, or a run by hand */
        for (i = 0; i < IOS_COLUMNS; i++)
            putchar((cells[(IOS_ROWS - 1) * IOS_COLUMNS + i] & 0x7F) >= ' '
                    ? cells[(IOS_ROWS - 1) * IOS_COLUMNS + i] & 0x7F : '.');
        printf("\nended with %d after %u screens\n", EndedWith,
               (unsigned)handed);
        if (--starts <= 0)
            return 0;
        typed = 0;
        if (ios_start(getenv("VC_DRIVE_C"), getenv("VC_DRIVE_D")) != 0) {
            fprintf(stderr, "vc: the program could not be started anew\n");
            return 1;
        }
    }
}
