#include <stdio.h>
#include <stdlib.h>

struct SuperBlock {
    char ident[8];
    int16_t blockSize;
    int fileSystemSize;
    int fatStart;
    int blocksInFat;
    int rootStart;
    int blocksInRoot;
};

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

void getDate(FILE *inputFile, struct Date *date);

void writeDate(FILE *outputFile, struct Date date);

void getNextRootBlock(FILE *inputFile, struct RootBlock *block);

void getSuperBlock(FILE *inputFile, struct SuperBlock *block);

int readInt32(FILE * inputFile);

int readInt16(FILE * inputFile);

void writeInt32(FILE * outputFile, int32_t buffer);

void writeInt16(FILE * outputFile, int16_t buffer);
