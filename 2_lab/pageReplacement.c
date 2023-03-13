#include "libQueue.h"

int main(void) {
    int numberOfFrames = 0;
    scanf("%d", &numberOfFrames);

    // scan in the page references
    int *pageReferences = (int *)calloc(100, sizeof(int));
    int length = 0;
    int temp;
    while (scanf("%d", &temp) == 1) {
        pageReferences[length] = temp;
        length++;
    }

    Queue queue = newQueue(numberOfFrames);
    
    int pageFaults = 0;

    for (int i = 0; i < length; i++) {
        int page = pageReferences[i];
        int found = isInQueue(queue, page);
        if (found == 0) {
            pageFaults++;
            if (queue.index == numberOfFrames) {
                dequeue(&queue);
            }
            enqueue(page, &queue);
        }
    }

    printf("%d\n", pageFaults);
    freeQueue(queue);
    free(pageReferences);
    return 0;
}