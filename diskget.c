#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

int main(int argc, char * args[]){

	if(argc < 3){
        printf("Please try again with a filename to copy from disk\n");
        return -1;
    }    

    char * fileName = args[1];    
    if(fileName == NULL){
        printf("Error when reading from args[]: %s\n", args[1]);
        return -1;
    }

    FILE * fp = fopen(fileName, "r");
    if(fp == NULL){
        printf("Error when reading from disk %s\n", args[1]);
        return -1;
    }

    copyFileFromDisk(fp, args[2]);
	return 0;
}