#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2)
    {
        if (rank == 0)
            printf("Run this program with 2 processes.\n");

        MPI_Finalize();
        return 0;
    }

    int data = rank + 1;

    if (rank == 0)
    {
        printf("Rank 0: Sending data using MPI_Bsend...\n");
        double start = MPI_Wtime();

        MPI_Bsend(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        double end = MPI_Wtime();
        printf("Rank 0: MPI_Bsend finished in %.3f seconds\n", end - start);
    }
    else
    {
        int received_data;
        int count;
        int source ;
        printf("Rank 1: Waiting to receive data using MPI_Probe...\n");
        MPI_Probe(source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Get_count(MPI_STATUS_IGNORE, MPI_INT, &count);
        printf("Rank 1: Received data count: %d\n", count);
    }

    MPI_Finalize();
    return 0;
}