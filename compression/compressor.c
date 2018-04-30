#include <sys/time.h>
//#include <linux/time.h>
#include "compressor.h"

/**
 * Command line arguments:
 *  
 * 1) Input File Name --- File to Compress
 * 2) Key size --- Number of bits to encode at a time
 *
 */

int main(int argc, char * argv[]){
  
  int NUMPROCS, MYRANK; 
  MPI_Init( &argc, &argv );
  MPI_Comm_size( MPI_COMM_WORLD, &NUMPROCS );
  MPI_Comm_rank( MPI_COMM_WORLD, &MYRANK );
  
  struct timeval tvStart, tvEnd;
  if(MYRANK == MASTER_RANK){
    gettimeofday(&tvStart, 0);
  }

  //Variables used by MASTER_RANK
  char *inputFileName, **dataFileNames, **metaFileNames;
  unsigned char *fileBuffer;
  unsigned long int bufferSize, inputFileSize;
  FILE *inputFile;

  //Vars used by workers
  unsigned char *recvBuffer;
  unsigned long int recvBuffSize;

  //Variables used by all procs
  char *dataFileName, *metaFileName;
  unsigned char *myBuffer;
  unsigned long int myBufferSize;
  MPI_Status status;
  unsigned int keySize, numDprocs;
  FILE *myDataFile, *myMetaFile;
  u64array counts;
  size_t cutoff;

    

  if(MYRANK == MASTER_RANK && argc != 3){
    fprintf(stderr, "Invalid number of input arguments. Got %d, expected 2.\n", (argc-1));
    fprintf(stderr, "Expected Arguments:\n(1) Input File Name\n(2) Key size in bits\n\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  //Do some input argument setup, assuming their validity
  inputFileName = argv[1];

  //Get the number of chars to take before the
  //first '.' of the given inputFileName    
  cutoff = (strchr(inputFileName, '.')) - &(inputFileName[0]);

  //Get the key size
  sscanf(argv[2], "%d", &keySize);
  numDprocs = NUMPROCS;

  //Let the master proc do the fileIO sanity checks
  //and also generate the file names for each thread
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
	
    //Setup the dataFileNames and metaFileNames arrays
    dataFileNames = (char **) malloc(sizeof(char *) * numDprocs);
    metaFileNames = (char **) malloc(sizeof(char *) * numDprocs);

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
	
    //Print some updates for the user
    printf("Reading with keySize of %d bits \n", keySize);
    printf("Number of decompress procs = %d \n", numDprocs);

    //Now let's create the files we will read and write to
    inputFile = fopen(inputFileName, "rb");
    //dataFile  = fopen(dataFileName,  "wb");
    //metaFile  = fopen(metaFileName,  "wb");
    
    if(!inputFile){
      fprintf(stderr, "Error opening input file \"%s\"\n", inputFileName);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }    
  }

  //Make sure all is good with input params before continuing
  MPI_Barrier(MPI_COMM_WORLD);

  //Get the number for our filename
  sprintf(num, "%u", MYRANK);

  //Calculate my .data file and my .meta file names
  dataFileName = (char *) malloc(sizeof(char) * (cutoff + 6 + strlen(num)));
  metaFileName = (char *) malloc(sizeof(char) * (cutoff + 6 + strlen(num)));

  dataFileName[cutoff + 5 + strlen(num)] = '\0';
  metaFileName[cutoff + 5 + strlen(num)] = '\0';

  memmove(dataFileName, inputFileName, cutoff);
  memmove(metaFileName, inputFileName, cutoff);
    
  //Then copy the number string
  for(int j = 0; j < strlen(num); ++j){
    dataFileName[cutoff+j] = num[j];
    metaFileName[cutoff+j] = num[j];		
  }
    
  //Now copy the .data and .meta
  for(int j = 0; j < 5; ++j){
    dataFileName[cutoff + strlen(num) + j] = data[j];
    metaFileName[cutoff + strlen(num) + j] = meta[j];
  }    
    
  MPI_Barrier(MPI_COMM_WORLD);

  //Next, let each process know how large of a buffer it will need
  if(MYRANK == MASTER_RANK){

    //Calculate the buffer size to use for sending
    inputFileSize = getFileSize(inputFileName);
    bufferSize = (inputFileSize/NUMPROCS);

    printf("Input filesize is: %lu\n", inputFileSize);

    //Setup our output file buffer
    fileBuffer = malloc(sizeof(unsigned char) * bufferSize);

    //The buffer size has been overshot to account for a file that is not evenly divisible
    //We will need to communcate how much of the buffer eahc process needs to read.

    //For each other process, tell it its buffer size
    for(unsigned int i = 1; i < numDprocs-1; ++i){
      MPI_Send(&bufferSize, 1, MPI_UNSIGNED_LONG, i, BUFFER_SIZE_TAG, MPI_COMM_WORLD);
	    	    
    }
	
    //Setup my buffer size
    myBufferSize = bufferSize;

    //The last process will have the leftover
    bufferSize += (inputFileSize % NUMPROCS);

    if(NUMPROCS != 1){
      MPI_Send(&bufferSize, 1, MPI_UNSIGNED_LONG, numDprocs-1, BUFFER_SIZE_TAG, MPI_COMM_WORLD);
    }
  }

  //If I am not the master proc, I should expect to get
  //the buffer size information
  else{
	
    //Wait for the buffer size information
    MPI_Recv(&recvBuffSize, 1, MPI_UNSIGNED_LONG, 0, BUFFER_SIZE_TAG, MPI_COMM_WORLD, &status);

    //Setup the receive buffer
    recvBuffer = malloc(sizeof(unsigned char) * (recvBuffSize));

    //Setup my buffer info
    myBufferSize = recvBuffSize;
    myBuffer = recvBuffer;
  }

  MPI_Barrier(MPI_COMM_WORLD);
    

  //Now that everyone has their buffers setup, lets send info around!
  if(MYRANK == MASTER_RANK){
    fileBuffer = malloc(sizeof(unsigned char) * bufferSize);
	
    if(!fileBuffer){
      fprintf(stderr, "Error allocating read buffer, not enough memory!\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    unsigned int numChars = bufferSize - (inputFileSize % NUMPROCS);
    fread(fileBuffer, numChars, 1, inputFile);
	
    //For each process (except the last), send it its work
    for(int i = 1; i < numDprocs-1; ++i){
	    
      fread(fileBuffer, numChars, 1, inputFile);
	    
      MPI_Send(fileBuffer, numChars, MPI_UNSIGNED_CHAR, i, SEND_BUFFER_TAG, MPI_COMM_WORLD);
    }

    if(NUMPROCS != 1){
      //Send the last process its work, it gets a little bit more
      fread(fileBuffer, bufferSize, 1, inputFile);
      MPI_Send(fileBuffer, bufferSize, MPI_UNSIGNED_CHAR, numDprocs-1, SEND_BUFFER_TAG, MPI_COMM_WORLD);
    }

    //Set master back to the beginning to fill masters buffer
    fseek(inputFile, 0, SEEK_SET);
    fread(fileBuffer, myBufferSize, 1, inputFile);

    //Setup my buffer info
    myBuffer = fileBuffer;

  }
  else{

    //Wait for my buffer to get filled
    MPI_Recv(recvBuffer, recvBuffSize, MPI_UNSIGNED_CHAR, 0, SEND_BUFFER_TAG, MPI_COMM_WORLD, &status);
  }

  //Calculate my portion of the work
  buffIter myIter;
  writeBuff metaWriter;
  writeBuff dataWriter;
  unsigned long int lastPos = 0;
  unsigned long int unusedBits = 0;
    
  uint64_t next = 0;
  uint64_t last = 0;
  uint64_t count = 1;

  myDataFile = fopen(dataFileName, "wb");
  myMetaFile = fopen(metaFileName, "wb");
    
  initDataFile(myDataFile, keySize);
  initWriteBuff(&dataWriter, myDataFile, keySize);
  initBuffIter(&myIter, myBuffer, myBufferSize, keySize);
  u64array_init(&counts);
    
  advance(&myIter, &next);	
  last = next;
    
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
  //This should actually be some wonky number
  unusedBits = unusedBuffBits(&myIter);

  closeWriteBuff(&dataWriter);
    

  //Now write to the meta file
  unsigned int numBits = 64;
    
  for(unsigned int i = 0; i < 64; ++i){
	
    if((counts.biggest << i) & 0x8000000000000000){
      numBits = 64 - i;
      break;
    }
  }
        
  //Write the num of bits to the meta file
  initMetaFile(myMetaFile, numBits, counts.n, numDprocs);
    
  //Now write the array elements to the meta file at the given bit level
  //Create chars of the elements.
    
  //For each array element convert it to the numBits format,
  //string them together to then write them as chars.
  initWriteBuff(&metaWriter, myMetaFile, numBits);
    
  for(unsigned long int i = 0; i < counts.n; ++i){
	
    pushToWriteBuff(&metaWriter, counts.data[i] << (64-numBits));
	
  }
    
  closeWriteBuff(&metaWriter);


  MPI_Barrier(MPI_COMM_WORLD);
  
  if (MYRANK == MASTER_RANK) {
    struct timeval elapsedTime;
    gettimeofday(&tvEnd, 0);
    subtractTime(&tvStart, &tvEnd, &elapsedTime);
    printf("Elapsed time: %ld.%ld06\n", elapsedTime.tv_sec,
	   elapsedTime.tv_usec);
  }
  
  //Free up any allocations we made
  if(MYRANK == MASTER_RANK){

    fclose(inputFile);       
    free(fileBuffer);
  }

  else{	

    free(recvBuffer);
  }

  fclose(myDataFile);
  fclose(myMetaFile);
  free(counts.data);
  free(dataFileName);
  free(metaFileName);	

  
  MPI_Finalize();
  return 0;
}
//printf("tvStart: %ld.%06ld\n", tvStart.tv_sec, tvStart.tv_usec);
