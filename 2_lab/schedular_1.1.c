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

bool isFinished(Process* process) {
    return process && process->bursts[process->currentBurst] == 0;
}

bool isProccessOver(Process* process) {
    return process && process->bursts[process->currentBurst] == -1;
}


Process* checkQueueProcess(Queue* queue, int time) {
    if(isQueueEmpty(queue)) {
        return NULL;
    }

    Process* process = peek(queue);
    process->bursts[process->currentBurst]--;
    return process;
}


int increaseTurnAroundTime(Process* process, Queue* queue, int time) {
    int arrivalTime = process->arrivalTime;
    Process* idk = dequeue(queue);
    free(idk->bursts);
    free(idk);
    return (time+1) - arrivalTime;
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
        Process* newlyArrived = peek(arrivalQueue);
        if(newlyArrived && newlyArrived->arrivalTime == time) {
            Process* temp = dequeue(arrivalQueue);
            enqueue(cpuQueue, temp);
        }

        Process* cpuProcess = checkQueueProcess(cpuQueue, time);

            
        if (isFinished(cpuProcess)) {
            cpuProcess->currentBurst++;
            if(isProccessOver(cpuProcess)) {
                turnaroundTime += increaseTurnAroundTime(cpuProcess, cpuQueue, time);
            }
        } else {
            if(isQueueEmpty(ioQueue)) {
                IOWait = true;
            }
            enqueue(ioQueue, dequeue(cpuQueue));
        }
        
        
        Process* ioProcess = NULL;
        if ((ioProcess = checkQueueProcess(ioQueue, time)) && !!IOWait) {
            if (isFinished(ioProcess)) {
                ioProcess->currentBurst++;
                if(isProccessOver(ioProcess)) {
                    turnaroundTime += increaseTurnAroundTime(ioProcess, ioQueue, time);
                }
            } else {
                enqueue(cpuQueue, dequeue(ioQueue));
            }
        }

        time++;
    }

    printf("%.0lf\n", turnaroundTime / totalProcesses); 

    freeQueue(arrivalQueue);
    freeQueue(cpuQueue);
    freeQueue(ioQueue);
    return 0;
}
