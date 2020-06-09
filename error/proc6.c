#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#define TIME 6


int main(void)
{
    for (int i = 0; i < TIME; i++)
    {
        printf("pid %d - (%d)\n", getpid(), i);
        sleep(1);
    }
    return 0;
}