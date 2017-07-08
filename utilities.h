#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#define DEBUG false

void removeSpaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = '\0';
}

void getOSName(FILE * fp, char string[]){

    fseek(fp, 3, SEEK_SET);
    fread(string, (size_t) sizeof(char), 8, fp);
    string[9] = '\0';
    return;
}

int getBytesPerSector(FILE * fp){

	int bytesPerSector;
	fseek(fp, 11, SEEK_SET);
    fread(&bytesPerSector, (size_t) sizeof(char), 2, fp);
    return bytesPerSector;
}

int countUsedSectors(FILE * fp, int bytesPerSector, int sectorsPerFat){

	int currentEntry = 2;
	int maxEntries = bytesPerSector * sectorsPerFat * 2/3;

	int usedSectors = 0;
	int fatIndexValue = 0;
	for(currentEntry; currentEntry < maxEntries; currentEntry++){

		fatIndexValue = getFATIndexValue(fp, currentEntry);
		if(fatIndexValue != 0){
			usedSectors++;
		}
	}

	return usedSectors;
}

long getFreeSpaceOnDisk(FILE * fp){

	int bytesPerSector = getBytesPerSector(fp);
	//printf("bytesPerSector: %d\n", bytesPerSector);
	int sectorsPerFat = getSectorsPerFat(fp);
	//printf("sectorsPerFat: %d\n", sectorsPerFat);
	int diskSize = getDiskSize(fp);
	//pr("disksize: %ld\n", diskSize);
	long usedSpace = (countUsedSectors(fp, bytesPerSector, sectorsPerFat) + 2) * 512;
	//printf("usedspace: %ld\n", usedSpace);
	return (diskSize - usedSpace);
}

int getDiskSize(FILE * fp){

	int bytesPerSector = getBytesPerSector(fp);
	int totalSectorCount = getTotalSectorCount(fp);
	return totalSectorCount * bytesPerSector;
}

int getTotalSectorCount(FILE * fp){

    fseek(fp, 19, SEEK_SET);
	int lowerhalf = getc(fp);
	int upperhalf = getc(fp);

    int totalSectorCount = (upperhalf << 8) | lowerhalf;
  	//printf("Total sector count: %d", totalSectorCount);
    return totalSectorCount;
}

int getNumberOfFats(FILE * fp){

	fseek(fp, 16, SEEK_SET);
    int numberOfFats = getc(fp);
    return numberOfFats;
}

int getSectorsPerFat(FILE * fp){

	fseek(fp, 22, SEEK_SET);
	int lowerhalf = getc(fp);
	int upperhalf = getc(fp);
    int sectorsPerFat = (upperhalf << 8) | lowerhalf;
    return sectorsPerFat;
}

int getFATIndexValue(FILE * fp, int fatIndex){

	int offset = 512;
	int upper = 0;
	int lower = 0;
	int value = 0;
	int lowerIndex = (3 * fatIndex)/2;
	int upperIndex = lowerIndex + 1;

	//printf("Pre-fat index: %d\n", fatIndex);

	if(fatIndex % 2 == 0){
		fseek(fp, offset + lowerIndex, SEEK_SET);
		lower = getc(fp);
		fseek(fp, offset + upperIndex, SEEK_SET);
		upper = (getc(fp) & 0x0F);
		//printf("lower: %d, upper: %d\n", lower, upper);
		value = (upper << 8) | lower;
	}

	else if(fatIndex % 2 == 1){
		fseek(fp, offset + lowerIndex, SEEK_SET);
		lower = (getc(fp) & 0xF0) >> 4;
		fseek(fp, offset + upperIndex, SEEK_SET);
		upper = getc(fp);
		//printf("lower: %d, upper: %d\n", lower, upper);
		value = (upper << 4) | lower;
	}

	//printf("Post-fat index value: %d\n", value);
	return value;
}

void getVolumeLabelFromBoot(FILE * fp, char volumeLabelBS[]){

	fseek(fp, 43, SEEK_SET);
    fread(volumeLabelBS, (size_t) sizeof(char), (size_t) 11, fp);
    volumeLabelBS[11] = '\0';
}

void getDateString(int date, char dateString[]){

	//1111 1110 0000 0000
	//printf("CREATION DATE: %d\n", date);
	int year = ((date & 0xFe00) >> 9);
	//printf("CREATION YEAR: %d\n", year);
	year += 1980;
	char yearStr[5];
	sprintf(yearStr, "%d", year);
	//0000 0001 1110 0000
	int month = (date & 0x01e0) >> 5;
	char monthStr[5];
	sprintf(monthStr, "%d", month);
	//0000 0000 0001 1111
	int day = (date & 0x001F);
	char dayStr[5];
	sprintf(dayStr, "%d", day);

	strcat(dateString, yearStr);
	strcat(dateString, "-");
	strcat(dateString, monthStr);
	strcat(dateString, "-");
	strcat(dateString, dayStr);
}

void getTimeString(int time, char timeString[]){

	//1111 1000 0000 0000
	//printf("CREATION TIME: %d\n", time);
	int hours = (time & 0xF800) >> 11;
	char hourStr[5];
	sprintf(hourStr, "%d", hours);

	//0000 0111 1110 0000
	int minutes = (time & 0x07e0) >> 5;
	char minuteStr[5];
	sprintf(minuteStr, "%d", minutes);

	if(hours < 10){
		strcat(timeString, "0");
	}
	strcat(timeString, hourStr);
	strcat(timeString, ":");

	if(minutes < 10){
		strcat(timeString, "0");
	}
	strcat(timeString, minuteStr);
}

void getFileNameAndExtension(FILE * fp, int rootDirStartOffset, int i, char fileName[], char extension[]){

	fseek(fp, rootDirStartOffset + (i * 32), SEEK_SET);
	fread(fileName, (size_t) sizeof(char), (size_t) 8, fp);
	removeSpaces(fileName);
	fread(extension, (size_t) sizeof(char), (size_t) 3, fp);
}

void printFileInfo(FILE * fp, int rootDirStartOffset, int i, char isDirectory){

	long fileSize;
	fseek(fp, rootDirStartOffset + (i * 32) + 28, SEEK_SET);
	fread(&fileSize, (size_t) 1 , (size_t) sizeof(long), fp);

	char fileName[13];
	memset(fileName, 0, (size_t) 12);
	char extension[4];
	memset(fileName, 0, (size_t) 4);

	getFileNameAndExtension(fp, rootDirStartOffset, i, fileName, extension);
	strcat(fileName, ".");
	strcat(fileName, extension);

	fseek(fp, rootDirStartOffset + (i * 32) + 16, SEEK_SET);
	int creationDate;
	fread(&creationDate, (size_t) sizeof(char), (size_t) 2, fp);
	char dateString[20];
	memset(dateString, (size_t) 0, (size_t) 20);
	getDateString(creationDate, dateString);

	fseek(fp, rootDirStartOffset + (i * 32) + 14, SEEK_SET);
	int lowerhalf = getc(fp);
	int upperhalf = getc(fp);
	int creationTime = (upperhalf << 8) | lowerhalf;
	char timeString[50];
	memset(timeString, (size_t) 0, (size_t) 50);
	getTimeString(creationTime, timeString);
	char fileType = isDirectory ? 'D' : 'F';
	printf("%c %10d %20s %12s %s\n", fileType, fileSize, fileName, dateString, timeString);
}

int getPhysicalSectorNumber(int number){
	return (31 + number);
}

//checks if file exists and sets values of firstLogicalCluster, fileSize
bool checkIfFileExists(FILE * fp, char fileToFind[], int * firstLogicalCluster, long * fileSize){

	int bytesPerSector = getBytesPerSector(fp);
	int rootDirStartOffset = 19 * bytesPerSector;
	int i = 0;
	int totalRootDirectoryEntries = 16 * 14;
	char attributes;
	bool fileFound = false;
	char firstByte = 0;
	char extension[4];
	char fileName[30];
	memset(fileName, 0, (size_t) sizeof(char) * 30);
	memset(extension, 0, (size_t) sizeof(char) * 4);

	for(i; i < totalRootDirectoryEntries; i++){

		fseek(fp, rootDirStartOffset + (i* 32), SEEK_SET);
		firstByte = getc(fp);
		fseek(fp, 10, SEEK_CUR);
		fread(&attributes, (size_t) sizeof(char), (size_t) 1, fp);

		char empty = 0xE5;
		if(firstByte != empty && firstByte != 0){
			bool isDirectory = (attributes & 0x0810) == 0x10;
			bool isVolumeLabel = (attributes & 0x08) == 0x08;
			if(!isVolumeLabel && !isDirectory){

				getFileNameAndExtension(fp, rootDirStartOffset, i, fileName, extension);
				if(strlen(extension) > 0){
					strcat(fileName, ".");
					strcat(fileName, extension);
					fileName[strlen(fileName) + strlen(extension) + 1] = '\0';
				}
				else{
					fileName[strlen(fileName)] = '\0';
				}
				//printf("Filename: %s\n", fileName);

				if(strcmp(fileName, fileToFind) == 0){
					fileFound = true;
					fseek(fp, rootDirStartOffset + (i * 32) + 26, SEEK_SET);
					fread(firstLogicalCluster, (size_t) sizeof(char), (size_t)2, fp);
					fseek(fp, rootDirStartOffset + (i * 32) + 28, SEEK_SET);
					fread(fileSize, (size_t) sizeof(long) , (size_t) 1, fp);
					break;
				}
			}
		}

		// memset(fileName,0, sizeof(fileName));
		// memset(extension,0, sizeof(extension));
		memset(fileName, 0, sizeof(char) * 30);
		memset(extension, 0, sizeof(char) * 4);
	}

	return fileFound;
}

void getExtensionAndSetFileName(char fileName[], char extension[], char * lastIndex){

	//extension has max/min len of 4
	*lastIndex = '\0';
	lastIndex++;
	strcpy(extension, lastIndex);
	return;
}

int findEmptyRootDirectoryIndex(FILE * fp){

	int rootDirStartOffset = 19 * getBytesPerSector(fp);
	int totalRootDirectoryEntries = 16 * 14;
	char firstByte = 0;
	int i = 0;
	for(i; i < totalRootDirectoryEntries; i++){

		fseek(fp, rootDirStartOffset + (i* 32), SEEK_SET);
		firstByte = getc(fp);
		//printFileInfo(fp, rootDirStartOffset, i, false);
		char empty = 0xE5;
		if(firstByte == empty || firstByte == 0){
			// directory entry is free
			break;
		}
	}

	return i;
}

int findEmptyFatIndex(FILE * fp, int i){

	int currentEntry = i > 2 ? i : 2;
	int maxEntries = getBytesPerSector(fp) * getSectorsPerFat(fp) * 2/3;
	int fatIndexValue = 0;

	for(currentEntry; currentEntry < maxEntries; currentEntry++){
		fatIndexValue = getFATIndexValue(fp, currentEntry);
		if (fatIndexValue == 0x00){
			return currentEntry;
			}
	}
}

void writeToFATIndex(FILE * fp, int fatIndex, int valueToWrite){

	int offset = 512;
	int upper = 0;
	int lower = 0;
	int lowerIndex = (3 * fatIndex)/2;
	int upperIndex = lowerIndex + 1;
	//printf("Writing to FAT table:\n");
	//printf("Value to write: %d\n", valueToWrite);

	if(fatIndex % 2 == 0){

		fseek(fp, offset + lowerIndex, SEEK_SET);
		// lower = getc(fp);
		//printf("test1\n");
		lower = (0x00FF & valueToWrite);
		//printf("test2\n");
		//printf("lower: %d\n", lower);

		fwrite(&lower, (size_t) sizeof(char), (size_t) 1, fp);

		fseek(fp, offset + upperIndex, SEEK_SET);
		//upper = (getc(fp) & 0x0F);
		upper = getc(fp) | ((valueToWrite & 0xF00) >> 8);
		fseek(fp, offset + upperIndex, SEEK_SET);
		fwrite(&upper, (size_t) sizeof(char), (size_t) 1, fp);
		//printf("upper: %d\n", upper);
		//printf("lower: %d, upper: %d\n", lower, upper);
	}

	else if(fatIndex % 2 == 1){
		fseek(fp, offset + lowerIndex, SEEK_SET);
		//lower = (getc(fp) & 0xF0) >> 4;
		lower = (((valueToWrite & 0x0F) << 4) | getc(fp)) & 0xFF;
		fseek(fp, offset + lowerIndex, SEEK_SET);
		fwrite(&lower, (size_t) sizeof(char), (size_t)1, fp);

		fseek(fp, offset + upperIndex, SEEK_SET);
		upper = (0x0FF0 & valueToWrite) >> 4;
		//printf("lower: %d, upper: %d\n", lower, upper);
		fwrite(&upper, (size_t) sizeof(char), (size_t)1, fp);
	}

	return;
}

unsigned int getDate(){

    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

	//0100 1001 0111 1000
	unsigned int date = 0;
	//1111 1110 0000 0000
	unsigned int year = timeinfo->tm_year + 1900 - 1980;
	year = year << 9;
	//0000 0001 1110 0000
	unsigned int  month = timeinfo->tm_mon + 1;
	month = month << 5;

	//0000 0000 0001 1111
	unsigned int  day = timeinfo->tm_mday & 0x1F;

	date = (year | month | day) & 0xFFFF;
	return date;
}

unsigned int getTime(){

    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    //1001 1001 0110 0000
    unsigned int time = 0;
	//1111 1000 0000 0000
	unsigned int hours = (timeinfo->tm_hour);
	hours = hours << 11;

	//0000 0111 1110 0000
	unsigned int minutes = (timeinfo->tm_min);
	minutes = minutes << 5;

	time = (hours | minutes) & 0xFFFF;
    //printf("[%d-%d-%d %d:%d]\n",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min);
	return time;

}

void copyFileToDisk(FILE * fp, FILE * dp, char fname[], long fileSize){

	int bytesPerSector = getBytesPerSector(dp);
	char extension[4];
	memset(extension, 0, (size_t) sizeof(char) * 4);
	int rootDirStartOffset = 19 * bytesPerSector;

	char * lastOccurenceOfDot = strrchr(fname, '.');
	char fileName[8];
	memset(fileName, 0, 8 * sizeof(char));

	//seperate filename and extension
	if(lastOccurenceOfDot != NULL){
		//printf("lastOccurenceOfDot: %s\n", lastOccurenceOfDot);
		char * t = fname;
		int i = 0;
		while(t != lastOccurenceOfDot){
			if(i>7){
				break;
			}
			fileName[i] = *t;
			i++;
			t++;
		}
		lastOccurenceOfDot++;
		strcpy(extension, lastOccurenceOfDot);
	}
	else{
		//printf("strlen: %d\n", strlen(fname));
		if(strlen(fname) > 8){
			printf("File name too long. This case is not accounted for\n");
			return;
		}

		printf("FILENAME: %s\n", fileName);
		printf("EXTENSION: %s\n", extension);
		printf("FILESIZE: %ld\n", fileSize);
		strcat(fileName,fname);
	}

	int emptyRootDirIndex = findEmptyRootDirectoryIndex(dp);
	// printf("Empty rdi: %d\n", emptyRootDirIndex);
	int emptyFatIndex = findEmptyFatIndex(dp, 0);
	int currentSectorToWrite = 0x00;

	// ----------------------------------------------------------------------
	// write in root directory
	// ----------------------------------------------------------------------
	int offset = rootDirStartOffset + (emptyRootDirIndex * 32);
	fseek(dp, offset, SEEK_SET);

	fwrite(fileName, (size_t) sizeof(char), (size_t) strlen(fileName), dp);
	fseek(dp, offset + 8, SEEK_SET);
	fwrite(extension, (size_t) sizeof(char), (size_t) strlen(extension), dp);

	int attribute = 0;
	fseek(dp, offset + 11, SEEK_SET);
	fwrite(&attribute, (size_t) 1, (size_t) sizeof(char), dp);

	unsigned int time = getTime();
	unsigned int date = getDate();

	offset = rootDirStartOffset + (emptyRootDirIndex * 32) + 14;
	fseek(dp, offset, SEEK_SET);
	//Creation Time
	fwrite(&time, (size_t) sizeof(char), (size_t) 2, dp);
	//Creation Date
	fwrite(&date, (size_t) sizeof(char), (size_t) 2, dp);
	//Last Access Date
	fwrite(&date, (size_t) sizeof(char), (size_t) 2, dp);

	fseek(dp, (size_t) 2, SEEK_CUR);
	//Last Write Time
	fwrite(&time, (size_t) sizeof(char), (size_t) 2, dp);
	//Last Date Time
	fwrite(&date, (size_t) sizeof(char), (size_t) 2, dp);

	int firstLogicalCluster = (emptyFatIndex & 0xFF);
	offset = rootDirStartOffset + (emptyRootDirIndex * 32) + 26;
	fseek(dp , offset, SEEK_SET);
	fwrite(&firstLogicalCluster, (size_t) sizeof(char), (size_t) 2 , dp);
	// printf("First logical cluster: %d\n", firstLogicalCluster);
	fwrite(&fileSize, (size_t) sizeof(char), (size_t)4, dp);
	// ----------------------------------------------------------------------
	// ----------------------------------------------------------------------
	//printf("Current empty fat index: %d\n", emptyFatIndex);

	int nextEmptyFatIndex = (fileSize > bytesPerSector) ? findEmptyFatIndex(dp, firstLogicalCluster + 1) : firstLogicalCluster;
	//printf("Next empty fat index: %d\n", nextEmptyFatIndex);

	char buffer[512];
	memset(buffer, 0, (size_t) sizeof(char) * 512);

	long transmittedBytes = 0;
	int bufferSize = 0;
	int bytesRead = 0;
	int totalBytesWritten = 0;
	int bytesWritten = 0;

	while(transmittedBytes < fileSize){

		//printf("Current empty fat index: %d\n", emptyFatIndex);
		bufferSize = (512 + transmittedBytes) > fileSize ? (fileSize - transmittedBytes) : 512;
		//printf("Current buffer size: %d\n", bufferSize);
		//write next logical cluster sector number in FAT
		fseek(dp, 512 + emptyFatIndex, SEEK_SET); // 512 for skipping bootsector
		//printf("Next empty fat index: %d\n", nextEmptyFatIndex);

		if(!(transmittedBytes + bufferSize >= fileSize)){
			writeToFATIndex(dp, emptyFatIndex, nextEmptyFatIndex);
		} else{
			writeToFATIndex(dp, emptyFatIndex, 0xFFF8);
		}

		//read file from current directory
		fseek(fp, transmittedBytes, SEEK_SET);
		bytesRead = fread(buffer, (size_t) sizeof(char), (size_t)bufferSize, fp);
		if(bytesRead > 0 && DEBUG){
			printf("READ SUCCESS: %d\n", bytesRead);
		}

		// printf("----------------------------PRINTING BUFFER----------------------------\n");
		// printf("%s\n", buffer);
		// printf("----------------------------PRINTING BUFFER----------------------------\n");

		currentSectorToWrite = getPhysicalSectorNumber(emptyFatIndex);
		// printf("currentEmptyFatIndex: %d\n", emptyFatIndex);
		// printf("currentSectorToWrite: %d\n", currentSectorToWrite);

		fseek(dp, currentSectorToWrite * 512, SEEK_SET);
		bytesWritten = fwrite(buffer, (size_t) sizeof(char), (size_t) bufferSize, dp);
		if(bytesWritten > 0 && DEBUG){
			printf("WRITE SUCCESS: %d\n", bytesWritten);
		}

		emptyFatIndex = nextEmptyFatIndex;
		nextEmptyFatIndex = findEmptyFatIndex(dp, emptyFatIndex + 1);
		transmittedBytes += bytesWritten;
		memset(buffer, 0, (size_t) sizeof(char) * 512);
	}

	// printf("Filesize: %d\n", fileSize);
	// printf("transmittedBytes: %d\n", transmittedBytes);
	return;
}

void copyFileFromDisk(FILE * fp, char fileToCopy[]){

	int bytesPerSector = getBytesPerSector(fp);
	int firstLogicalCluster;
	long fileSize;
	bool fileFound = checkIfFileExists(fp, fileToCopy, &firstLogicalCluster, &fileSize);

	if(!fileFound){
		printf("File with file name %s not on disk.\n", fileToCopy);
		return;
	}

	FILE * newFile = fopen(fileToCopy, "wb");
	char buffer[512];
	int nextPhysicalSector;
	int fatIndexValue;
	int transmittedBytes = 0;
	int bufferSize = 0;
	int bytesRead = 0;
	int bytesWritten = 0;
	int diff;

	// printf("Filesize: %d\n", fileSize);
	while(transmittedBytes < fileSize){
		memset(buffer, 0, (size_t) 512);
		diff = fileSize - transmittedBytes;
		if(diff > 512){
			bufferSize = 512;
		}
		else{
			bufferSize = diff;
		}

		// printf("Current buffersize: %d\n", bufferSize);
		nextPhysicalSector = getPhysicalSectorNumber(firstLogicalCluster);
		// check FAT for next sector number
		fatIndexValue = getFATIndexValue(fp, firstLogicalCluster);
		//printf("FAT INDEX VALUE: %d\n", fatIndexValue);
		if(fatIndexValue == 0){
			//printf("Fat index value is zero\n");
			break;
		}

		if(fatIndexValue == 0x00){
			printf("File allocation table for the corresponding file points to unused clusters.\nThis is not good. Diskget will now quit.\n");
			break;
		}

		if(fatIndexValue >= 0xFF0 && fatIndexValue <= 0xFF7){
			printf("File allocation table for the corresponding file points to reserved or bad clusters.\nThis is not good. Diskget will now quit.\n");
			break;
		}

		firstLogicalCluster = fatIndexValue;
		fseek(fp, nextPhysicalSector * 512, SEEK_SET);
		bytesRead = fread(buffer, (size_t) sizeof(char), (size_t) bufferSize, fp);
		bytesWritten = fwrite(buffer, (size_t) sizeof(char), (size_t) bufferSize, newFile);
		transmittedBytes += bytesWritten;
	}

	printf("Copied bytes: %d\n", transmittedBytes);
	fclose(newFile);
	return;
}

int getVolumeNameAndFileCountFromRoot(FILE * fp, char volumeLabel[], bool printDir){

	int bytesPerSector = getBytesPerSector(fp);
	int rootDirStartOffset = 19 * bytesPerSector;
	int i = 0;
	int totalRootDirectoryEntries = 16 * 14;
	char attributes;
	bool flag = false;
	char firstByte;
	int totalFiles = 0;

	for(i; i < totalRootDirectoryEntries; i++){

		fseek(fp, rootDirStartOffset + (i* 32), SEEK_SET);
		firstByte = getc(fp);
		fseek(fp, 10, SEEK_CUR);
		fread(&attributes, (size_t) sizeof(char), (size_t) 1, fp);

		if(!flag){
			if(attributes == 0x08){
				fseek(fp, rootDirStartOffset + (i* 32), SEEK_SET); //each directory is 32 bytes long
				fread(volumeLabel, (size_t) sizeof(char), (size_t) 8, fp);
				flag = true;
			}
		}

		char empty = 0xE5;
		if(firstByte != empty && firstByte != 0){
			bool isDirectory = (attributes & 0x10) == 0x10;
			bool isVolumeLabel = (attributes & 0x08) == 0x08;
			if(!isVolumeLabel && !isDirectory){
				totalFiles++;
				if(printDir){
					printFileInfo(fp, rootDirStartOffset, i, isDirectory);
				}
			}
		}
	}

	return totalFiles;
}
