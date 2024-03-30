#include <stdio.h>
#include <stdlib.h>

int* logicalAddresses;
int* pageNumbers;
int* offsets;
char** physicalMem;

int maskNum(int dec) {
    int mask = 0xffff;
    return (mask & dec);
}

int main(int argc, char** argv) {
    FILE* fp = fopen("addresses.txt", "r");
    int nAddresses = 1000; //TODO find actual number of addresses in the file
    logicalAddresses = (int*)malloc(sizeof(int) * nAddresses);
    pageNumbers = (int*)malloc(sizeof(int) * nAddresses);
    offsets = (int*)malloc(sizeof(int) * nAddresses);
    physicalMem = (char**)malloc(sizeof(char *) * 256);
    int pageTable[256];
    for(int i = 0; i < 256; ++i) {
        pageTable[i] = -1;
        physicalMem[i] = NULL;
    }

    for(int i = 0; i < nAddresses; i++) {
        fscanf(fp, "%d", &logicalAddresses[i]);
    }
    fclose(fp);

    FILE* backing = fopen("BACKING_STORE.bin", "rb");
    for(int i = 0; i < nAddresses; ++i) {
        int frameNumber;
        logicalAddresses[i] = maskNum(logicalAddresses[i]);
        offsets[i] = (logicalAddresses[i] & 0xff);
        pageNumbers[i] = ((logicalAddresses[i]>>8) & 0xff); 
        int currPage = pageNumbers[i];

        //check if page is in page table
        //if it is, get frame number
        frameNumber = pageTable[currPage];
        if(frameNumber == -1) {
            //if not, bring the page in from backing store
            fseek(backing, currPage*256, SEEK_SET);
            int freeFrame = currPage; //find first free frame. in this case, current page number
            physicalMem[freeFrame] = (char *)malloc(256 * sizeof(char));
            fread(physicalMem[freeFrame], sizeof(char), 256, backing);

            pageTable[currPage] = freeFrame;
            frameNumber = pageTable[currPage];
        } else {
            printf("page found\n");
        }
        
    }
    fclose(backing);

    printf("address 0 : %d\n", logicalAddresses[0]);
    printf("    page  : %d\n", pageNumbers[0]);
    printf("    offset: %d\n", offsets[0]);
    printf("address 999: %d\n", logicalAddresses[999]);
    printf("    page  : %d\n", pageNumbers[999]);
    printf("    offset: %d\n", offsets[999]);

    free(logicalAddresses);
    free(pageNumbers);
    free(offsets);
    for(int i = 0; i < 256; ++i) {
        free(physicalMem[i]);
    }
    free(physicalMem);
    return 0;
}