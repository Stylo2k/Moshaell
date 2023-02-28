#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct process {
    int *bursts;
    int arrivalTime;
    int maxIndex;
    int index;
} Process;

typedef struct queue {
    struct process *processes;
    int numProcesses;
    int currentIndex;
} Queue;

#endif
