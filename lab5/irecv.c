#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int data;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            fprintf(stderr,
                    "This program must be run with exactly 2 processes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Process %d starts\n", rank);

        long long x = 0;

        for (int i = 0; i < 10000; i++)
            for (int j = 0; j < 1000000; j++)
                x += i - j;

        data = 123;

        MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        printf("Process %d sent data: %d\n", rank, data);

    } else if (rank == 1) {
        printf("Process %d starts\n", rank);

        MPI_Request request;

        printf("Process %d is about to initiate a non-blocking receive \n", rank);

        MPI_Irecv(&data, 1, MPI_INT, 0, 0,
                  MPI_COMM_WORLD, &request);

        printf("Process %d controls return from the recv operation \n", rank);

        // Wait until the non-blocking receive completes
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        printf("Process %d received data: %d\n", rank, data);
    }

    printf("Process %d ends\n", rank);

    MPI_Finalize();
    return 0;
}
