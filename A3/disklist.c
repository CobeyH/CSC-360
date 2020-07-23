#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "Constants.h"

struct Date {
    int16_t year;
    int8_t month;
    int8_t day;
    int8_t hours;
    int8_t minutes;
    int8_t seconds;
};

struct RootBlock {
    int8_t status;
    int startBlock;
    int numBlocks;
    int fileSize;
    struct Date createTime;
    struct Date modifyTime;
    char fileName[31];
};

struct SuperBlock {
    char  ident[8];
    int16_t blockSize;
    int fileSystemSize;
    int fatStart;
    int blocksInFat;
    int rootStart;
    int blocksInRoot;
};

int readInt32(FILE * inputFile) {
    int32_t buffer;
    fread(&buffer, 1, sizeof(buffer), inputFile);
    return ntohl(buffer);
}

int readInt16(FILE * inputFile) {
    int16_t buffer;
    fread(&buffer, 1, sizeof(buffer), inputFile);
    return ntohs(buffer);
}

void getDate(FILE *inputFile, struct Date *date) {
    date->year = readInt16(inputFile);
    fread(&date->month, 1, 5, inputFile);
}

void getNextRootBlock(FILE *inputFile, struct RootBlock *block) {
    fread(&block->status, 1, 1, inputFile);
    block->startBlock = readInt32(inputFile);
    block->numBlocks  = readInt32(inputFile);
    block->fileSize   = readInt32(inputFile);
    getDate(inputFile, &block->createTime);
    getDate(inputFile, &block->modifyTime);
    fread(&block->fileName, 1, DIRECTORY_MAX_NAME_LENGTH, inputFile);
}

void printDate(struct Date date) {
    printf("%d/%d/%d %d:%d:%d\n", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
}

void printBlock(struct RootBlock block) {
    int isFile = block.status & DIRECTORY_ENTRY_FILE;
    int isDirectory = block.status & DIRECTORY_ENTRY_DIRECTORY;
    if(isFile) {
        printf("F ");
    } else if(isDirectory) {
        printf("D ");
    }
    printf("%10d %30s ", block.fileSize, block.fileName);
    printDate(block.modifyTime);
}

int main(int argc, char *argv[]) {
    if( argc < 2  ) {
        printf ("No input file found");
        exit(0);
    } 
    char *filename = argv[1];
    FILE *inputFile = fopen(filename, "r");
    struct SuperBlock sBlock;
    if(inputFile == NULL) {
        printf("Failed to open input file");
        exit(0);
    }
    // The data is all read into the struct so that individual sections can be accessed
    fread(&sBlock, 1, 30, inputFile);
    // Seek to the start of the root directory
    int startPoint = ntohs(sBlock.rootStart) * DEFAULT_BLOCK_SIZE;
    fseek(inputFile, startPoint, SEEK_SET);
    for(int i = 1; i <= ntohs(sBlock.blocksInRoot); i++) {
        struct RootBlock currRootBlock;
        getNextRootBlock(inputFile, &currRootBlock);
        printBlock(currRootBlock);
        fseek(inputFile, startPoint + i * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    }

    fclose(inputFile);
    return 1;
}
