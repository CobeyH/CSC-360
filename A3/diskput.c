#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include "Constants.h"
#include "helpers.h"

// Returns the index of the first free directory entry. Also moves the file pointer to that position
int findFirstFreeRoot(FILE* diskImage, struct SuperBlock sBlock, struct RootBlock *rootBlock) {
    // Seek to the start of the root directory
    int startPoint = sBlock.rootStart * DEFAULT_BLOCK_SIZE;
    fseek(diskImage, startPoint, SEEK_SET);
    for(int i = 1; i <= sBlock.blocksInRoot * DIRECTORY_ENTRY_PER_BLOCK; i++) {
        getNextRootBlock(diskImage, rootBlock);
        if(!(rootBlock->status & DIRECTORY_ENTRY_USED)) { 
            return i - 1;
        }
        fseek(diskImage, startPoint + i * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    }
    printf("All directory entries are full");
    exit(0);
}

int findNextFreeFAT(FILE* diskImage, int fatStart, int prevIndex) {
    int fatEntryVal;
    fseek(diskImage, fatStart * DEFAULT_BLOCK_SIZE + prevIndex * FAT_ENTRY_SIZE, SEEK_SET);
    int freeBlockPos = prevIndex;
    while(fatEntryVal != FAT_FREE) {
        fatEntryVal = readInt32(diskImage);
        freeBlockPos++;
    }
    // The file pointer should point to the start of the entry so we need to back up to the start of the entry that was just read
    fseek(diskImage, -FAT_ENTRY_SIZE, SEEK_CUR);
    return freeBlockPos;
}

int min(int num1, int num2) {
    return (num1 > num2 ) ? num2 : num1;
}

int main(int argc, char *argv[]) {
    if( argc < 2  ) {
        printf ("No input file found");
        exit(0);
    } 
    char *filename = argv[1];
    char *newFileName = argv[2];

    FILE *diskImage = fopen(filename, "r+");
    FILE *newFile = fopen(newFileName, "r");
    if(diskImage == NULL) {
        printf("Failed to open disk image");
        exit(0);
    } else if (newFile == NULL) {
        printf("File not found.");
        exit(0);
    }
    struct SuperBlock sBlock;
    getSuperBlock(diskImage, &sBlock);
    struct RootBlock newRootBlock;
    struct stat stats;
    stat(newFileName, &stats);
    newRootBlock.fileSize = stats.st_size;
    int amountWritten = 0;
    int blockPos = findFirstFreeRoot(diskImage, sBlock, &newRootBlock);
    int buffer[DEFAULT_BLOCK_SIZE];
    newRootBlock.startBlock = blockPos;
    printf("%d\n", newRootBlock.startBlock);
    do {
        int nextBlockPos = findNextFreeFAT(diskImage, sBlock.fatStart, blockPos);
        blockPos = htonl(blockPos);
        // Write the current block position into the FAT table
        fwrite(&blockPos, 1, FAT_ENTRY_SIZE, diskImage);
        int amountToWrite = min(DEFAULT_BLOCK_SIZE, newRootBlock.fileSize - amountWritten);
        fread(buffer, 1, amountToWrite, newFile);

        blockPos = nextBlockPos;

    } while(amountWritten < newRootBlock.fileSize);

    /* do { */
    /*     fread(buffer, 1, amountToRead, diskImage); */
    /*     amountRead += amountToRead; */
    /*     fwrite(&buffer, 1, amountToRead, newFile); */
    /*     fseek(diskImage, DEFAULT_BLOCK_SIZE + fatEntryVal * FAT_ENTRY_SIZE, SEEK_SET); */
    /*     fatEntryVal = readInt32(diskImage); */
    /* } while(fatEntryVal != FAT_EOF); */

    fclose(diskImage);
    fclose(newFile);
    return 1;
}
