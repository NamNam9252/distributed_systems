#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char** argv) {
    int rank, size;
    int global_sum = 0;
    int matrix[2][2][2];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int base[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    matrix[i][j][k] = base[i][j][k];
                }
            }
        }
    }

    // Broadcast the same matrix to all ranks.
    MPI_Bcast(matrix, 8, MPI_INT, 0, MPI_COMM_WORLD);

    // The matrix has only 2 rows, so use a safe row index for every rank.
    int row = rank % 2;
    int local_sum = 0;

    for (int col = 0; col < 2; col++) {
        local_sum += matrix[row][col][0] + matrix[row][col][1];
    }

    printf("Process %d: using row %d, local sum = %d\n", rank, row, local_sum);

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total sum = %d\n", global_sum);
        printf("Number of processes = %d\n", size);
    }

    MPI_Finalize();
    return 0;
}