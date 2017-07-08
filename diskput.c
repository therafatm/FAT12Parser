#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

int main(int argc, char * args[]){

	if(argc < 3){
        printf("Please try again with a filename\n");
        return -1;
    }    

    char * diskName = args[1];    
    char * fileToPut = args[2];

    if(diskName == NULL){
        printf("Error when reading from given disk argument: %s\n", args[1]);
        return -1;
    }

    if(fileToPut == NULL){
        printf("Error when reading from file argument: %s\n", args[2]);
        return -1;
    }

    FILE * dp = fopen(diskName, "rb+");
    if(dp == NULL){
        printf("Error when reading from disk %s\n", args[1]);
        return -1;
    }

    long freeDiskSpace = getFreeSpaceOnDisk(dp);
    //printf("free: %ld\n", freeDiskSpace);
    FILE * fp = fopen(fileToPut, "rb");
    if(fp == NULL){
    	printf("File %s not found\n", fileToPut);
        return -1;
    }

	fseek(fp, 0L, SEEK_END);
	long fileSize = ftell(fp);
    if(fileSize == -1){
        fileSize = 0;
    }

	if(fileSize > freeDiskSpace){
		printf("Not enough free space on the disk image.\n");
		return -1;
	}

	copyFileToDisk(fp, dp, args[2], fileSize);
    fclose(fp);
    fclose(dp);
	return 0;
}