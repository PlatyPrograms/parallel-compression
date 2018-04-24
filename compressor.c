
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comp_funcs.h"

//BUFFER_SIZE measured in bytes
#define BUFFER_SIZE 200

/**
 * Command line arguments:
 *  
 * 1) Input File Name --- File to Compress
 * 2) Key size --- Number of bits to encode at a time
 *
 */

int main(int argc, char * argv[]){

    char *inputFileName, *dataFileName, *metaFileName;
    unsigned int keySize;
    FILE *inputFile, *dataFile, *metaFile;
    size_t inputNameLength, cutoff;
    
    if(argc != 3){
	fprintf(stderr, "Invalid number of input arguments. Got %d, expected 2.\n", (argc-1));
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Frame Size\n(3) Panel Size\n");
	return -1;
    }
   
    inputFileName = argv[1];
    inputNameLength = strlen(inputFileName);

    //Get the number of chars to take before the
    //first '.' of the given inputFileName
    cutoff = (strchr(inputFileName, '.')) - &(inputFileName[0]);
    
    dataFileName = (char *) malloc(sizeof(char)*(cutoff+5));
    metaFileName = (char *) malloc(sizeof(char)*(cutoff+5));

    //Make a copy of the input file name with
    //all the chars before the '.' extension
    memmove(dataFileName, inputFileName, cutoff);
    memmove(metaFileName, inputFileName, cutoff);

    strcat(dataFileName, ".data");
    strcat(metaFileName, ".meta");    
    
    sscanf(argv[2], "%d", &keySize);
    
    //Print some updates for the user
    printf("Producing files named: %s and %s\n", dataFileName, metaFileName);
    printf("keySize = %d bits \n", keySize);

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
	fprintf(stderr, "Error allocating read buffer, not enough memory!\n");
	return -1;
    }

    //TODO: Start by writing the key size to the data file
    //and by writing the first six placeholder bits of the meta file

    //This portion reads the file by buffering char values
    long int lastPos = 0;
    unsigned long long longestRun = 0;

    while(fread(buffer, BUFFER_SIZE, 1, inputFile) == 1){
	long int currPos = ftell(inputFile);
	
	//Print the buffer contents
	//fwrite(buffer, sizeof(char), currPos-lastPos, stdout);	
	
	//Now that the char buffer is filled, we can start reading
	//the elements with the given key size, then writing them
	
	//For each element in the buffer, read up to n bits at a time
	for(long int i = 0; i < BUFFER_SIZE; ++i){
	    long int readUpTo = 0;
	    //for(; readUpTo){

	    //}
	}


	lastPos = ftell(inputFile);
    }    
    if(feof(inputFile)){
	long int bytesLeft = ftell(inputFile) - lastPos;
	
	//Print the buffer contents
	//fwrite(buffer, sizeof(char), bytesLeft, stdout);
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
