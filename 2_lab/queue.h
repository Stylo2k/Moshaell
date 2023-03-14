#ifndef QUEUE_H
#define QUEUE_H

void doubleQueueSize(Queue* queue);
void enqueue(Queue *queue, Process *process);
Process* dequeue(Queue *queue);
void freeQueue(Queue *queue);
Queue* createQueue();
bool isQueueEmpty(Queue *queue);
Process* peek(Queue *queue);

#endif