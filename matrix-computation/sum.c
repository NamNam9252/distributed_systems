#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    int M = 0, N = 0;
    int i, j;

    int *A = NULL;
    int *B = NULL;
    int *C = NULL;
    int *local_C = NULL;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        printf("Enter number of rows (M): ");
        fflush(stdout);

        if (scanf("%d", &M) != 1 || M <= 0)
        {
            fprintf(stderr, "Invalid M.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        printf("Enter number of columns (N): ");
        fflush(stdout);

        if (scanf("%d", &N) != 1 || N <= 0)
        {
            fprintf(stderr, "Invalid N.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    A = malloc(M * N * sizeof(int));
    B = malloc(M * N * sizeof(int));

    if (A == NULL || B == NULL)
    {
        fprintf(stderr,
                "Rank %d: Memory allocation failed for A/B.\n",
                rank);

        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0)
    {
        printf("\nEnter Matrix A:\n");
        fflush(stdout);

        for (i = 0; i < M; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (scanf("%d", &A[i * N + j]) != 1)
                {
                    fprintf(stderr, "Invalid input for Matrix A.\n");
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
            }
        }

        printf("\nEnter Matrix B:\n");
        fflush(stdout);

        for (i = 0; i < M; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (scanf("%d", &B[i * N + j]) != 1)
                {
                    fprintf(stderr, "Invalid input for Matrix B.\n");
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
            }
        }
    }

    MPI_Bcast(
        A,
        M * N,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    MPI_Bcast(
        B,
        M * N,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    int base_rows = M / size;
    int remainder = M % size;

    int local_rows;

    if (rank < remainder)
        local_rows = base_rows + 1;
    else
        local_rows = base_rows;

    int start_row = rank * base_rows;

    if (rank < remainder)
        start_row += rank;
    else
        start_row += remainder;

    int end_row = start_row + local_rows;


    local_C = calloc(M * N, sizeof(int));

    if (local_C == NULL)
    {
        fprintf(stderr,
                "Rank %d: Memory allocation failed for local_C.\n",
                rank);

        MPI_Abort(MPI_COMM_WORLD, 1);
    }


    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < N; j++)
        {
            local_C[i * N + j] =A[i * N + j] +B[i * N + j];
        }
    }


    if (rank == 0)
    {
        C = malloc(M * N * sizeof(int));

        if (C == NULL)
        {
            fprintf(stderr,
                    "Rank 0: Memory allocation failed for C.\n");

            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Reduce(
        local_C,             /* send buffer */
        C,                    /* receive buffer */
        M * N,                /* number of elements */
        MPI_INT,              /* data type */
        MPI_SUM,              /* reduction operation */
        0,                    /* root */
        MPI_COMM_WORLD        /* communicator */
    );


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

