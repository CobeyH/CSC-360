#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <netinet/in.h>
#include <time.h>
#include "Constants.h"
#include "helpers.h"

// Returns the index of the first free directory entry. Also moves the file pointer to that position
int findFirstFreeRoot(FILE* diskImage, struct SuperBlock sBlock, struct RootBlock *rootBlock, char *fileName) {
    // Seek to the start of the root directory
    int startPoint = sBlock.rootStart * DEFAULT_BLOCK_SIZE;
    fseek(diskImage, startPoint, SEEK_SET);
    for(int i = 1; i <= sBlock.blocksInRoot * DIRECTORY_ENTRY_PER_BLOCK; i++) {
        getNextRootBlock(diskImage, rootBlock);
        if(strcmp(rootBlock->fileName, fileName) == 0) {
            printf("ERROR: File with the same name already exists in the file system");
            exit(0);
        }
        if(!(rootBlock->status & DIRECTORY_ENTRY_USED)) { 
            return i - 1;
        }
        fseek(diskImage, startPoint + i * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    }
    printf("ERROR: All directory entries are full");
    exit(0);
}

int findNextFreeFAT(FILE* diskImage, int fatStart, int prevIndex) {
    fseek(diskImage, fatStart * DEFAULT_BLOCK_SIZE + prevIndex * FAT_ENTRY_SIZE, SEEK_SET);
    int fatEntryVal = readInt32(diskImage);
    int freeBlockPos = prevIndex;
    while(fatEntryVal != FAT_FREE) {
        fatEntryVal = readInt32(diskImage);
        freeBlockPos++;
    }
    // The file pointer should point to the start of the entry so we need to back up to the start of the entry that was just read
    return freeBlockPos;
}

void fillDate(struct Date *date) {
    time_t rawTime = time(NULL);
    struct tm *fullTime = localtime(&rawTime);
    // Year function returns years since 1900 so we need to add 1900
    date->year = 1900 + fullTime->tm_year;
    date->month = fullTime->tm_mon + 1;
    date->day = fullTime->tm_mday;
    date->hours = fullTime->tm_hour;
    date->minutes = fullTime->tm_min;
    date->seconds = fullTime->tm_sec;
}

void printDate(struct Date date) {
    printf("%04d/%02d/%02d %02d:%02d:%02d\n", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
}

void fillRootStats(struct RootBlock *dirEntry, char* fileName) {
    struct stat stats;
    stat(fileName, &stats);
    switch (stats.st_mode & S_IFMT) {
        case S_IFDIR:  dirEntry->status = 0b101;    break;
        case S_IFREG:  dirEntry->status = 0b11;     break;
        default:       printf("Unsupported File type passed\n");   break;
    }
    dirEntry->fileSize = stats.st_size;
    double numBlocks = (dirEntry->fileSize) / (double)DEFAULT_BLOCK_SIZE;
    dirEntry->numBlocks = ceil(numBlocks);
    for(int i = 0; i < DIRECTORY_MAX_NAME_LENGTH; i++) {
        dirEntry->fileName[i] = fileName[i];
    }
    fillDate(&dirEntry->createTime);
    fillDate(&dirEntry->modifyTime);
    printDate(dirEntry->createTime);
}

void writeDirEntryToFile(FILE *file, int writePos, struct RootBlock dirEntry, struct SuperBlock sBlock) {
    printf("Decimal Address of Root address %d\n", sBlock.rootStart * DEFAULT_BLOCK_SIZE + writePos * DIRECTORY_ENTRY_SIZE);
    fseek(file, sBlock.rootStart * DEFAULT_BLOCK_SIZE + writePos * DIRECTORY_ENTRY_SIZE, SEEK_SET);
    fwrite(&dirEntry.status, 1, 1, file);
    writeInt32(file, dirEntry.startBlock);
    writeInt32(file, dirEntry.numBlocks);
    writeInt32(file, dirEntry.fileSize);
    writeDate(file, dirEntry.createTime);
    printDate(dirEntry.createTime);
    writeDate(file, dirEntry.modifyTime);
    fwrite(&dirEntry.fileName, 1, 31, file);
}

int min(int num1, int num2) {
    return (num1 > num2 ) ? num2 : num1;
}

int main(int argc, char *argv[]) {
    const int fatEnd = FAT_EOF;
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
    int amountWritten = 0;
    int rootEntryPos = findFirstFreeRoot(diskImage, sBlock, &newRootBlock, newFileName);
    fillRootStats(&newRootBlock, newFileName);
    int blockPos = findNextFreeFAT(diskImage, sBlock.fatStart, 0);
    newRootBlock.startBlock = blockPos;
    writeDirEntryToFile(diskImage, rootEntryPos, newRootBlock, sBlock);
    char buffer[DEFAULT_BLOCK_SIZE];
    while(1) {    
        // Write the current block position into the FAT table
        int amountToWrite = min(DEFAULT_BLOCK_SIZE, newRootBlock.fileSize - amountWritten);
        fread(buffer, 1, amountToWrite, newFile);
        fseek(diskImage, DEFAULT_BLOCK_SIZE * blockPos, SEEK_SET);
        fwrite(buffer, 1, amountToWrite, diskImage);
        amountWritten += amountToWrite;
        printf("Where are we writing to %d\n", blockPos);
        if(amountWritten < newRootBlock.fileSize) {
            int nextBlock = findNextFreeFAT(diskImage, sBlock.fatStart, blockPos + 1);
            int blockForNetwork = htonl(nextBlock);
            fseek(diskImage, DEFAULT_BLOCK_SIZE * sBlock.fatStart + blockPos * FAT_ENTRY_SIZE, SEEK_SET);
            fwrite(&blockForNetwork, 1, FAT_ENTRY_SIZE, diskImage);
            fseek(diskImage, DEFAULT_BLOCK_SIZE * sBlock.fatStart + nextBlock * FAT_ENTRY_SIZE, SEEK_SET);
            blockPos = nextBlock;
        } else {
            fseek(diskImage, DEFAULT_BLOCK_SIZE * sBlock.fatStart + blockPos * FAT_ENTRY_SIZE, SEEK_SET);
            fwrite(&fatEnd, 1, FAT_ENTRY_SIZE, diskImage);
            break;
        }
    };

    fclose(diskImage);
    fclose(newFile);
    return 1;
}
