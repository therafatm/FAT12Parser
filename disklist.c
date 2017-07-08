#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utilities.h"

int main(int argc, char * args[]){

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

    char volumeLabel[20];
    memset(volumeLabel, 0, 20);
	getVolumeNameAndFileCountFromRoot(fp, volumeLabel, true);

	return 0;
}
