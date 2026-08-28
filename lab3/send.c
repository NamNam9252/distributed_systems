#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void delay_loop(int steps)
{
    volatile int value = 0;
    for (int i = 0; i < steps; i++) {
        value += i;
    }
}

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand((unsigned)time(NULL) + rank);

    char message[128];
    int tag = 5;

    if (rank == 0) {
        for (int source = 1; source < size; source++) {
            delay_loop(20000 + rand() % 50000);
            MPI_Status status;
            MPI_Recv(message, sizeof(message), MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
            printf("Rank 0 received: %s\n", message);
        }
    } else {
        delay_loop(40000 + rand() % 100000);
        sprintf(message, "Hello from rank %d using Send", rank);
        MPI_Send(message, (int)strlen(message) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}