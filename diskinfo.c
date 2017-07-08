#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utilities.h"

int main(int argc, char ** args){

	if(argc < 2){
        printf("Please try again with a filename\n");
        return -1;
    }    

    char * fileName = args[1];    
    if(fileName == NULL){
        printf("Error when reading from args[]\n");
        return -1;
    }

    FILE * fp = fopen(fileName, "r");
    if(fp == NULL){
        perror("Error reading given input file");
        return -1;
    }

    char osname[20];
	getOSName(fp,osname);

    char volumeLabelBS[12];
    char volumeLabelRD[10];    
    getVolumeLabelFromBoot(fp, volumeLabelBS);

    int totalFilesInRootDir = getVolumeNameAndFileCountFromRoot(fp, volumeLabelRD, false); 
    int sectorsPerFat = getSectorsPerFat(fp);
    int numberOfFats = getNumberOfFats(fp);

    long freeDiskSpace = getFreeSpaceOnDisk(fp);
    long diskSize = getDiskSize(fp);

	printf("\nOS Name: %s\n", osname);
	if(strcmp(volumeLabelRD, volumeLabelBS) != 0 && strlen(volumeLabelBS) > 0){
		//printf("Volume label read from boot sector and root directory are different.\n");
		printf("Label of the disk: %s\n", volumeLabelRD);   
	} else{
		printf("Label of the disk: %s\n", volumeLabelBS);
	}

    printf("Total size of the disk: %ld\n", diskSize);
    printf("Free size of the disk:  %ld\n", freeDiskSpace); // -1 for boot sector
	printf("==============\nThe number of files in the root directory (not including subdirectories): %d\n", totalFilesInRootDir);
	printf("\n=============\nNumber of FAT copies: %d\n",numberOfFats);
	printf("Sectors per FAT: %d\n\n", sectorsPerFat);

    fclose(fp);
	return 1;
}