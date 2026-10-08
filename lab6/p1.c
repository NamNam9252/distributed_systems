#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, size;
    int local_value, global_sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    local_value = rank + 1;
    global_sum = 0;

    MPI_Reduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total sum = %d\n", global_sum);
        printf("Number of processes = %d\n", size);
    }

    MPI_Finalize();
    return 0;
}
