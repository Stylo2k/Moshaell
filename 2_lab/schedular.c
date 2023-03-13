#include "common.h"

Process* readProccess() {
    int i = 0;
    int temp = EOF;
    // read the arrival time
    if (!scanf("%d", &temp)) {
        fprintf(stderr, "Error while reading input");
        exit(EXIT_FAILURE);
        // just to make the compiler happy
        return NULL;
    }
    // end of file, return null
    if (temp == EOF) return NULL;
    
    
    int* bursts = (int*)calloc(SIZE, sizeof(int));
    Process *p = (Process *)calloc(1, sizeof(Process));

    p->arrivalTime = temp;

    while (scanf("%d", &temp)) {
        bursts[i] = temp;
        if (temp == -1) {
            break;
        }
        i++;
    }

    p->bursts = bursts;
    p->maxIndex = i;
    p->currentBurst = 0;

    return p;
}


bool finishedBurst(Process* process) {
    return process && process->bursts[process->currentBurst] == 0;
}


bool noBursts(Process* process) {
    return process && process->bursts[process->currentBurst] == -1;
}


void removeProcess(Queue* queue) {
    Process* idk = dequeue(queue);
    free(idk->bursts);
    free(idk);
}


double checkQueue(Queue* queue, Queue* cpuQueue, bool IOWait) {
    if (isQueueEmpty(queue) || IOWait) {
        return -1;
    }
    Process* process = peek(queue);
    process->bursts[process->currentBurst]--;
    if (finishedBurst(process)) {
        process->currentBurst++;
        if (noBursts(process)){
            removeProcess(queue);
            return process->arrivalTime;
        }
        else{
            enqueue(cpuQueue, dequeue(queue));
        }
    }
    return -1;
}

double checkCpuQueue(Queue* queue1, Queue* queue2, bool* IOWait) {
    if (isQueueEmpty(queue1) || (*IOWait)) {
        return -1;
    }
    Process* process = peek(queue1);
    process->bursts[process->currentBurst]--;
    if (finishedBurst(process)) {
        process->currentBurst++;
        if (noBursts(process)){
            removeProcess(queue1);
            return process->arrivalTime;
        }
        else{
            if(isQueueEmpty(queue2)) {
                (*IOWait) = true;
            }
            enqueue(queue2, dequeue(queue1));
        }
    }
    return -1;
}


int main() {
    Queue *arrivalQueue = createQueue(); 
    Queue *cpuQueue = createQueue();
    Queue *ioQueue = createQueue();

    Process *process = NULL;
    while ((process = readProccess())) {
        enqueue(arrivalQueue, process);
    }

    double totalProcesses = arrivalQueue->numProcesses;
    int time = 0;
    double turnaroundTime = 0;
    
    while (!isQueueEmpty(arrivalQueue) || !isQueueEmpty(cpuQueue) || !isQueueEmpty(ioQueue)) {
        bool IOWait = false;

        // check if any processes have arrived
        if(peek(arrivalQueue) && peek(arrivalQueue)->arrivalTime == time) {
            Process* temp = dequeue(arrivalQueue);
            enqueue(cpuQueue, temp);
        }

        double newTime = -1;
    
        newTime = checkCpuQueue(cpuQueue, ioQueue, &IOWait);
        if (newTime != -1) {
            turnaroundTime += (time+1) - newTime;
        }

        newTime = checkQueue(ioQueue, cpuQueue, IOWait);
        if (newTime != -1) {
            turnaroundTime += (time+1) - newTime;
        }

        time++;
    }

    printf("%.0lf\n", turnaroundTime / totalProcesses); 

    freeQueue(arrivalQueue);
    freeQueue(cpuQueue);
    freeQueue(ioQueue);
    return 0;
}