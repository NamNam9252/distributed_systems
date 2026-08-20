#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int rank, size;
	int send_value;
	int recv_value;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int parity = rank % 2;
	int next = rank + 2;
	int prev = rank - 2;

	if (next >= size) {
		next = parity;
	}

	if (prev < 0) {
		prev = size - 1 - ((size - 1) % 2 == parity ? 0 : 1);
	}

	send_value = rank;
	if (next == rank && prev == rank) {
		recv_value = send_value;
	} else if (rank == parity) {
		MPI_Send(&send_value, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
		MPI_Recv(&recv_value, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
	} else {
		MPI_Recv(&recv_value, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
		MPI_Send(&send_value, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
	}

	printf("MY-Rank %d received %d from rank %d and I-sent to rank %d\n",
		   rank, recv_value, prev, next);

	MPI_Finalize();
	return 0;
}
