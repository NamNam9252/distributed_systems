#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ROUNDS 1

void print_vector_clock(int *clock, int size)
{
    printf("[");

    for (int i = 0; i < size; i++)
    {
        printf("%d", clock[i]);

        if (i < size - 1)
            printf(", ");
    }

    printf("]");
}

int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2)
    {
        if (rank == 0)
            printf("Run with at least 2 processes.\n");

        MPI_Finalize();
        return 1;
    }

    int *clock = (int*)calloc(size, sizeof(int));

    if (clock == NULL)
    {
        printf("Memory allocation failed.\n");
        MPI_Finalize();
        return 1;
    }

    int next = (rank + 1) % size;
    int previous = (rank - 1 + size) % size;

    int received[size];

    MPI_Barrier(MPI_COMM_WORLD);
    for (int round = 1; round <= ROUNDS; round++)
    {
        if (rank == 0)
        {
            clock[rank]++;

            printf("\n========================================\n");
            printf("ROUND %d\n", round);
            printf("========================================\n");

            printf("P0 -> P1 | Sent = ");
            print_vector_clock(clock, size);
            printf("\n");

            MPI_Send(clock, size, MPI_INT,
                     next, 0, MPI_COMM_WORLD);

            MPI_Recv(received, size, MPI_INT,
                     previous, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int before[size];

            for (int i = 0; i < size; i++)
                before[i] = clock[i];

            for (int i = 0; i < size; i++)
            {
                if (received[i] > clock[i])
                    clock[i] = received[i];
            }

            clock[rank]++;

            printf("P0 <- P%d | Received = ",
                   previous);

            print_vector_clock(received, size);

            printf(" | Updated = ");

            print_vector_clock(clock, size);

            printf("\n");
        }
        else
        {
            MPI_Recv(received, size, MPI_INT,
                     previous, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int before[size];

            for (int i = 0; i < size; i++)
                before[i] = clock[i];

            printf("P%d <- P%d | Received = ",
                   rank, previous);

            print_vector_clock(received, size);

            printf(" | Local Before = ");

            print_vector_clock(before, size);
            for (int i = 0; i < size; i++)
            {
                if (received[i] > clock[i])
                    clock[i] = received[i];
            }

 
            clock[rank]++;

            printf(" | Updated = ");

            print_vector_clock(clock, size);

            printf("\n");

 
            clock[rank]++;

            printf("P%d -> P%d | Sent = ",
                   rank, next);

            print_vector_clock(clock, size);

            printf("\n");

            MPI_Send(clock, size, MPI_INT,
                     next, 0, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    printf("P%d finished | Final Vector Clock = ",
           rank);

    print_vector_clock(clock, size);

    printf("\n");

    free(clock);

    MPI_Finalize();

    return 0;
}