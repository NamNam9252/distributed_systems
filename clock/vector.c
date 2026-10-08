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

/* Print vector clock */
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
    /*
     * Increment own entry before sending.
     */
    clock[rank]++;

    /*
     * Send the complete vector clock.
     */
    MPI_Send(clock, size, MPI_INT,
             destination, 0, MPI_COMM_WORLD);

    printf("P%d -> P%d | Sent | Vector Clock = ",
           rank, destination);

    print_vector_clock(clock, size);
    printf("\n");
}

/* Check and receive all waiting messages */
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
            /*
             * Receive the vector clock sent by the other process.
             */
            MPI_Recv(received_clock, size, MPI_INT,
                     status.MPI_SOURCE, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            /*
             * Print exactly what was received.
             */
            printf("P%d <- P%d | Received Vector Clock = ",
                   rank, status.MPI_SOURCE);

            print_vector_clock(received_clock, size);

            /*
             * Show our clock before updating.
             */
            printf(" | Local Before = ");

            print_vector_clock(clock, size);

            printf("\n");

            /*
             * Vector clock merge:
             *
             * clock[i] = max(clock[i], received_clock[i])
             */
            for (int i = 0; i < size; i++)
            {
                if (received_clock[i] > clock[i])
                    clock[i] = received_clock[i];
            }

            /*
             * Receiving a message is also an event,
             * so increment our own clock.
             */
            clock[rank]++;

            /*
             * Print the final updated clock.
             */
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

    /*
     * Vector clock.
     *
     * For N processes:
     *
     * P0 -> [0,0,0,...]
     * P1 -> [0,0,0,...]
     * ...
     */
    int *clock = (int *)calloc(size, sizeof(int));

    if (clock == NULL)
    {
        printf("Memory allocation failed\n");
        MPI_Finalize();
        return 1;
    }

    /* Different random seed for every process */
    srand(time(NULL) + rank);

    for (int i = 0; i < MESSAGES; i++)
    {
        /*
         * Check messages that have already arrived.
         */
        receive_messages(clock, size, rank);

        /*
         * Simulate some computation.
         */
        random_work();

        /*
         * Choose a random destination.
         */
        int destination = rand() % size;

        /*
         * Don't send to ourselves.
         */
        while (destination == rank)
            destination = rand() % size;

        /*
         * Send vector clock to destination.
         */
        send_message(clock, size, destination, rank);
    }

    /*
     * Give processes time to receive
     * messages that are still in transit.
     */
    for (int i = 0; i < 10; i++)
    {
        random_work();

        receive_messages(clock, size, rank);
    }

    /*
     * Print final vector clock.
     */
    printf("P%d finished | Final Vector Clock = ",
           rank);

    print_vector_clock(clock, size);
    printf("\n");

    free(clock);

    MPI_Finalize();

    return 0;
}