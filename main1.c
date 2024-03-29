#include <stdio.h>
#include <stdlib.h>

int* logicalAddresses;
int* pageNumbers;
int* offsets;

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

    for(int i = 0; i < nAddresses; i++) {
        fscanf(fp, "%d", &logicalAddresses[i]);
    }
    fclose(fp);

    for(int i = 0; i < nAddresses; ++i) {
        logicalAddresses[i] = maskNum(logicalAddresses[i]);
        offsets[i] = (logicalAddresses[i] & 0xff);
        pageNumbers[i] = ((logicalAddresses[i]>>8) & 0xff); 
    }
    printf("address 0: %d\n", logicalAddresses[0]);
    printf("    page  : %d\n", pageNumbers[0]);
    printf("    offset: %d\n", offsets[0]);
    printf("address 999: %d\n", logicalAddresses[999]);
    printf("    page  : %d\n", pageNumbers[999]);
    printf("    offset: %d\n", offsets[999]);

    free(logicalAddresses);
    return 0;
}