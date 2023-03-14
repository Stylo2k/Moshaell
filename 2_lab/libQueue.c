#include "libQueue.h"


Queue newQueue(int size)
{
  Queue queue;
  queue.array = calloc(size,  sizeof(int));
  assert(queue.array != NULL);
  queue.back = 0;
  queue.front = 0;
  queue.index = 0;
  queue.size = size;
  return queue;
}

int isEmptyQueue(Queue queue)
{
  return (queue.back == queue.front);
}

void queueEmptyError()
{
  printf("**Queue is empty**\nAborting...");
  abort();
}

void enqueue(int item, Queue *qp)
{
  qp->array[qp->back] = item;
  qp->back = (qp->back + 1) % qp->size;
  if (qp->back == qp->front)
  qp->index++;
}

void doubleQueueSize(Queue *qp)
{
  int i;
  int oldSize = qp->size;
  qp->size = 2 * oldSize;
  qp->array = realloc(qp->array, qp->size * sizeof(int));
  assert(qp->array != NULL);
  for (i = 0; i < qp->back; i++)
  {
    qp->array[oldSize + i] = qp->array[i];
  }
  qp->back = qp->back + oldSize;
}


int dequeue(Queue *qp)
{
  int item;
  item = qp->array[qp->front];
  qp->front = (qp->front + 1) % qp->size;
  qp->index--;
  return item;
}

void freeQueue(Queue queue)
{
  free(queue.array);
}

int isInQueue(Queue queue, int item) {
    for (int i = 0; i < queue.size; i++) {
        if (queue.array[i] == item) {
            return 1;
        }
    }
    return 0;
}

void printQueue(Queue queue)
{
  printf("[ ");
  for (int i = 0; i < queue.size; i++)
  {
    if (i > queue.size)
    {
      i = i % queue.size;
    }
    printf("|%d| ", queue.array[i]);
  }
  printf("]\n");
}