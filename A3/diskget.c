#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Constants.h"
#include "helpers.h"

void findDirEntry(struct RootBlock *rootBlock, FILE* inputFile, struct SuperBlock sBlock, char fileToMatch[]) {
    // Seek to the start of the root directory
    int startPoint = sBlock.rootStart * DEFAULT_BLOCK_SIZE;
    fseek(inputFile, startPoint, SEEK_SET);
    for(int i = 1; i <= sBlock.blocksInRoot * DIRECTORY_ENTRY_PER_BLOCK; i++) {
        getNextRootBlock(inputFile, rootBlock);
        if(strcmp(rootBlock->fileName, fileToMatch) == 0) { break; }
        fseek(inputFile, startPoint + i * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    }
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
    char *outputFileName = argv[2];
    FILE *inputFile = fopen(filename, "r");
    FILE *outputFile = fopen(outputFileName, "w");
    if(inputFile == NULL) {
        printf("Failed to open disk image");
        exit(0);
    } else if (outputFile == NULL) {
        printf("No output file provided");
        exit(0);
    }
    struct SuperBlock sBlock;
    // The data is all read into the struct so that individual sections can be accessed
    getSuperBlock(inputFile, &sBlock);
    struct RootBlock currRootBlock;
    findDirEntry(&currRootBlock, inputFile, sBlock, outputFileName);
    int fatEntryVal = currRootBlock.startBlock;
    char buffer[DEFAULT_BLOCK_SIZE];
    int amountRead = 0;
    do {
        // Go to the address for the data of the file and write it to the output file
        fseek(inputFile, DEFAULT_BLOCK_SIZE * fatEntryVal, SEEK_SET);
        int amountToRead = min(DEFAULT_BLOCK_SIZE, currRootBlock.fileSize - amountRead);
        fread(buffer, 1, amountToRead, inputFile);
        amountRead += amountToRead;
        fwrite(&buffer, 1, amountToRead, outputFile);
        // Find the next FAT address in the linked list
        fseek(inputFile, DEFAULT_BLOCK_SIZE + fatEntryVal * FAT_ENTRY_SIZE, SEEK_SET);
        fatEntryVal = readInt32(inputFile);
    } while(fatEntryVal != FAT_EOF);

    fclose(inputFile);
    return 1;
}
