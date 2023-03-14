#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Queue
{
  int *array;
  int back;
  int front;
  int size;
  int index;
} Queue;

int isInQueue(Queue queue, int item);
Queue newQueue(int s);
int isEmptyQueue(Queue q);
void queueEmptyError();
void doubleQueueSize(Queue *qp);
void enqueue(int item, Queue *qp);
Queue enqueueNoPointer(int item, Queue queue);
int dequeueNoPointer(Queue queue);
int dequeue(Queue *qp);
void printQueue(Queue queue);
void freeQueue(Queue q);


#endif