
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


#define ROOT_RANK 0

int main(int argc, char** argv){

    int NUMTHREADS, MYTHREAD; 
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &NUMTHREADS );
    MPI_Comm_rank( MPI_COMM_WORLD, &MYTHREAD );


    printf("[%d] I am ready to work!\n", MYTHREAD);

    if(MYTHREAD == ROOT_RANK){
	printf("[%d]\tI am root!\n", MYTHREAD);
    }

    MPI_Finalize();

    return 0;
}
