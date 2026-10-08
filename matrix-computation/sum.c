#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    int M, N;
    int i, j;

    int *A = NULL;
    int *B = NULL;
    int *C = NULL;
    int *local_C = NULL;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Rank 0 takes the dimensions */
    if (rank == 0)
    {
        printf("Enter number of rows (M): ");
        scanf("%d", &M);

        printf("Enter number of columns (N): ");
        scanf("%d", &N);
    }

    /* Send M and N to all processes */
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Allocate memory on every process */
    A = (int *)malloc(M * N * sizeof(int));
    B = (int *)malloc(M * N * sizeof(int));

    /*
     * local_C has the same size as the complete matrix.
     * Only the assigned rows will contain useful values.
     */
    local_C = (int *)calloc(M * N, sizeof(int));

    /* Rank 0 allocates the final matrix */
    if (rank == 0)
    {
        C = (int *)malloc(M * N * sizeof(int));
    }

    /* Rank 0 reads matrix A */
    if (rank == 0)
    {
        printf("\nEnter Matrix A:\n");

        for (i = 0; i < M; i++)
        {
            for (j = 0; j < N; j++)
            {
                scanf("%d", &A[i * N + j]);
            }
        }

        /* Rank 0 reads matrix B */
        printf("\nEnter Matrix B:\n");

        for (i = 0; i < M; i++)
        {
            for (j = 0; j < N; j++)
            {
                scanf("%d", &B[i * N + j]);
            }
        }
    }

    /* Send both matrices to every process */
    MPI_Bcast(A, M * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, M * N, MPI_INT, 0, MPI_COMM_WORLD);

    /*
     * Divide rows among processes.
     *
     * Example:
     * M = 100
     * size = 10
     *
     * rows_per_process = 10
     *
     * Process 0 -> rows 0-9
     * Process 1 -> rows 10-19
     * Process 2 -> rows 20-29
     * ...
     * Process 9 -> rows 90-99
     */

    int rows_per_process = M / size;

    int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process;

    /*
     * If M is not perfectly divisible by number of processes,
     * the last process handles the remaining rows.
     */
    if (rank == size - 1)
    {
        end_row = M;
    }

    /* Each process computes only its assigned rows */
    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < N; j++)
        {
            local_C[i * N + j] =
                A[i * N + j] + B[i * N + j];
        }
    }

    /*
     * Combine all partial results.
     *
     * Since each process has values only for its assigned rows,
     * MPI_SUM combines them into the final matrix at rank 0.
     */
    MPI_Reduce(
        local_C,
        C,
        M * N,
        MPI_INT,
        MPI_SUM,
        0,
    MPI_COMM_WORLD
    );

    /* Rank 0 prints the final matrix */
    if (rank == 0)
    {
        printf("\nMatrix C = A + B:\n");

        for (i = 0; i < M; i++)
        {
            for (j = 0; j < N; j++)
            {
                printf("%d ", C[i * N + j]);
            }
            printf("\n");
        }
    }

    /* Free memory */
    free(A);
    free(B);
    free(local_C);

    if (rank == 0)
    {
        free(C);
    }

    MPI_Finalize();

    return 0;
}
