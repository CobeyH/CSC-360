#include <stdlib.h>
#include <stdio.h>
#include "Constants.h"
#include "helpers.h"

void printDate(struct Date date) {
    printf("%04d/%02d/%02d %02d:%02d:%02d\n", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
}

void printBlock(struct RootBlock block) {
    int inUse = block.status & DIRECTORY_ENTRY_USED;
    int isFile = block.status & DIRECTORY_ENTRY_FILE;
    int isDirectory = block.status & DIRECTORY_ENTRY_DIRECTORY;
    if(inUse) {
        if(isFile) {
            printf("F ");
        } else if(isDirectory) {
            printf("D ");
        }
        printf("%10d %30s ", block.fileSize, block.fileName);
        printDate(block.modifyTime);
    }
}

int main(int argc, char *argv[]) {
    if( argc < 2  ) {
        printf ("No input file found");
        exit(0);
    } 
    char *filename = argv[1];
    FILE *inputFile = fopen(filename, "r");
    if(inputFile == NULL) {
        printf("Failed to open input file");
        exit(0);
    }
    struct SuperBlock sBlock;
    // The data is all read into the struct so that individual sections can be accessed
    getSuperBlock(inputFile, &sBlock);
    // Seek to the start of the root directory
    int startPoint = sBlock.rootStart * DEFAULT_BLOCK_SIZE;
    fseek(inputFile, startPoint, SEEK_SET);
    for(int i = 1; i <= sBlock.blocksInRoot * DIRECTORY_ENTRY_PER_BLOCK; i++) {
        struct RootBlock currRootBlock;
        getNextRootBlock(inputFile, &currRootBlock);
        printBlock(currRootBlock);
        fseek(inputFile, startPoint + i * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    }

    fclose(inputFile);
    return 1;
}
