
#include "compressor.h"

/**
 * Command line arguments:
 *  
 * 1) Input File Name --- File to Compress
 * 2) Key size --- Number of bits to encode at a time
 * 3) Number of decompress proccessess --- Number of processes used for decompression
 *
 */

int main(int argc, char * argv[]){

    int NUMPROCS, MYRANK; 
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &NUMPROCS );
    MPI_Comm_rank( MPI_COMM_WORLD, &MYRANK );

    char *inputFileName, *dataFileName, *metaFileName;
    unsigned int keySize, numDprocs;
    FILE *inputFile, *dataFile, *metaFile;
    size_t inputNameLength, cutoff;
    
    if(MYRANK == MASTER_RANK && argc != 4){
	fprintf(stderr, "Invalid number of input arguments. Got %d, expected 3.\n", (argc-1));
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Key size in bitse\n(3) Number of processes in decompression\n");
	return -1;
    }

    sscanf(argv[2], "%d", &keySize);
    sscanf(argv[3], "%d", &numDprocs);

    //Let the master proc do the fileIO and sanity checks
    if(MYRANK == MASTER_RANK){
	
	if(keySize < 1 || keySize > 64){
	    fprintf(stderr, "Invalid key size \"%d\"; must be in range of [1, 64]", keySize);
	    return -1;
	}

	if(numDprocs < 1){
	    fprintf(stderr, "Invalid number of decompress processes given \"%d\"; must be greater than zero.");
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
	printf("Reading with keySize of %d bits \n", keySize);
	printf("Number of decompress procs = %d \n", numDprocs);

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
    }

    //Make sure all is good with input params before continuing
    MPI_Barrier(MPI_COMM_WORLD);

    unsigned char *buffer = malloc(sizeof(unsigned char)*BUFFER_SIZE);

    if(MYRANK == MASTER_RANK && !buffer){
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

    initWriteBuff(&dataWriter, dataFile, keySize);
    initBuffIter(&myIter, buffer, BUFFER_SIZE, keySize);
    u64array_init(&counts);

    advance(&myIter, &next);	
    last = next;

    //This portion reads the file by buffering char values
    while(validRead == 1){	
	unsigned long int currPos = ftell(inputFile);
	
	//Go through the buffer
	while(iterHasNext(&myIter)){    
	   
	    advance(&myIter, &next);
	   
	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{
	
		u64array_push_back(&counts, count);
		pushToWriteBuff(&dataWriter, last);
	
		count = 1;
	    }
	    last = next;
	}

	//Here we account for unused bits from the buffer
	unusedBits = unusedBuffBits(&myIter);

	fseek(inputFile, -(unusedBits/8), SEEK_CUR);

	if(unusedBits % 8){
	    fseek(inputFile, -1, SEEK_CUR);
	}

	if(unusedBits){
	    unusedBits = 8 - (unusedBits % 8);
	}
    
	lastPos = ftell(inputFile);

	//Fill the buffer with the next set of contents
	validRead = fread(buffer, BUFFER_SIZE, 1, inputFile);
	setStartOffset(&myIter, unusedBits);
    }

    if(counts.n == 0){
	--count;
    }

    //When we hit the end of the file, yet still need to process
    //the leftover contents
    if(feof(inputFile)){
	unsigned long int bytesLeft = ftell(inputFile) - lastPos;
	
	initBuffIter(&myIter, buffer, bytesLeft, keySize);  

	while(iterHasNext(&myIter)){    
	   
	    last = next;
	    advance(&myIter, &next);

	    //If we have a match, keep running	    
	    if(next == last){	
		++count;
	    }
	    //If they don't match, write 'last' and 'count' to our files
	    else{

		u64array_push_back(&counts, count);
		pushToWriteBuff(&dataWriter, last);

		count = 1;
	    }
	}
    }
    
    closeWriteBuff(&dataWriter);

    //Now calculate the min bit size for the biggest element of the array.
    unsigned int numBits = 64;

    printf("Biggest run is: %" PRIu64 "\n", counts.biggest);

    for(unsigned int i = 0; i < 64; ++i){
	
	if((counts.biggest << i) & 0x8000000000000000){
	    numBits = 64 - i;
	    break;
	}
    }

    printf("Min num of bits for META counts is: %lu\n", numBits);

    //Write the num of bits to the meta file
    initMetaFile(metaFile, numBits, counts.n);
    
    //Now write the array elements to the meta file at the given bit level
    //Create chars of the elements.

    //For each array element convert it to the numBits format,
    //string them together to then write them as chars.
    initWriteBuff(&metaWriter, metaFile, numBits);

    for(unsigned long int i = 0; i < counts.n; ++i){

	pushToWriteBuff(&metaWriter, counts.data[i] << (64-numBits));

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
