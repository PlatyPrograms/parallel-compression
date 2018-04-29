
#include "compressor.h"

/**
 * Command line arguments:
 *  
 * 1) Input File Name --- File to Compress
 * 2) Key size --- Number of bits to encode at a time
 * 3) Number of decompress proccesses --- Number of processes used for decompression
 *
 */

int main(int argc, char * argv[]){

    int NUMPROCS, MYRANK; 
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &NUMPROCS );
    MPI_Comm_rank( MPI_COMM_WORLD, &MYRANK );

    //Variables used by MASTER_RANK
    char *inputFileName, *dataFileName, *metaFileName, **dataFileNames, **metaFileNames;
    unsigned char *fileBuffer;

    //Variables used by all procs
    unsigned int keySize, numDprocs;
    FILE *inputFile, *dataFile, *metaFile;
    unsigned char *sendBuffer, *recvBuffer;
    u64array counts;
    

    if(MYRANK == MASTER_RANK && argc != 4){
	fprintf(stderr, "Invalid number of input arguments. Got %d, expected 3.\n", (argc-1));
	fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Key size in bits\n(3) Number of processes in decompression\n");
	MPI_Abort(MPI_COMM_WORLD, -1);
    }

    sscanf(argv[2], "%d", &keySize);
    sscanf(argv[3], "%d", &numDprocs);

    //Let the master proc do the fileIO sanity checks
    if(MYRANK == MASTER_RANK){
	
	if(keySize < 1 || keySize > 64){
	    fprintf(stderr, "Invalid key size \"%d\"; must be in range of [1, 64]\n", keySize);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	}

	if(numDprocs < 1){
	    fprintf(stderr, "Invalid number of decompress processes given \"%d\"; must be greater than zero.\n", numDprocs);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	}	
	
	//Generate all the subsequent .data and .meta
	//file names for the worker threads.
	inputFileName = argv[1];

	//Get the number of chars to take before the
	//first '.' of the given inputFileName
	size_t cutoff = (strchr(inputFileName, '.')) - &(inputFileName[0]);
	
	printf("Cutoff is %d\n", cutoff);

	//Setup the dataFileNames and metaFileNames arrays
	dataFileNames = (char **) malloc(sizeof(char *) * numDprocs);
	metaFileNames = (char **) malloc(sizeof(char *) * numDprocs);

	char num[10];
	char data[5] = ".data";
	char meta[5] = ".meta";

	//For each thread, there will be created a .data and .meta file
	for(unsigned int i = 0; i < numDprocs; ++i){
	    
	    sprintf(num, "%u", i);

	    //Allocate the space for each name
	    dataFileNames[i] = malloc(sizeof(char) * (cutoff + 6 + strlen(num)));
	    metaFileNames[i] = malloc(sizeof(char) * (cutoff + 6 + strlen(num)));

	    dataFileNames[i][cutoff + 5 + strlen(num)] = '\0';
	    metaFileNames[i][cutoff + 5 + strlen(num)] = '\0';

	    //Now fill each name in, put the file name first
	    memmove(dataFileNames[i], inputFileName, cutoff);
	    memmove(metaFileNames[i], inputFileName, cutoff);

	    //Then copy the number string
	    for(int j = 0; j < strlen(num); ++j){
		dataFileNames[i][cutoff+j] = num[j];
		metaFileNames[i][cutoff+j] = num[j];		
	    }

	    //Now copy the .data and .meta
	    for(int j = 0; j < 5; ++j){
		dataFileNames[i][cutoff + strlen(num) + j] = data[j];
		metaFileNames[i][cutoff + strlen(num) + j] = meta[j];
	    }

	}    
	
	//print the output file names to double-check them
	for(int i = 0; i < numDprocs; ++i){
	    
	    printf("Outputting data file: %s\n", dataFileNames[i]);
	    printf("Outputting meta file: %s\n", metaFileNames[i]);

	}
    
	//Print some updates for the user
	printf("Reading with keySize of %d bits \n", keySize);
	printf("Number of decompress procs = %d \n", numDprocs);

	//Now let's create the files we will read and write to
	inputFile = fopen(inputFileName, "rb");
	dataFile  = fopen(dataFileName,  "wb");
	metaFile  = fopen(metaFileName,  "wb");
    
	if(!inputFile){
	    fprintf(stderr, "Error opening input file \"%s\"\n", inputFileName);
	    MPI_Abort(MPI_COMM_WORLD, -1);
	}    

	if(!dataFile || !metaFile){
	    fprintf(stderr, "Error creating data and meta files\n");
	    MPI_Abort(MPI_COMM_WORLD, -1);
	}
    }

    //Make sure all is good with input params before continuing
    MPI_Barrier(MPI_COMM_WORLD);

    if(MYRANK == MASTER_RANK){
	fileBuffer = malloc(sizeof(unsigned char) * FILE_BUFFER_SIZE);
	//recvBuffer = malloc(sizeof(unsigned char) * RECV_BUFFER_SIZE);

	if(!fileBuffer){
	    fprintf(stderr, "Error allocating read buffer, not enough memory!\n");
	    MPI_Abort(MPI_COMM_WORLD, -1);
	}
	
	buffIter myIter;
	writeBuff metaWriter;
	writeBuff dataWriter;
	unsigned long int lastPos = 0;
	unsigned long int unusedBits = 0;
	
	uint64_t next = 0;
	uint64_t last = 0;
	uint64_t count = 1;

	size_t validRead = fread(fileBuffer, FILE_BUFFER_SIZE, 1, inputFile);

	initDataFile(dataFile, keySize);
	initWriteBuff(&dataWriter, dataFile, keySize);
	initBuffIter(&myIter, fileBuffer, FILE_BUFFER_SIZE, keySize);
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
	    validRead = fread(fileBuffer, FILE_BUFFER_SIZE, 1, inputFile);
	    setStartOffset(&myIter, unusedBits);
	}

	if(counts.n == 0){
	    --count;
	}

	//When we hit the end of the file, yet still need to process
	//the leftover contents
	if(feof(inputFile)){
	    unsigned long int bytesLeft = ftell(inputFile) - lastPos;
	
	    initBuffIter(&myIter, fileBuffer, bytesLeft, keySize);  

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
	initMetaFile(metaFile, numBits, counts.n, numDprocs);
    
	//Now write the array elements to the meta file at the given bit level
	//Create chars of the elements.

	//For each array element convert it to the numBits format,
	//string them together to then write them as chars.
	initWriteBuff(&metaWriter, metaFile, numBits);

	for(unsigned long int i = 0; i < counts.n; ++i){

	    pushToWriteBuff(&metaWriter, counts.data[i] << (64-numBits));

	}

	closeWriteBuff(&metaWriter);
    }

    //If MYRANK is not the MASTER_RANK, then wait for work
    else{

	//Setup each processes' send and recv buffers. Each is pretty big for now.
	sendBuffer = malloc(sizeof(unsigned char) * SEND_BUFFER_SIZE);	
	recvBuffer = malloc(sizeof(unsigned char) * RECV_BUFFER_SIZE);
	
	
	//Wait for work

	//Process the work

	//Send the work back
	
    }


    MPI_Barrier(MPI_COMM_WORLD);    
    
    //Close the files after we use them
    if(MYRANK == MASTER_RANK){

	fclose(inputFile);
	fclose(dataFile);
	fclose(metaFile);
	
	//Free up any allocations we made
	free(dataFileName);
	free(metaFileName);
	free(counts.data);
	free(fileBuffer);
    }

    //free(sendBuffer);
    //free(recvBuffer);

    MPI_Finalize();
    
    return 0;
}
