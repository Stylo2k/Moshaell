#include "common.h"


Process* readProccess() {
    int* process = (int*)calloc(100, sizeof(int));
    Process *p = (Process *)calloc(1, sizeof(Process));
    int i = 0;
    int temp;
    while (scanf("%d", &temp) == 1) {
        if (i == 0) {
            p->arrivalTime = temp;
            i++;
            continue;
        } else {
            process[i-1] = temp;
        }
        if (temp == -1) {
            break;
        }
        i++;
    }
    if (i == 0) {
        free(process);
        free(p);
        return NULL;
    }
    p->bursts = process;
    p->maxIndex = i;
    p->index = 0;
    return p;
}

void printProcess(int* process) {
    int i = 0;
    while (process) {
        printf("%d ", process[i]);
        if (process[i] == -1) {
            break;
        }
        i++;
    }
    printf("\n");
}

void enqueue(Queue *queue, Process *process) {
    if (queue->processes == NULL) {
        queue->processes = process;
        queue->numProcesses = 1;
        queue->currentIndex = 0;
    } else {
        queue->processes[queue->numProcesses] = *process;
        free(process);
        queue->numProcesses++;
    }
}

Process* dequeue(Queue *queue) {
    if (queue->numProcesses == 0) {
        return NULL;
    }
    // copy the process
    Process *process = (Process *)calloc(1, sizeof(Process));
    *process = queue->processes[queue->currentIndex];
    queue->processes[queue->currentIndex].bursts = NULL;
    // increment the index
    queue->currentIndex++;
    queue->numProcesses--;
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
    while(queue->processes[i].bursts) {
        free(queue->processes[i].bursts);
        i++;
    }
    free(queue->processes);
    free(queue);
}

Queue* createQueue() {
    Queue *queue = (Queue *)calloc(1, sizeof(Queue));
    queue->processes = (Process *)calloc(100, sizeof(Process));
    queue->numProcesses = 0;
    queue->currentIndex = 0;
    return queue;
}

bool isQueueEmpty(Queue *queue) {
    return queue->numProcesses == 0;
}

Process* peek(Queue *queue) {
    if (queue->numProcesses == 0) {
        return NULL;
    }
    return &queue->processes[queue->currentIndex];
}

int main() {
    Queue *arrivalQueue = createQueue(); 

    Process *process = readProccess();

    while (process) {
        enqueue(arrivalQueue, process);
        process = readProccess();
    }

    int totalProcesses = arrivalQueue->numProcesses;
    
    Queue *cpuQueue = createQueue();
    Queue *ioQueue = createQueue();

    int time = 0;
    double turnaroundTime = 0;
    
    while (!isQueueEmpty(arrivalQueue) || !isQueueEmpty(cpuQueue) || !isQueueEmpty(ioQueue)) {
        // check if any processes have arrived
        while (!isQueueEmpty(arrivalQueue) && peek(arrivalQueue)->arrivalTime == time) {
            enqueue(cpuQueue, dequeue(arrivalQueue));
        }
        // check if any processes are done with their cpu burst
        if (!isQueueEmpty(cpuQueue)) {
            Process *p = peek(cpuQueue);
            if (p->index >= p->maxIndex) {
                turnaroundTime += time - p->arrivalTime;
                dequeue(cpuQueue);
            } else
            if (p->bursts[p->index] == 0) {
                // check if the process is done
                if (p->bursts[p->index + 1] == -1) {
                    turnaroundTime += time - p->arrivalTime;
                    dequeue(cpuQueue);
                } else {
                    // move the process to the io queue
                    // if the index is out of bounds, the process is done
                    p->index++;
                    enqueue(ioQueue, dequeue(cpuQueue));
                }
            } else {
                // decrement the burst time
                p->bursts[p->index]--;
            }
        }
        // check if any processes are done with their io burst
        if (!isQueueEmpty(ioQueue)) {
            Process *p = peek(ioQueue);
            if (p->index >= p->maxIndex) {
                turnaroundTime += time - p->arrivalTime;
                dequeue(ioQueue);
            } else
            if (p->bursts[p->index] == 0) {
                if (p->bursts[p->index + 1] == -1) {
                    turnaroundTime += time - p->arrivalTime;
                    dequeue(ioQueue);
                } else {
                    // move the process to the cpu queue
                    p->index++;
                    enqueue(cpuQueue, dequeue(ioQueue));
                }
            } else {
                // decrement the burst time
                p->bursts[p->index]--;
            }
        }
        time++;
    }

    printf("Average Turnaround Time: %f\n", turnaroundTime / totalProcesses);

    freeQueue(arrivalQueue);
    freeQueue(cpuQueue);
    freeQueue(ioQueue);
    return 0;
}
