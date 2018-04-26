
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "buffIter.h"
#include "writeBuff.h"

/**
 * Command line arguments:
 *  
 * 1) Input File Name --- File to Compress
 * 2) Key size --- Number of bits to encode at a time
 *
 */

void initMetaFile(FILE * metaFile, unsigned int keySize){

    //Write first 48 bits as all zeros
    fputc(0, metaFile);
    fputc(0, metaFile);
    fputc(0, metaFile);
    fputc(0, metaFile);
    fputc(0, metaFile);
    fputc(0, metaFile);

    //Write next 8 bits as keySize
    fputc((int) keySize, metaFile);
}


void initDataFile(FILE * dataFile, unsigned int keySize){

    //Write first 8 bits as keySize
    fputc(keySize, dataFile);
}


int main(int argc, char * argv[]){

    char *inputFileName, *dataFileName, *metaFileName;
    unsigned int keySize;
    FILE *inputFile, *dataFile, *metaFile;
    size_t inputNameLength, cutoff;
    
    if(argc != 3){
	fprintf(stderr, "Invalid number of input arguments. Got %d, expected 2.\n", (argc-1));
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Key size in bitse\n");
	return -1;
    }

    sscanf(argv[2], "%d", &keySize);

    if(keySize < 1 || keySize > 64){
	fprintf(stderr, "Invalid key size \"%d\"; must be in range of [1, 64]", keySize);
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
    
    //Print some updates for the user
    printf("Producing files named: %s and %s\n", dataFileName, metaFileName);
    printf("keySize = %d bits \n", keySize);

    //Now let's create the files we will read and write to
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

    unsigned char *buffer = malloc(sizeof(unsigned char)*BUFFER_SIZE);

    if(!buffer){
	fprintf(stderr, "Error allocating read buffer, not enough memory!\n");
	return -1;
    }

    initMetaFile(metaFile, keySize);
    initDataFile(dataFile, keySize);

    buffIter myIter;
    writeBuff metaWriter;
    writeBuff dataWriter;
    unsigned long int lastPos = 0;

    initWriteBuff(&metaWriter, metaFile, keySize);
    initWriteBuff(&dataWriter, dataFile, keySize);

    //This portion reads the file by buffering char values
    while(fread(buffer, BUFFER_SIZE, 1, inputFile) == 1){	
	unsigned long int currPos = ftell(inputFile);

	initBuffIter(&myIter, buffer, BUFFER_SIZE, keySize);

	//Print the buffer contents
	fwrite(buffer, sizeof(char), currPos-lastPos, stdout);

	printf("\n");	

	uint64_t next = 0;
	uint64_t last = 0;
	uint64_t count = 1;
	uint64_t bitIdx = keySize;


	advance(&myIter, &next);
	//printf("Step %lu Read in %" PRIx64 "\n", myIter.currStep, next);

	//Go through the buffer
	while(iterHasNext(&myIter)){    
	   
	    last = next;
	    advance(&myIter, &next);
	    //printf("Step %lu Read in %" PRIx64 "\n", myIter.currStep, next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
		//printf("Writing in: %" PRIx64 "\n", last);
		pushToWriteBuff(&dataWriter, last);
		printf("Buffer Post Push : %" PRIx64 "\n", dataWriter.buff);
		count = 1;
	    }	    	    
	}
	//Keep in mind that the writeBuff still has to post its contents
	//just move on to the next buffer and continue it from there
	
	//When it has run out, we need to grab the next buffer
	//but need to correctly offset it to account for the
	//bits we never used from this last buffer.

	return 0;

	//printf("\n");

	lastPos = ftell(inputFile);
    }    
    if(feof(inputFile)){
	unsigned long int bytesLeft = ftell(inputFile) - lastPos;
	
	//Print the buffer contents
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
