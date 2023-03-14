#include "common.h"

void doubleQueueSize(Queue* queue) {
    queue->size *= 2;
    queue->processes = (Process *)realloc(queue->processes, queue->size * sizeof(Process));
    if (!queue->processes) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }
}

void enqueue(Queue *queue, Process *process) {
    // make sure the proc
    if (queue->processes == NULL) {
        queue->processes = process;
        queue->numProcesses = 1;
        queue->front = 0;
        queue->back = 1;
        return;
    }
    
    queue->processes[queue->back] = *process;
    queue->back++;
    if (queue->back == queue->size) {
        doubleQueueSize(queue);
    }
    free(process);
    queue->numProcesses++;
}

Process* dequeue(Queue *queue) {
    if (queue->numProcesses == 0) {
        return NULL;
    }
    // copy the process
    Process *process = (Process *)calloc(1, sizeof(Process));
    *process = queue->processes[queue->front];
    queue->numProcesses--;
    queue->front++;
    if (queue->front == queue->size) {
        exit(EXIT_FAILURE);
    }
    return process;
}

void freeQueue(Queue *queue) {
    if(!queue) {
        return;
    }
    if(!queue->processes) {
        free(queue);
        return;
    }
    int i = 0;
    while(i < queue->numProcesses) {
        free(queue->processes[i].bursts);
        i++;
    }
    free(queue->processes);
    free(queue);
}

Queue* createQueue() {
    Queue *queue = (Queue *)calloc(1, sizeof(Queue));
    if(!queue) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }
    queue->processes = (Process *)calloc(QUEUE_SIZE, sizeof(Process));
    if(!queue->processes) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }
    queue->numProcesses = 0;
    queue->front = 0;
    queue->size = QUEUE_SIZE;
    queue->back = 0;
    return queue;
}

bool isQueueEmpty(Queue *queue) {
    return queue->numProcesses == 0;
}

Process* peek(Queue *queue) {
    if (queue->numProcesses == 0) {
        return NULL;
    }
    if (queue->front == queue->size) {
        exit(EXIT_FAILURE);
    }
    return &queue->processes[queue->front];
}
