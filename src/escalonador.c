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
#define MAX_P 3

int pDuration[MAX_P] = {QUANTUM, 2*QUANTUM, 4*QUANTUM};

int flag_procTerm = 0;
int flag_IO = 0;

int elapsTime = 0;

pthread_t idObserver;

struct procStruct {
    int pid;
    int ioRemTime;
    char *name;
    int prio;
};


void print_newQueue(void *val)
{
    printf("%s <- ", ((struct procStruct *)val)->name);
}

void print_readQueue(void *val)
{
    struct procStruct *proc = (struct procStruct *) val;
    printf("%s pid(%d) <- ", proc->name, proc->pid);
}


void print_ioQueue(void *val)
{
    struct procStruct *proc = (struct procStruct *) val;
    printf("%s pid(%d) - prioridade (%d)- tempo io res(%d) <- ", proc->name, proc->pid, proc->prio, proc->ioRemTime);
}

void *map_subtract_elapsTime(void *val)
{
    int *number = &(((struct procStruct *)val)->ioRemTime);

    *number -= elapsTime;
    if (*number < 0)
        *number = 0;

    return val;
}

void *procObserver(void *arg)
{
    struct procStruct **curProc = (struct procStruct **)arg;
    int status;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    for (;;)
    {
        if (*curProc)
        {
            if ((*curProc)->pid == -1)
                return NULL;

            if ((*curProc)->pid != 0)
            {
                waitpid((*curProc)->pid, &status, WUNTRACED);
                
                if (!WIFSTOPPED(status))
                    kill(getpid(), SIGUSR1);
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

void handle_procTerm(int sig)
{
    flag_procTerm = 1;
}

void handle_procIO(int sig)
{
    flag_IO = 1;
}

int searchNewQueues(Queue **readyQueue, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (!queue_isEmpty(readyQueue[i]))
            return i;
    }
    return -1;
}

int main(int argc, char **argv)
{
    if (!(argc > 1))
        return 0;
    struct procStruct procs[argc-1];
    struct procStruct *curProc = NULL, *tmpProc = NULL;
    
    /* create queues and fill process structs */
    Queue *newQueue;
    Queue *readyQueue[MAX_P];
    Queue *ioQueue = queue_create();
    for (int i = 0; i < MAX_P; i++)
        readyQueue[i] = queue_create();

    newQueue = queue_create();
    for (int i = 1; i < argc; i++)
    {
        procs[i-1].pid = 0;
        procs[i-1].ioRemTime = 0;
        procs[i-1].name = argv[i];
        procs[i-1].prio = -1;
        queue_push(newQueue, &procs[i-1]);
    }
    fputs("Fila de novos: ", stdout);
    queue_print(newQueue, print_newQueue);
    
    signal(SIGUSR1, handle_procTerm);
    signal(SIGUSR2, handle_procIO);
    pthread_create(&idObserver, NULL, procObserver, &curProc);
    int remTime, err, curPrio;
    for (;;)
    {
        if (!queue_isEmpty(newQueue))
        {
            curProc = (struct procStruct *) queue_pop(newQueue);
            curProc->pid = initProc(curProc->name);
            curProc->prio = 0;
            printf("\nIniciando processo %s - pid %d - prioridade %d\n", curProc->name, curProc->pid, curProc->prio);

            curPrio = 0;
        } 
        else if ((curPrio = searchNewQueues(readyQueue, MAX_P)) != -1)
        {
            curProc = (struct procStruct *) queue_pop(readyQueue[curPrio]);
            printf("\nRetomando processo %s - pid %d - prioridade %d\n", curProc->name, curProc->pid, curProc->prio);
            kill(curProc->pid, SIGCONT);
        }
        else if (!queue_isEmpty(ioQueue)) /* if all processes are doing IO */
        {
            puts("\nTodos os processos estao fazendo I/O");
            puts("Dormindo por 1 QUANTUM");
            sleep(QUANTUM);
            elapsTime = QUANTUM;

            queue_map(ioQueue, map_subtract_elapsTime);

            tmpProc = (struct procStruct *) queue_seeFirst(ioQueue);
            curPrio = tmpProc->prio;
            while (tmpProc->ioRemTime == 0)
            {
                printf("\nProcesso terminou I/O %s - pid %d - prioridade %d\n", tmpProc->name, tmpProc->pid, tmpProc->prio);
                queue_pop(ioQueue);
                queue_push(readyQueue[tmpProc->prio], tmpProc);
                tmpProc = (struct procStruct *) queue_seeFirst(ioQueue);
                if (tmpProc == NULL)
                    break;

            }
            puts("");
            fputs("Fila de novos: ", stdout);
            queue_print(newQueue, print_newQueue);
            for (int i = 0; i < MAX_P; i++)
            {
                printf("Fila de prontos %d: ", i);
                queue_print(readyQueue[i], print_readQueue);
            }
            fputs("Fila de io: ", stdout);
            queue_print(ioQueue, print_ioQueue);
            puts("");
            puts("----------------------------------");
            continue;
        }
        else /* all programs have finished */
        {
            curProc->pid = -1; /* to make thread exit */
            break;
        }

        puts("");
        fputs("Fila de novos: ", stdout);
        queue_print(newQueue, print_newQueue);
        for (int i = 0; i < MAX_P; i++)
        {
            printf("Fila de prontos %d: ", i);
            queue_print(readyQueue[i], print_readQueue);
        }
        fputs("Fila de io: ", stdout);
        queue_print(ioQueue, print_ioQueue);
        puts("");
        remTime = sleep(pDuration[curPrio]);
        
        err = kill(curProc->pid, SIGSTOP);
        fputs("", stdout); /* to avoid pending signals from being received at the next sleep */
        elapsTime = pDuration[curPrio] - remTime;

        if (!queue_isEmpty(ioQueue)) /* check for current io processes */
        {   
            queue_map(ioQueue, map_subtract_elapsTime);

            tmpProc = (struct procStruct *) queue_seeFirst(ioQueue);
            while (tmpProc->ioRemTime == 0)
            {
                printf("\nProcesso terminou I/O %s - pid %d - prioridade %d\n", tmpProc->name, tmpProc->pid, tmpProc->prio);
                queue_pop(ioQueue);
                queue_push(readyQueue[tmpProc->prio], tmpProc);
                tmpProc = (struct procStruct *) queue_seeFirst(ioQueue);
                if (tmpProc == NULL)
                    break;
            }
        }

        if (err) /* process ended */
        {
            printf("Processo acabou %s - pid %d\n", curProc->name, curProc->pid);
        }
        else
        {
            if (flag_IO)
            {
                curProc->ioRemTime = IO_DURATION;
                if (curProc->prio != 0)
                    curProc->prio -= 1;

                queue_push(ioQueue, curProc);
                printf("Processo comecou a fazer I/O %s - pid %d - nova prioridade %d\n", curProc->name, curProc->pid, curProc->prio);
                flag_IO = 0;
            }
            else 
            {
                if (curProc->prio != MAX_P-1)
                    curProc->prio += 1;
                queue_push(readyQueue[curProc->prio], curProc);
                printf("Processo sendo parado %s - pid %d - nova prioridade %d\n", curProc->name, curProc->pid, curProc->prio);
            }
        }
        puts("----------------------------------");
    }

    pthread_join(idObserver, NULL);

    for (int i = 0; i < 3; i++)
        queue_free(readyQueue[i]);
    queue_free(newQueue);
    queue_free(ioQueue);
    return 0;
}