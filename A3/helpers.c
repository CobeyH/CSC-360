#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "Constants.h"

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

void writeInt32(FILE * outputFile, int32_t buffer) {
    int32_t toWrite = htonl(buffer);
    fwrite(&toWrite, 1, sizeof(toWrite), outputFile);
}

void writeInt16(FILE * outputFile, int16_t buffer) {
    int16_t toWrite = htons(buffer);
    fwrite(&toWrite, 1, sizeof(toWrite), outputFile);
}

void getSuperBlock(FILE *inputFile, struct SuperBlock *block) {                                                                                                                     fread(&block->ident, 1, 8, inputFile);
    block->blockSize      = readInt16(inputFile);
    block->fileSystemSize = readInt32(inputFile);
    block->fatStart       = readInt32(inputFile);
    block->blocksInFat    = readInt32(inputFile);
    block->rootStart      = readInt32(inputFile);
    block->blocksInRoot   = readInt32(inputFile);
}

void getDate(FILE *inputFile, struct Date *date) {
    date->year = readInt16(inputFile);
    fread(&date->month, 1, 5, inputFile);
}

void writeDate(FILE *outputFile, struct Date date) {
    writeInt16(outputFile, date.year);
    fwrite(&date.month   , 1, 1, outputFile);
    fwrite(&date.day     , 1, 1, outputFile);
    fwrite(&date.hours   , 1, 1, outputFile);
    fwrite(&date.minutes , 1, 1, outputFile);
    fwrite(&date.seconds , 1, 1, outputFile);
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
