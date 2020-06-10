#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

struct node{
    void *val;
    struct node *next;
};

struct queue{
    struct node *last;
    struct node *first;
};

Queue *queue_create(void)
{
    Queue *q = malloc(sizeof(Queue));
    q->first = NULL;
    q->last = NULL;
    return q;
}

int queue_isEmpty(Queue *q)
{
    return (q->last == NULL) ? 1:0;
}

void queue_push(Queue *q, void *val)
{
    struct node *n = malloc(sizeof(struct node));
    n->val = val;
    n->next = NULL;
    if (queue_isEmpty(q))
        q->first = n;
    else
        q->last->next = n;

    q->last = n;
}

void *queue_pop(Queue *q)
{
    if (queue_isEmpty(q))
        return NULL;

    void *ret = q->first->val;
    struct node *rm = q->first;
    q->first = q->first->next;
    if(q->first == NULL)
        q->last = NULL;
    
    free(rm);
    return ret;
}

void queue_print(Queue *q, void (*print)(void *))
{
    struct node *it = q->first;
    while (it != NULL)
    {
        print(it->val);
        it = it->next;
    }
    puts("");
}

void queue_map(Queue *q, void *(*map)(void *))
{
    struct node *it = q->first;
    while (it != NULL)
    {
        it->val = map(it->val);
        it = it->next;
    }
}

void *queue_seeFirst(Queue *q)
{
    if (queue_isEmpty(q))
        return NULL;
        
    return q->first->val;
}

void queue_free(Queue *q)
{
    while (!queue_isEmpty(q))
        queue_pop(q);

    free(q);
}