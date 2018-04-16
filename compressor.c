
#include <stdio.h>

int main(int argc, char * argv[]){

    char * toCompFileName;
    unsigned int frameSize, panelSize;
    
    if(argc != 4){
	printf("Invalid number of input arguments. Got %d, expected 4.\n", argc);
	return 1;
    }
    
    toCompFileName = argv[1];
    sscanf(argv[2], "%d", &frameSize);
    sscanf(argv[3], "%d", &panelSize);    
    
    printf("Starting Program...\n");
    printf("%s\n", toCompFileName);
    printf("frameSize = %d \t panelSize = %d\n", frameSize, panelSize);

    return 0;
}
