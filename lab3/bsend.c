#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000000
#define BIG_LOOP 2000000000

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

    if (rank == 0)
    {
        printf("\n===== TEST 1: MPI_Send =====\n");
        printf("Rank 0: Sending data using MPI_Send...\n");

        double start = MPI_Wtime();

        MPI_Send(data, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);

        double end = MPI_Wtime();

        printf("Rank 0: MPI_Send finished in %.3f seconds\n",
               end - start);
    }
    else
    {
        printf("Rank 1: Doing a big loop before receiving...\n");

        long long answer = 0;

        for (long long i = 0; i < BIG_LOOP; i++)
            {answer += i;}

        printf("Rank 1: Big loop finished. Now receiving data.\n");
        MPI_Recv(data, SIZE, MPI_INT, 0, 0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 1: Data received.\n");}

    if (rank == 0)
        printf("\n===== TEST 2: MPI_Bsend =====\n");
    int buffer_size = SIZE * sizeof(int) + MPI_BSEND_OVERHEAD;
    char *buffer = malloc(buffer_size);

    if (buffer == NULL)
    {
        printf("Buffer allocation failed.\n");
        free(data);
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        MPI_Buffer_attach(buffer, buffer_size);
        printf("Rank 0: Sending data using MPI_Bsend...\n");
        double start = MPI_Wtime();
        MPI_Bsend(data, SIZE, MPI_INT, 1, 1,MPI_COMM_WORLD);
        double end = MPI_Wtime();
        printf("Rank 0: MPI_Bsend finished in %.3f seconds\n",end - start);
        printf("Rank 0: The data was copied to the MPI buffer.\n");
        void *temp_buffer;
        int temp_size;
        MPI_Buffer_detach(&temp_buffer, &temp_size);
        printf("Rank 0: The buffer work done %f.\n", MPI_Wtime() - start);
        free(temp_buffer);
    }
    else
    {
        printf("Rank 1: Doing a big loop before receiving...\n");
        long long answer = 0;
        for (long long i = 0; i < BIG_LOOP; i++)
        {
            answer += i;
        }
        printf("Rank 1: Big loop finished. Now receiving data.\n");
        MPI_Recv(data, SIZE, MPI_INT, 0, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Rank 1: Buffered data received.\n");
        free(buffer);
    }

    free(data);
    MPI_Finalize();

    return 0;
}
