#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MESSAGES 5

void random_work()
{
    volatile long x = 0;
    int loops = 1000000 + rand() % 5000000;

    for (int i = 0; i < loops; i++)
        x += i;
}

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

/* Send a message to a destination process */
void send_message(int *clock, int size, int destination, int rank)
{
    
    clock[rank]++;
    MPI_Send(clock, size, MPI_INT,
             destination, 0, MPI_COMM_WORLD);

    printf("P%d -> P%d | Sent | Vector Clock = ",
           rank, destination);

    print_vector_clock(clock, size);
    printf("\n");
}

void receive_messages(int *clock, int size, int rank)
{
    int flag;
    int *received_clock;
    MPI_Status status;

    received_clock = (int *)malloc(size * sizeof(int));

    do
    {
        MPI_Iprobe(MPI_ANY_SOURCE, 0,
                   MPI_COMM_WORLD, &flag, &status);

        if (flag)
        {
           
            MPI_Recv(received_clock, size, MPI_INT,
                     status.MPI_SOURCE, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("P%d <- P%d | Received Vector Clock = ",
                   rank, status.MPI_SOURCE);

            print_vector_clock(received_clock, size);
            printf(" | Local Before = ");

            print_vector_clock(clock, size);

            printf("\n");
            for (int i = 0; i < size; i++)
            {
                if (received_clock[i] > clock[i])
                    clock[i] = received_clock[i];
            }
            clock[rank]++;
            printf("P%d | Updated Vector Clock = ",
                   rank);

            print_vector_clock(clock, size);

            printf("\n");
        }

    } while (flag);

    free(received_clock);
}


int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int *clock = (int *)calloc(size, sizeof(int));

    if (clock == NULL)
    {
        printf("Memory allocation failed\n");
        MPI_Finalize();
        return 1;
    }
    srand(time(NULL) + rank);

    for (int i = 0; i < MESSAGES; i++)
    {
        receive_messages(clock, size, rank);
        random_work();
        int destination = rand() % size;
            while (destination == rank)
            destination = rand() % size;
        send_message(clock, size, destination, rank);
    }
    
    for (int i = 0; i < 10; i++)
    {
        random_work();

        receive_messages(clock, size, rank);
    }

    printf("P%d finished | Final Vector Clock = ",
           rank);

    print_vector_clock(clock, size);
    printf("\n");

    free(clock);

    MPI_Finalize();

    return 0;
}