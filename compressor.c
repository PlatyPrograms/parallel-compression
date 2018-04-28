
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

void initMetaFile(FILE * metaFile, unsigned int lengthOfRunInBits, unsigned long int numRuns){

    //Write first 48 bits as all zeros
    for(int i = 1; i <= 6; ++i){
	unsigned char chunk = (numRuns >> (48 - i*8)) & 0xFF;

	fputc(chunk, metaFile);	
    }
   
    //Write next 8 bits as the length of a run in bits
    fputc((int) lengthOfRunInBits, metaFile);
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

    initDataFile(dataFile, keySize);

    buffIter myIter;
    writeBuff metaWriter;
    writeBuff dataWriter;
    unsigned long int lastPos = 0;
    unsigned long int unusedBits = 0;

    uint64_t next = 0;
    uint64_t last = 0;
    uint64_t count = 1;

    u64array counts;

    size_t validRead = fread(buffer, BUFFER_SIZE, 1, inputFile);

    initWriteBuff(&dataWriter, dataFile, keySize, 0);
    initBuffIter(&myIter, buffer, BUFFER_SIZE, keySize);
    u64array_init(&counts);

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
	    //printf("next is: %" PRIx64 "\n", next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
		//printf("Pushing to write %" PRIx64 "\n", last);
		//printf("Counted: %" PRIu64 "\n", count);
		//fputc(count & 0xFF, metaFile);
		u64array_push_back(&counts, count);
		pushToWriteBuff(&dataWriter, last);
		//printf("Buffer post push: %" PRIx64 "\n", dataWriter.buff);
		count = 1;
	    }
	    last = next;
	}
	
	unusedBits = unusedBuffBits(&myIter);

	//printf("End of buffer hit! Count is:%" PRIu64 "\n",count);
	//printf("Current unused bits: %lu\n", unusedBits);
	//printf("Current file pos: %lu\n", currPos);

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
      
	//printf("File pos correction: %lu\n", lastPos);	
	//printf("moving on to next buffer\n");
	//printf("Unused bits: %lu\n\n", unusedBits);

	//Fill the buffer with the next set of contents
	validRead = fread(buffer, BUFFER_SIZE, 1, inputFile);
	setStartOffset(&myIter, unusedBits);
    }

    if(feof(inputFile)){
	unsigned long int bytesLeft = ftell(inputFile) - lastPos;

	if(counts.n == 0){
	    --count;
	}
	
	//Print the buffer contents
	//fwrite(buffer, sizeof(char), bytesLeft, stdout);

	initBuffIter(&myIter, buffer, bytesLeft, keySize);  

	while(iterHasNext(&myIter)){    
	   
	    last = next;
	    advance(&myIter, &next);
	    //printf("END next is: %" PRIx64 "\n", next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
		//printf("END Pushing to write %" PRIx64 "\n", last);
		//printf("END Counted: %" PRIu64 "\n", count);
		//fputc(count & 0xFF, metaFile);
		u64array_push_back(&counts, count);
		pushToWriteBuff(&dataWriter, last);
		//printf("END Buffer post push: %" PRIx64 "\n", dataWriter.buff);
		count = 1;
	    }
	    //last = next;
	}
    }
    
    closeWriteBuff(&dataWriter);

    //Now calculate the min bit size for the biggest element of the array.
    unsigned int numBits = 64;

    printf("Biggest run is: %" PRIu64 "\n", counts.biggest);
    //printf("We got %lu runs!\n", counts.n);

    for(unsigned int i = 0; i < 64; ++i){
	
	if((counts.biggest << i) & 0x8000000000000000){
	    numBits = 64 - i;
	    break;
	}
    }

    printf("Min num of bits is: %lu\n", numBits);

    //printf("Our array has %lu elements\n", counts.n);
    //Write the num of bits to the meta file
    initMetaFile(metaFile, numBits, counts.n);
    
    //Now write the array elements to the meta file at the given bit level
    //Create chars of the elements.

    //For each array element convert it to the numBits format,
    //string them together to then write them as chars.
    initWriteBuff(&metaWriter, metaFile, numBits, 0);

    //printf("Key size of meta writer is %lu\n", metaWriter.keySize);

    for(unsigned long int i = 0; i < counts.n; ++i){
	//printf("Pushed " " %" PRIx64 " to meta file\n", counts.data[i]);
	pushToWriteBuff(&metaWriter, counts.data[i] << (64-numBits));
	//printf("Buffer looks like: %" PRIx64 "\n\n", metaWriter.buff);
    }

    closeWriteBuff(&metaWriter);
    
    //Close the files after we use them
    fclose(inputFile);
    fclose(dataFile);
    fclose(metaFile);

    //Free up any allocations we made
    free(dataFileName);
    free(metaFileName);
    free(buffer);
    free(counts.data);
    
    return 0;
}
