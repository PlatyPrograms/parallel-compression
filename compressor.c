
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]){

    char *inputFileName, *dataFileName, *metaFileName;
    unsigned int frameSize, panelSize;
    FILE *inputFile, *dataFile, *metaFile;
    size_t inputNameLength, cutoff;
    
    if(argc != 4){
	printf("Invalid number of input arguments. Got %d, expected 3.\n", (argc-1));
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
    
    sscanf(argv[2], "%d", &frameSize);
    sscanf(argv[3], "%d", &panelSize);    

    //Print some updates for the user
    printf("Producing files named: %s and %s\n", dataFileName, metaFileName);
    printf("frameSize = %d bits \t panelSize = %d bits\n", frameSize, panelSize);

    //Now let's create the files we will be filling
    if((inputFile = fopen(inputFileName, "r")) == NULL){
	printf("Input file \"%s\" can not be opened...\n", inputFileName);
	return -1;
    }
    
    dataFile  = fopen(dataFileName,  "w");
    metaFile  = fopen(metaFileName,  "w");
    
    //Let's try to read the input file
    
    

    //Close the files after we use them
    fclose(inputFile);
    fclose(dataFile);
    fclose(metaFile);

    //Free up any allocations we made
    free(dataFileName);
    free(metaFileName);
    
    return 0;
}
