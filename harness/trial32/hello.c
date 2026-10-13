/* The one toy with Open Watcom's C library and its start-up: to see
   whether an extender that stops the others stops this too. CauseWay
   does. */
#include <stdio.h>

int main(void)
{
    printf("hello from the C library\n");
    return 0;
}
