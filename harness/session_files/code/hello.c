/* The smallest program worth reading. */
#include <stdio.h>

int main(void)
{
    const char *names[] = { "left panel", "right panel" };
    int i;

    for (i = 0; i < 2; i++)
        printf("Hello from the %s!\n", names[i]);
    return 0;
}
