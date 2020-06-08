#include <stdio.h>
#include "src/queue.h"

void list_arr_print(void *arg)
{
    printf("%d ", *(int *)arg);
}


int main(void)
{
    Queue *q = queue_create();
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; i++)
    {
        queue_push(q, &arr[i]);
        queue_print(q, list_arr_print);
    }
    for (int i = 0; i < 10; i++)
    {
        queue_pop(q);
        queue_print(q, list_arr_print);
    }
    for (int i = 0; i < 10; i++)
    {
        queue_push(q, &arr[i]);
        queue_print(q, list_arr_print);
    }
    queue_free(q);
    return 0;
}