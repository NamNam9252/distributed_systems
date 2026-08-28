#include <stdio.h>
#include<mpi.h>
#include<string.h>
#define MAX_SIZE 100


void doProcessing(int rank , int size){
    int source ;
    int dest ;
    int tag =0 ;
    char message[MAX_SIZE] ;
    int count ; 
    MPI_Status status ;
    if(rank!=0){
        sprintf(message,"Hello from RANK %d",rank);
        dest = 0 ;
        MPI_Send(message,strlen(message)+1,MPI_CHAR,dest,tag,MPI_COMM_WORLD);
    }else{
        count = 0;
        while(count < size-1){
            if(MPI_Recv(message,MAX_SIZE,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status)==MPI_SUCCESS){
                printf("Message from RANK %d : %s\n",source,message);
                count++;
            }else{
                printf("Error in receiving message from RANK %d\n",source);
            }
            
        }
    }
}



int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Hello from process %d of %d\n", rank, size);
    doProcessing(rank,size);
    MPI_Finalize();
    return 0;
}