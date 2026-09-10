// MPI program for two processes: rank 0 sends data and rank 1 receives it.

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define BIG 9999999999


int main(int argc, char *argv[]) {
	int rank, size;
	int data;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size != 2) {
		if (rank == 0) {
			fprintf(stderr, "This program must be run with exactly 2 processes.\n");
		}
		MPI_Finalize();
		return 1;
	}

	if (rank == 0) {
        printf("Process %d starts\n", rank);
        int x = 0;
        for (int i = 0; i < 10000; i++)
            for (int j = 0; j < 1000000; j++)
                x += i - j;
		data = 123;
		MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		printf("Process %d sent data: %d\n", rank, data);
	} else if (rank == 1) {
        printf("Process %d starts\n", rank);
        printf("Process %d is about to initiate a blocking receive \n", rank);
		MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d controls return from the recv operation \n", rank);
		printf("Process %d received data: %d\n", rank, data);
	}

	printf("Process %d ends\n", rank);

	MPI_Finalize();
	return 0;
}
