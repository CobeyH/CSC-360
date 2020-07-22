#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "Constants.h"

struct RootBlock {
    unsigned char status;
    int32_t startBlock;
    int32_t numBlocks;
    int32_t fileSize;
    unsigned char createTime[7];
    unsigned char modifyTime[7];
    unsigned char fileName[31];
};

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
    // Seek to the start of the root directory
    fseek(inputFile, ntohs(block.rootStart) * DEFAULT_BLOCK_SIZE, SEEK_SET);
    printf("Start: %4X \n", ntohs(block.rootStart) * DEFAULT_BLOCK_SIZE);
    struct RootBlock file;
    fread(&file, 1, DIRECTORY_ENTRY_SIZE, inputFile);
    printf("Status: %2X \n", file.status);
    printf("Start Block: %4X \n", file.startBlock);
    printf("Num Blocks: %4X \n", file.numBlocks); 
    printf("fileSize: %4X \n", ntohs(file.fileSize));
    for(int i = 0; i < 31; i++) {
        printf("%c", file.fileName[i]);
    }
    printf("\n");

    fclose(inputFile);
    return 1;
}
