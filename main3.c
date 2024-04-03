#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define PHYSICAL_MEM_SIZE (PAGE_TABLE_SIZE * PAGE_SIZE)

int* logicalAddresses;
int* pageNumbers;
int* offsets;
char* physicalMem;
int* pageTable;
int* LRU_timestamps;

int searchTLB(int pageNumber, int TLB[16][2]) {
    for(int i = 0; i < 16; ++i) {
        if(TLB[i][0] == pageNumber) {
            return TLB[i][1];
        }
    }
    return -1; //TLB miss
}

// Mask the least significant 16 bits
int maskNum(int dec) {
    return (dec & 0xFFFF);
}

// Initialize LRU timestamps
void initLRU() {
    LRU_timestamps = (int*)malloc(sizeof(int) * PAGE_TABLE_SIZE);
    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        LRU_timestamps[i] = 0;
    }
}

// Update timestamp for a page
void updateLRU(int pageNumber) {
    LRU_timestamps[pageNumber] = time(NULL);
}

// Find the least recently used page
int findLRUPage() {
    int minTimestamp = LRU_timestamps[0];
    int lruPage = 0;
    for (int i = 1; i < PAGE_TABLE_SIZE; ++i) {
        if (LRU_timestamps[i] < minTimestamp) {
            minTimestamp = LRU_timestamps[i];
            lruPage = i;
        }
    }
    return lruPage;
}

// Function to handle LRU page replacement
void handlePageFault(int pageNumber, FILE* backing) {
    int lruPage = findLRUPage();
    pageTable[lruPage] = pageNumber; // Update page table
    updateLRU(lruPage); // Update LRU timestamp

    // Read data from backing store
    fseek(backing, pageNumber * PAGE_SIZE, SEEK_SET);
    fread(physicalMem + (lruPage * PAGE_SIZE), sizeof(char), PAGE_SIZE, backing);
}

int main(int argc, char** argv) {
    FILE* fp = fopen("addresses.txt", "r");
    FILE* out_f1 = fopen("out1.txt", "w"); // Open output file
    FILE* out_f2 = fopen("out2.txt", "w");
    FILE* out_f3 = fopen("out3.txt", "w");
    int nAddresses = 1000; // Assume 1000 addresses, as per the provided sample
    logicalAddresses = (int*)malloc(sizeof(int) * nAddresses);
    pageNumbers = (int*)malloc(sizeof(int) * nAddresses);
    offsets = (int*)malloc(sizeof(int) * nAddresses);
    physicalMem = (char*)malloc(sizeof(char) * PHYSICAL_MEM_SIZE);
    pageTable = (int*)malloc(sizeof(int) * PAGE_TABLE_SIZE);
    int TLB[16][2];

    // Initialize TLB
    for(int i = 0; i < 16; ++i) {
        TLB[i][0] = -1;
        TLB[i][1] = -1;
    }

    // Initialize page table
    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        pageTable[i] = -1; // Mark all entries as invalid
    }

    // Initialize LRU timestamps
    initLRU();

    FILE* backing = fopen("BACKING_STORE.bin", "rb");

    int fifoTLBIndex = 0;
    int TLBmisses = 0;
    for (int i = 0; i < nAddresses; ++i) {
        fscanf(fp, "%d", &logicalAddresses[i]);
        logicalAddresses[i] = maskNum(logicalAddresses[i]);
        offsets[i] = (logicalAddresses[i] & 0xFF);
        pageNumbers[i] = ((logicalAddresses[i] >> 8) & 0xFF);

        int pageNumber = pageNumbers[i];
        int offset = offsets[i];

        // Search TLB
        int frameNumber = searchTLB(pageNumber, TLB);

        if(frameNumber == -1) { // TLB miss
            TLBmisses++;
            frameNumber = pageTable[pageNumber];
            if (frameNumber == -1) { // Page fault
                handlePageFault(pageNumber, backing);
                frameNumber = pageNumber;
            }

            // Update TLB
            TLB[fifoTLBIndex][0] = pageNumber;
            TLB[fifoTLBIndex][1] = frameNumber;
            fifoTLBIndex = (fifoTLBIndex + 1) % 16;
        }

        int physicalAddress = (frameNumber * PAGE_SIZE) + offset;
        char value = physicalMem[physicalAddress];

        printf("Virtual Address: %d, Physical Address: %d, Value: %d\n", logicalAddresses[i], physicalAddress, value);

        fprintf(out_f1,"%d\n", logicalAddresses[i]);
        fprintf(out_f2,"%d\n", physicalAddress);
        fprintf(out_f3,"%d\n", value);
    }

    fclose(fp);
    fclose(backing);
    fclose(out_f1); // Close the output file

    free(logicalAddresses);
    free(pageNumbers);
    free(offsets);
    free(physicalMem);
    free(pageTable);
    free(LRU_timestamps);

    return 0;
}

