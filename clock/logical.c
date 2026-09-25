#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Helper function to find the maximum of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

int main(int argc, char** argv) {
    int rank, size;
    
    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // We need at least 2 processes to send/receive messages
    if (size < 2) {
        if (rank == 0) {
            printf("Please run with at least 2 processes (e.g., mpirun -np 2 ./clock)\n");
        }
        MPI_Finalize();
        return 0;
    }

    // Seed the random number generator uniquely for each process
    srand(time(NULL) + rank);

    int local_clock = 0;
    int num_events = 5; // Number of messages each process will send
    MPI_Request requests[num_events];

    for (int i = 0; i < num_events; i++) {
        // 1. Add a random delay to simulate asynchronous behavior (0 to 1000 milliseconds)
        int delay = rand() % 1000;
        usleep(delay * 1000); 

        // 2. Before sending, check if there are any incoming messages waiting
        int has_message;
        MPI_Status status;
        
        // MPI_Iprobe checks for messages without blocking
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
        
        while (has_message) {
            int received_clock;
            // Receive the pending message
            MPI_Recv(&received_clock, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Lamport Clock Receive Rule: max(local, received) + 1
            local_clock = max(local_clock, received_clock) + 1;
            
            printf("[Process %d] RECEIVED msg from Process %d | Logical Clock updated to: %d\n", 
                   rank, status.MPI_SOURCE, local_clock);
                   
            // Check again if more messages arrived while processing
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
        }

        // 3. Prepare to send a message
        // Pick a random target process (make sure it's not itself)
        int target = rand() % size;
        while (target == rank) {
            target = rand() % size;
        }

        // Lamport Clock Send Rule: increment local clock before sending
        local_clock++;
        
        printf("[Process %d] SENDING msg to Process %d   | Logical Clock: %d (Delay was %d ms)\n", 
               rank, target, local_clock, delay);

        // Send the message asynchronously 
        MPI_Isend(&local_clock, 1, MPI_INT, target, 0, MPI_COMM_WORLD, &requests[i]);
    }

    // 4. Drain remaining messages (graceful termination)
    // Wait a brief moment to allow in-flight messages to arrive
    usleep(1500 * 1000); 
    
    int has_message;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
    while (has_message) {
        int received_clock;
        MPI_Recv(&received_clock, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_clock = max(local_clock, received_clock) + 1;
        printf("[Process %d] LATE RECEIVE from Process %d | Logical Clock updated to: %d\n", 
               rank, status.MPI_SOURCE, local_clock);
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
    }

    // Ensure all our asynchronous sends actually completed before shutting down
    MPI_Waitall(num_events, requests, MPI_STATUSES_IGNORE);

    // Sync all processes before finalizing
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}