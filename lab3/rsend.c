#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BIG_LOOP 3000000000LL


void timestamp(double start, int rank, const char *message)
{
    printf("[T = %7.3f s] Rank %d: %s\n",
           MPI_Wtime() - start,
           rank,
           message);

    fflush(stdout);
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4)
    {
        if (rank == 0)
            printf("Run with exactly 4 processes.\n");

        MPI_Finalize();
        return 0;
    }

    double start = MPI_Wtime();

    int data = rank;


    /* =========================================================
       TEST 1
       MPI_Send WITHOUT MPI_Recv
       ========================================================= */

    if (rank == 0)
    {
        timestamp(start, rank,
                  "TEST 1: Calling MPI_Send to Rank 1.");

        timestamp(start, rank,
                  "Rank 1 will NOT call MPI_Recv.");

        timestamp(start, rank,
                  ">>> MPI_Send START <<<");

        MPI_Send(&data, 1, MPI_INT, 1, 100,
                 MPI_COMM_WORLD);

        timestamp(start, rank,
                  ">>> MPI_Send COMPLETED <<<");

        timestamp(start, rank,
                  "Rank 0 continues normally.");
    }


    else if (rank == 1)
    {
        timestamp(start, rank,
                  "TEST 1: I will NOT call MPI_Recv.");

        timestamp(start, rank,
                  "I am doing other work.");

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            /* work */
        }

        timestamp(start, rank,
                  "Work finished.");

        timestamp(start, rank,
                  "Notice: I never called MPI_Recv.");
    }


    /* =========================================================
       TEST 2
       MPI_Rsend WITHOUT MPI_Recv
       ========================================================= */

    else if (rank == 2)
    {
        timestamp(start, rank,
                  "TEST 2: Calling MPI_Rsend to Rank 3.");

        timestamp(start, rank,
                  "Rank 3 will NOT call MPI_Recv.");

        timestamp(start, rank,
                  "MPI_Rsend REQUIRES the receive to already be posted.");

        timestamp(start, rank,
                  ">>> MPI_Rsend START <<<");

        /*
         * INTENTIONALLY INVALID MPI PROGRAM
         *
         * Rank 3 has not posted MPI_Recv().
         *
         * MPI_Rsend may:
         *
         *   - return immediately
         *   - block
         *   - produce an MPI error
         *   - behave differently depending on implementation
         *
         * There is NO portable guarantee.
         */

        MPI_Rsend(&data, 1, MPI_INT, 3, 200,
                  MPI_COMM_WORLD);

        timestamp(start, rank,
                  ">>> MPI_Rsend RETURNED <<<");

        timestamp(start, rank,
                  "WARNING: This does NOT mean the operation was valid.");

        timestamp(start, rank,
                  "Rank 3 never posted MPI_Recv().");
    }


    else if (rank == 3)
    {
        timestamp(start, rank,
                  "TEST 2: I will NOT call MPI_Recv.");

        timestamp(start, rank,
                  "Rank 2 is using MPI_Rsend.");

        timestamp(start, rank,
                  "NO matching receive has been posted.");

        timestamp(start, rank,
                  "Doing other work instead.");

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            /* work */
        }

        timestamp(start, rank,
                  "Work finished.");

        timestamp(start, rank,
                  "MPI_Recv was NEVER posted.");

        timestamp(start, rank,
                  "Therefore Rank 2's MPI_Rsend violated its requirement.");
    }


    timestamp(start, rank,
              "Reached end of program.");

    MPI_Finalize();

    return 0;
}