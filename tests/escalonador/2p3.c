#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main(void)
{
    int i = 0;
    for (; i < 10; i++)
    {
        printf("pid %d - (%d)\n", getpid(), i);
        sleep(1);
    }

    kill(getppid(), SIGUSR2);
    
    for (; i < 11; i++)
    {
        sleep(1);
        printf("pid %d - (%d)\n", getpid(), i);
    }
    
    return 0;
}
