#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "Constants.h"

struct SuperBlock {
    char    ident[8];
    int16_t blockSize;
    int32_t fileSystemSize;
    int32_t fatStart;
    int32_t blocksInFat;
    int32_t rootStart;
    int32_t blocksInRoot;
};

int main(int argc, char *argv[]) {
    if( argc < 2  ) {
        printf ("No input file found");
        exit(0);
    } 
    char *filename = argv[1];
    FILE *inputFile = fopen(filename, "r");
    struct SuperBlock block;
    if(inputFile == NULL) {
        printf("Failed to open input file");
        exit(0);
    }
    // The data is all read into the struct so that individual sections can be accessed
    fread(&block, 1, 30, inputFile);
    printf("Super block information:\n");
    printf("Block size: %d\nBlock count: %d\nFAT starts: %d\nFAT blocks: %d\nRoot directory start: %d\nRoot directory blocks: %d\n",
            ntohs(block.blockSize), ntohs(block.fileSystemSize), ntohs(block.fatStart), ntohs(block.blocksInFat), ntohs(block.rootStart), ntohs(block.blocksInRoot));

    int free = 0, reserved = 0, allocated = 0;
    int32_t entry;
    fseek(inputFile, ntohs(block.fatStart) * DEFAULT_BLOCK_SIZE - 1, SEEK_SET);
    for(int i = 0; i < ntohs(block.fileSystemSize); i++) {
        fread(&entry, 1, FAT_ENTRY_SIZE, inputFile);
        switch(entry) {
            case FAT_FREE:
                free++;
                break;
            case FAT_RESERVED:
                reserved++;
                break;
            default:
                allocated++;
        }
    }
    printf("\nFAT information:\nFree Blocks: %d\nReserved Blocks: %d\nAllocated Blocks: %d", free, reserved, allocated);
    fclose(inputFile);
    return 1;
}
