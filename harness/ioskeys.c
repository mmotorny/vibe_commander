/*
 * The table of src/portable_c/keys_ios.c run on Linux, for the test
 * portable_c-ios-keys, harness/ioskeys.py: a key event of UIKit a line
 * on the standard input, the bytes that the table makes of it a line on
 * the standard output, with one struct IosKeys for all of them, as the
 * shell keeps one.
 *
 * A line is the event (1 a press, 2 a repeat, 3 a release), the usage
 * and the modifiers in hexadecimal, and characters and
 * charactersIgnoringModifiers as their bytes of UTF-8 in hexadecimal,
 * "-" for an empty string; or "lost", for IosKeysLost. What is written is
 * the bytes in hexadecimal, or "-" for none.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys_ios.h"

/* The bytes of `hex` at `to`, which has room for `room` and a zero byte;
   false where it is no hexadecimal or too long. */
static bool Bytes(const char *hex, char *to, size_t room)
{
    size_t length = strlen(hex), i;
    char pair[3] = { 0, 0, 0 };

    if (strcmp(hex, "-") == 0) {
        to[0] = '\0';
        return true;
    }
    if (length % 2 != 0 || length / 2 >= room)
        return false;
    for (i = 0; i < length / 2; i++) {
        pair[0] = hex[2 * i];
        pair[1] = hex[2 * i + 1];
        to[i] = (char)strtol(pair, NULL, 16);
    }
    to[length / 2] = '\0';
    return true;
}

int main(void)
{
    struct IosKeys keys = { false, false };
    char line[256], characters[64], ignoring[64], bytes[IOS_KEY_ROOM];
    char hexChars[130], hexIgnoring[130];
    struct IosKey key;
    unsigned usage, modifiers;
    int event;
    i32 count, i;

    while (fgets(line, sizeof line, stdin) != NULL) {
        if (strncmp(line, "lost", 4) == 0)
            count = IosKeysLost(&keys, bytes, IOS_KEY_ROOM);
        else {
            if (sscanf(line, "%d %x %x %129s %129s", &event, &usage,
                       &modifiers, hexChars, hexIgnoring) != 5
                || !Bytes(hexChars, characters, sizeof characters)
                || !Bytes(hexIgnoring, ignoring, sizeof ignoring)) {
                fprintf(stderr, "a line that is no key event: %s", line);
                return 2;
            }
            key.usage = (i32)usage;
            key.modifiers = modifiers;
            key.characters = characters;
            key.ignoring = ignoring;
            key.event = event;
            count = IosKeyBytes(&keys, &key, bytes, IOS_KEY_ROOM);
        }
        if (count == 0)
            printf("-");
        for (i = 0; i < count; i++)
            printf("%02X", (unsigned)(u8)bytes[i]);
        printf("\n");
    }
    return 0;
}
