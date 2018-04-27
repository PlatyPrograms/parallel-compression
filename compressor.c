
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
    unsigned long int unusedBits = 0;

    uint64_t next = 0;
    uint64_t last = 0;
    uint64_t count = 1;

    size_t validRead = fread(buffer, BUFFER_SIZE, 1, inputFile);

    initWriteBuff(&metaWriter, metaFile, keySize);
    initWriteBuff(&dataWriter, dataFile, keySize);
    initBuffIter(&myIter, buffer, BUFFER_SIZE, keySize);
    
    advance(&myIter, &next);	
    last = next;

    //This portion reads the file by buffering char values
    while(validRead == 1){	
	unsigned long int currPos = ftell(inputFile);

	//Print the buffer contents
	//fwrite(buffer, sizeof(char), currPos-lastPos, stdout);
	
	//Go through the buffer
	while(iterHasNext(&myIter)){    
	   
	    //last = next;
	    advance(&myIter, &next);
	    printf("next is: %" PRIx64 "\n", next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
		printf("Pushing to write %" PRIx64 "\n", last);
		printf("Counted: %" PRIu64 "\n", count);
		pushToWriteBuff(&dataWriter, last);
		printf("Buffer post push: %" PRIx64 "\n", dataWriter.buff);
		count = 1;
	    }
	    last = next;
	}
	
	unusedBits = unusedBuffBits(&myIter);

	printf("End of buffer hit! Count is:%" PRIu64 "\n",count);
	printf("Current unused bits: %lu\n", unusedBits);
	printf("Current file pos: %lu\n", currPos);

	//Need to change fread() to account for unused bits from this iteration
	//Take the current position and move the file pointer back a few bytes
	//based on how many unused bits there are.
	fseek(inputFile, -(unusedBits/8), SEEK_CUR);

	if(unusedBits % 8){
	    fseek(inputFile, -1, SEEK_CUR);
	}

	if(unusedBits){
	    unusedBits = 8 - (unusedBits % 8);
	}
    
	lastPos = ftell(inputFile);
      
	printf("File pos correction: %lu\n", lastPos);	
	printf("moving on to next buffer\n");
	printf("Unused bits: %lu\n\n", unusedBits);

	//Fill the buffer with the next set of contents
	validRead = fread(buffer, BUFFER_SIZE, 1, inputFile);
	setStartOffset(&myIter, unusedBits);
    }

    if(feof(inputFile)){
	unsigned long int bytesLeft = ftell(inputFile) - lastPos;
	
	//Print the buffer contents
	//fwrite(buffer, sizeof(char), bytesLeft, stdout);

	initBuffIter(&myIter, buffer, bytesLeft, keySize);  

	while(iterHasNext(&myIter)){    
	   
	    //last = next;
	    advance(&myIter, &next);
	    printf("END next is: %" PRIx64 "\n", next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
		printf("END Pushing to write %" PRIx64 "\n", last);
		printf("END Counted: %" PRIu64 "\n", count);
		pushToWriteBuff(&dataWriter, last);
		printf("END Buffer post push: %" PRIx64 "\n", dataWriter.buff);
		count = 1;
	    }
	    last = next;
	}

    }
    
    closeWriteBuff(&dataWriter);

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
