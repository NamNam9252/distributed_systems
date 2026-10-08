#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ROUNDS 3

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

    int *clock = calloc(size, sizeof(int));

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

    /*
     * ==========================================
     * TOKEN RING
     * ==========================================
     *
     * P0 starts every round.
     *
     * P0 -> P1 -> P2 -> ... -> P0
     */

    for (int round = 1; round <= ROUNDS; round++)
    {
        /*
         * --------------------------------------
         * P0 starts the round
         * --------------------------------------
         */
        if (rank == 0)
        {
            /*
             * Sending is an event.
             */
            clock[rank]++;

            printf("\n========================================\n");
            printf("ROUND %d\n", round);
            printf("========================================\n");

            printf("P0 -> P1 | Sent = ");
            print_vector_clock(clock, size);
            printf("\n");

            MPI_Send(clock, size, MPI_INT,
                     next, 0, MPI_COMM_WORLD);

            /*
             * P0 now waits for the token to
             * complete the ring.
             */
            MPI_Recv(received, size, MPI_INT,
                     previous, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int before[size];

            for (int i = 0; i < size; i++)
                before[i] = clock[i];

            /*
             * Merge received clock.
             */
            for (int i = 0; i < size; i++)
            {
                if (received[i] > clock[i])
                    clock[i] = received[i];
            }

            /*
             * Receive event.
             */
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
            /*
             * ----------------------------------
             * Other processes receive token
             * ----------------------------------
             */

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

            /*
             * Merge clocks.
             */
            for (int i = 0; i < size; i++)
            {
                if (received[i] > clock[i])
                    clock[i] = received[i];
            }

            /*
             * Receive event.
             */
            clock[rank]++;

            printf(" | Updated = ");

            print_vector_clock(clock, size);

            printf("\n");

            /*
             * Sending is another event.
             */
            clock[rank]++;

            printf("P%d -> P%d | Sent = ",
                   rank, next);

            print_vector_clock(clock, size);

            printf("\n");

            /*
             * Forward token.
             */
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