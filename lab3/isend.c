#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000000
#define BIG_LOOP 2000000000LL


/* =========================================================
   Print a timestamped message
   ========================================================= */

void print_time(double start, int rank, const char *message)
{
    double now = MPI_Wtime();

    printf("[T = %8.3f s] Rank %d: %s\n",
           now - start, rank, message);

    fflush(stdout);
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* We need exactly 2 processes */
    if (size != 2)
    {
        if (rank == 0)
            printf("Run this program with 2 processes.\n");

        MPI_Finalize();
        return 0;
    }


    /* =========================================================
       CREATE DATA
       ========================================================= */

    int *data = malloc(SIZE * sizeof(int));

    if (data == NULL)
    {
        printf("Memory allocation failed.\n");
        MPI_Finalize();
        return 1;
    }

    for (int i = 0; i < SIZE; i++)
    {
        data[i] = i;
    }


    /* =========================================================
       TEST 1: MPI_Send
       ========================================================= */

    double test_start = MPI_Wtime();

    if (rank == 0)
    {
        printf("\n\n============================================\n");
        printf("              TEST 1: MPI_Send\n");
        printf("============================================\n");

        print_time(test_start, rank,
                   "Calling MPI_Send...");

        MPI_Send(data, SIZE, MPI_INT, 1, 0,
                 MPI_COMM_WORLD);

        print_time(test_start, rank,
                   "MPI_Send returned.");
    }
    else
    {
        print_time(test_start, rank,
                   "Starting BIG LOOP before MPI_Recv...");

        long long answer = 0;

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            answer += i;
        }

        print_time(test_start, rank,
                   "BIG LOOP finished.");

        print_time(test_start, rank,
                   "Calling MPI_Recv...");

        MPI_Recv(data, SIZE, MPI_INT, 0, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        print_time(test_start, rank,
                   "Data received.");
    }


    /* =========================================================
       TEST 2: MPI_Bsend
       ========================================================= */

    int buffer_size =
        SIZE * sizeof(int) + MPI_BSEND_OVERHEAD;

    char *buffer = malloc(buffer_size);

    if (buffer == NULL)
    {
        printf("Buffer allocation failed.\n");
        free(data);
        MPI_Finalize();
        return 1;
    }

    test_start = MPI_Wtime();

    if (rank == 0)
    {
        printf("\n\n============================================\n");
        printf("              TEST 2: MPI_Bsend\n");
        printf("============================================\n");

        MPI_Buffer_attach(buffer, buffer_size);

        print_time(test_start, rank,
                   "Calling MPI_Bsend...");

        MPI_Bsend(data, SIZE, MPI_INT, 1, 1,
                  MPI_COMM_WORLD);

        print_time(test_start, rank,
                   "MPI_Bsend returned.");

        print_time(test_start, rank,
                   "Data has been copied to MPI buffer.");

        /*
         * Detach the buffer.
         */
        void *temp_buffer;
        int temp_size;

        MPI_Buffer_detach(&temp_buffer, &temp_size);

        free(temp_buffer);
    }
    else
    {
        print_time(test_start, rank,
                   "Starting BIG LOOP before MPI_Recv...");

        long long answer = 0;

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            answer += i;
        }

        print_time(test_start, rank,
                   "BIG LOOP finished.");

        print_time(test_start, rank,
                   "Calling MPI_Recv...");

        MPI_Recv(data, SIZE, MPI_INT, 0, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        print_time(test_start, rank,
                   "Buffered data received.");

        free(buffer);
    }


    /* =========================================================
       TEST 3: MPI_Isend
       ========================================================= */

    test_start = MPI_Wtime();

    if (rank == 0)
    {
        printf("\n\n============================================\n");
        printf("              TEST 3: MPI_Isend\n");
        printf("============================================\n");

        MPI_Request request;

        /*
         * -----------------------------------------------------
         * STEP 1: Start the non-blocking send
         * -----------------------------------------------------
         */

        print_time(test_start, rank,
                   "Calling MPI_Isend...");

        MPI_Isend(data, SIZE, MPI_INT, 1, 2,
                  MPI_COMM_WORLD, &request);

        print_time(test_start, rank,
                   "MPI_Isend returned!");

        print_time(test_start, rank,
                   "IMPORTANT: Send may STILL be in progress.");

        /*
         * -----------------------------------------------------
         * STEP 2: Do other work
         * -----------------------------------------------------
         */

        print_time(test_start, rank,
                   "Doing OTHER WORK while send is in progress...");

        long long answer = 0;

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            answer += i;
        }

        print_time(test_start, rank,
                   "Other work finished.");

        /*
         * -----------------------------------------------------
         * STEP 3: Wait for send completion
         * -----------------------------------------------------
         */

        print_time(test_start, rank,
                   "Calling MPI_Wait...");

        MPI_Wait(&request, MPI_STATUS_IGNORE);

        print_time(test_start, rank,
                   "MPI_Wait returned.");

        print_time(test_start, rank,
                   "MPI_Isend is now COMPLETE.");

        print_time(test_start, rank,
                   "Send buffer can now safely be reused.");
    }
    else
    {
        /*
         * -----------------------------------------------------
         * Receiver deliberately waits before receiving
         * -----------------------------------------------------
         */

        print_time(test_start, rank,
                   "Starting BIG LOOP before MPI_Recv...");

        long long answer = 0;

        for (long long i = 0; i < BIG_LOOP; i++)
        {
            answer += i;
        }

        print_time(test_start, rank,
                   "BIG LOOP finished.");

        /*
         * -----------------------------------------------------
         * Now receive the data
         * -----------------------------------------------------
         */

        print_time(test_start, rank,
                   "Calling MPI_Recv...");

        MPI_Recv(data, SIZE, MPI_INT, 0, 2,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        print_time(test_start, rank,
                   "MPI_Recv completed.");

        print_time(test_start, rank,
                   "Data from MPI_Isend has been received.");
    }


    /* =========================================================
       CLEANUP
       ========================================================= */

    free(data);

    MPI_Finalize();

    return 0;
}
