#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


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

    int hand = 0;
    int pageFaults = 0;

    int *frames = (int *)malloc(numberOfFrames*sizeof(int));
    for (int i = 0; i < numberOfFrames; i++) {
        frames[i] = -1;
    }
    int *referenceBits = (int *)calloc(numberOfFrames, sizeof(int));

    for (int i = 0; i < length; i++) {
        int page = pageReferences[i];
        int found = 0;
        for (int j = 0; j < numberOfFrames; j++) {
            if (frames[j] == page) {
                found = 1;
                referenceBits[j] = 1; // second chance
                break;
            }
        }
        if (found == 0) {
            pageFaults++;
            while (referenceBits[hand] == 1) {
                referenceBits[hand] = 0;
                hand = (hand + 1) % numberOfFrames;
            }
            frames[hand] = page;
            referenceBits[hand] = 0;
            hand = (hand + 1) % numberOfFrames;
        }
    }

    printf("%d\n", pageFaults);

    free(pageReferences);
    free(frames);
    free(referenceBits);
    return 0;
}