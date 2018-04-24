
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//BUFFER_SIZE measured in bytes
#define BUFFER_SIZE 200

/**
 * Command line arguments:
 *  
 * 1) Input File Name
 * 2) Frame Size
 * 3) Panel Size
 *
 */

int main(int argc, char * argv[]){

    char *inputFileName, *dataFileName, *metaFileName;
    unsigned int frameSize, panelSize;
    FILE *inputFile, *dataFile, *metaFile;
    size_t inputNameLength, cutoff;
    
    if(argc != 4){
	fprintf(stderr, "Invalid number of input arguments. Got %d, expected 3.\n", (argc-1));
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Frame Size\n(3) Panel Size\n");
	return -1;
    }
   
    inputFileName = argv[1];
    inputNameLength = strlen(inputFileName);

    //Get the number of chars to take before the
    //first '.' of the given inputFileName
    cutoff = (strchr(inputFileName, '.')) - &(inputFileName[0]);
    //printf("%p - %p = %d\n", strchr(inputFileName, '.'), &(inputFileName[0]), cutoff);
    
    dataFileName = (char *) malloc(sizeof(char)*(cutoff+5));
    metaFileName = (char *) malloc(sizeof(char)*(cutoff+5));

    //Make a copy of the input file name with
    //all the chars before the '.' extension
    memmove(dataFileName, inputFileName, cutoff);
    memmove(metaFileName, inputFileName, cutoff);

    strcat(dataFileName, ".data");
    strcat(metaFileName, ".meta");    
    
    sscanf(argv[2], "%d", &frameSize);
    sscanf(argv[3], "%d", &panelSize);    

    //Print some updates for the user
    printf("Producing files named: %s and %s\n", dataFileName, metaFileName);
    printf("frameSize = %d bits \t panelSize = %d bits\n", frameSize, panelSize);

    //Now let's create the files and buffer we will be filling
    char *buffer = malloc(sizeof(char)*BUFFER_SIZE);
    inputFile = fopen(inputFileName, "rb");
    dataFile  = fopen(dataFileName,  "wb");
    metaFile  = fopen(metaFileName,  "wb");
    
    if(!inputFile){
	fprintf(stderr, "Error opening input file \"%s\"\n", inputFileName);
	return -1;
    }    

    if(!dataFile || !metaFile){
	fprintf(stderr, "Error creating data and meta files\n");
	return -1;
    }    

    if(!buffer){
	fprintf(stderr, "Error allocating buffer, not enough memory!\n");
	return -1;
    }

    //This portion reads the file by buffering char values

    long int lastPos = 0;

    while(fread(buffer, BUFFER_SIZE, 1, inputFile) == 1){
	long int currPos = ftell(inputFile);

	fwrite(buffer, sizeof(char), currPos-lastPos, stdout);	

	memset(buffer, ' ', sizeof(char)*BUFFER_SIZE);

	lastPos = ftell(inputFile);
    }    
    if(feof(inputFile)){
	long int bytesLeft = ftell(inputFile) - lastPos;
	
	fwrite(buffer, sizeof(char), bytesLeft, stdout);
    }

    //Close the files after we use them
    fclose(inputFile);
    fclose(dataFile);
    fclose(metaFile);

    //Free up any allocations we made
    free(dataFileName);
    free(metaFileName);
    free(buffer);
    
    return 0;
}
