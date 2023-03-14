#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 200
#define QUEUE_SIZE 50000

typedef struct process {
    int *bursts;
    int arrivalTime;
    int maxIndex;
    int currentBurst;
} Process;

typedef struct queue {
    Process *processes;
    int front;
    int back;
    int size;
    double numProcesses;
} Queue;



#include "queue.h"

#endif
