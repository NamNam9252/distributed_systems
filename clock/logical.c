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

/* Send a message to a destination process */
void send_message(int *clock, int destination, int rank)
{
    (*clock)++;
    MPI_Send(clock, 1, MPI_INT,
             destination, 0, MPI_COMM_WORLD);

    printf("P%d -> P%d | Sent | Clock = %d\n",
           rank, destination, *clock);
}

/* Check and receive all waiting messages */
void receive_messages(int *clock, int rank)
{
    int flag;
    int received_clock;

    MPI_Status status;

    do
    {
        MPI_Iprobe(MPI_ANY_SOURCE, 0,
                   MPI_COMM_WORLD, &flag, &status);

        if (flag)
        {
            MPI_Recv(&received_clock, 1, MPI_INT,
                     status.MPI_SOURCE, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            /* Lamport clock update */
            if (received_clock > *clock)
                *clock = received_clock;

            (*clock)++;

            printf("P%d <- P%d | Received | Clock = %d\n",
                   rank, status.MPI_SOURCE, *clock);
        }

    } while (flag);
}

int main(int argc, char *argv[])
{
    int rank, size;
    int clock = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Different random seed for every process */
    srand(time(NULL) + rank);

    for (int i = 0; i < MESSAGES; i++)
    {
        /* Check messages that arrived */
        receive_messages(&clock, rank);

        /* Simulate some computation */
        random_work();

        /* Choose a random destination */
        int destination = rand() % size;

        /* Don't send to ourselves */
        while (destination == rank)
            destination = rand() % size;

        /* Send message */
        send_message(&clock, destination, rank);
    }

    for (int i = 0; i < 10; i++)
    {
        random_work();
        receive_messages(&clock, rank);
    }

    printf("P%d finished | Final Clock = %d\n",
           rank, clock);

    MPI_Finalize();

    return 0;
}
