typedef struct queue Queue;

Queue *queue_create(void);

int queue_isEmpty(Queue *q);

void queue_push(Queue *q, void *val);

void *queue_pop(Queue *q);

void queue_print(Queue *q, void (*print)(void *));

void queue_free(Queue *q);