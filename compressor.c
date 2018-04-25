
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

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
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Key size in bitse\n");
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
    unsigned char *buffer = malloc(sizeof(unsigned char)*BUFFER_SIZE);
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

    uint64_t currMacroBuff = 0;
    uint64_t nextMacroBuff = 0;

    unsigned long int bufferPos = 0; //Measured in bytes

    unsigned long int lastPos = 0;    
    //This portion reads the file by buffering char values
    while(fread(buffer, BUFFER_SIZE, 1, inputFile) == 1){	
	unsigned long int currPos = ftell(inputFile);

	//Print the buffer contents
	fwrite(buffer, sizeof(char), currPos-lastPos, stdout);

	
	//If the keySize is less than 33, then we need to use
	//the micro buffers on the macro buffers
	if(keySize <= 32){
	    
	    uint64_t currMicroBuff = 0;
	    uint64_t nextMicroBuff = 0;
	    
	}

	//If the keySize is greater than 32 bits, then we can easily
	//just use the macro buffers. Will need to look at every
	//two macro buffers
	else{

	    uint64_t curr = 0;
	    uint64_t next = 0;

	    currMacroBuff = buffer[bufferPos];
	    nextMacroBuff = buffer[bufferPos+8];

	    while(bufferPos+8 < BUFFER_SIZE){
		
		getFirstNBits(&currMacroBuff, &curr, keySize);
		//Now get the next (64 minus keySize) bits from currMacroBuff and
		//then the other (keySize + keySize - 64) bits from nextMicroBuff
		
		getLastNBits(&curr, &next, (64-keySize));
		next = next << (keySize);

		uint64_t temp = 0;
		getFirstNBits(&nextMacroBuff, &temp, (keySize));
		temp = temp >> (64-keySize);

		uint64_t joint = 0;
		joinBits(next, temp, &joint, (64-keySize));
		
	    }

	}
	
	lastPos = ftell(inputFile);
    }    
    if(feof(inputFile)){
	unsigned long int bytesLeft = ftell(inputFile) - lastPos;
	
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
