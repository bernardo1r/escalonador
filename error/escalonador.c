#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "queue.h"

#define QUANTUM 1
#define IO_DURATION 3*QUANTUM

int pDuration[3] = {QUANTUM, 2*QUANTUM, 4*QUANTUM};
int flag_procTerm = 0;

pthread_t idObserver;

struct procStruct {
    int pid;
    int ioRemTime;
    char *name;
};


void print_newQueue(void *val)
{
    printf("%s -- ", ((struct procStruct *)val)->name);
}

void print_readQueue(void *val)
{
    struct procStruct *proc = (struct procStruct *) val;
    printf("%s pid(%d) -- ", proc->name, proc->pid);
}

void *procObserver(void *arg)
{
    struct procStruct **curProc = (struct procStruct **)arg;
    int status;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    for (;;)
    {
        if (*curProc)
        {
            if ((*curProc)->pid == -1)
                return NULL;
            else if ((*curProc)->pid != 0)
            {
                waitpid((*curProc)->pid, &status, WUNTRACED);
                
                if (!WIFSTOPPED(status))
                {
                    if(!flag_procTerm)
                        kill(getpid(), SIGUSR1);
                }
            }
        }
    }
}

int initProc(char *procName)
{
    int parent_pid = getpid(); /* if exec fails */
    int pid = fork();
    if (!pid)
    {
        execlp(procName, procName, NULL);
        fprintf(stderr, "exec failed when tried to execute '%s'\n", procName);
        kill(parent_pid, SIGKILL);
        exit(EXIT_FAILURE);
    }
    return pid;
}

void handle_ignore(int sig)
{
    flag_procTerm = 1;
}

int main(int argc, char **argv)
{
    if (!(argc > 1))
        return 0;
    struct procStruct procs[argc-1];
    struct procStruct *curProc = NULL;
    
    /* create queues and fill process structs */
    Queue *newQueue;
    Queue *readyQueue[3];
    for (int i = 0; i < 3; i++)
        readyQueue[i] = queue_create();

    newQueue = queue_create();
    for (int i = 1; i < argc; i++)
    {
        procs[i-1].pid = 0;
        procs[i-1].ioRemTime = 0;
        procs[i-1].name = argv[i];
        queue_push(newQueue, &procs[i-1]);
    }
    fputs("Fila de novos: ", stdout);
    queue_print(newQueue, print_newQueue);
    
    signal(SIGUSR1, handle_ignore);
    pthread_create(&idObserver, NULL, procObserver, &curProc);
    int remTime;
    for (;;)
    {
        if (!queue_isEmpty(newQueue))
        {
            curProc = (struct procStruct *) queue_pop(newQueue);
            curProc->pid = initProc(curProc->name);
            printf("\nIniciando processo %s - pid %d\n", curProc->name, curProc->pid);
        } 
        else if (!queue_isEmpty(readyQueue[0]))
        {
            curProc = (struct procStruct *) queue_pop(readyQueue[0]);
            printf("\nRetomando processo %s - pid %d\n", curProc->name, curProc->pid);
            kill(curProc->pid, SIGCONT);
        }
        else
            break;

        flag_procTerm = 0;
        //fputs("Fila de novos: ", stdout);
        //queue_print(newQueue, print_newQueue);
        //fputs("Fila de prontos: ", stdout);
        //queue_print(readyQueue[0], print_readQueue);
        //puts("");
        remTime = sleep(pDuration[0]);
        if (flag_procTerm)
        {
            printf("Processo terminou %s - pid %d\n", curProc->name, curProc->pid);
            curProc = NULL;
        }
        else
        {
            //printf("Processo sendo parado %s - pid %d\n", curProc->name, curProc->pid);
            kill(curProc->pid, SIGSTOP);
            queue_push(readyQueue[0], curProc);
        }
    }

    for (int i = 0; i < 3; i++)
        queue_free(readyQueue[i]);
    queue_free(newQueue);
    return 0;
}