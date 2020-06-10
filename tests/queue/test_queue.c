#include <stdio.h>
#include "src/queue.h"

void list_arr_print(void *arg)
{
    printf("%d ", *(int *)arg);
}

void *multiply_2(void *val)
{
    int *number = val;
    *number *= 2;
    return number;
}

void *subtract_1(void *val)
{
    int *number = val;
    *number -= 1;
    return number;
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

    puts("\nMultiply all queue members by 2:");
    queue_map(q, multiply_2);
    queue_print(q, list_arr_print);

    puts("\nSubtract all queue members by 1:");
    queue_map(q, subtract_1);
    queue_print(q, list_arr_print);

    puts("\nSee first element in the queue");
    printf("%d\n", *(int *)queue_seeFirst(q));
    queue_print(q, list_arr_print);

    queue_free(q);

    return 0;
}