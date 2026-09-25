#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Helper to find the maximum of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Helper to format the vector clock as a string (e.g., "[1, 0, 2]")
void format_vector(int* vector, int size, char* buffer) {
    int offset = sprintf(buffer, "[");
    for (int i = 0; i < size; i++) {
        offset += sprintf(buffer + offset, "%d%s", vector[i], (i == size - 1) ? "" : ", ");
    }
    sprintf(buffer + offset, "]");
}

int main(int argc, char** argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Size determines the length of our vector

    if (size < 2) {
        if (rank == 0) {
            printf("Please run with at least 2 processes (e.g., mpirun -np 3 ./vector_clock)\n");
        }
        MPI_Finalize();
        return 0;
    }

    srand(time(NULL) + rank);

    // 1. Initialize the Vector Clock (array of size N, filled with 0s)
    int* vector_clock = (int*)calloc(size, sizeof(int));
    
    // Buffer to hold an incoming vector clock from another process
    int* incoming_vector = (int*)malloc(size * sizeof(int));

    int num_events = 4;
    MPI_Request requests[num_events];
    
    // 2. Allocate separate memory buffers for each asynchronous send. 
    // If we just sent `vector_clock`, we might modify it locally before MPI finishes sending it over the network!
    int** send_buffers = (int**)malloc(num_events * sizeof(int*));
    for (int i = 0; i < num_events; i++) {
        send_buffers[i] = (int*)malloc(size * sizeof(int));
    }

    char clock_str[256]; 

    for (int i = 0; i < num_events; i++) {
        // Random delay to simulate asynchronous behavior
        int delay = rand() % 1000;
        usleep(delay * 1000); 

        // 3. Check for incoming messages non-blockingly
        int has_message;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
        
        while (has_message) {
            // Receive the vector clock array from the sender
            MPI_Recv(incoming_vector, size, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Vector Clock Receive Rule: 
            // Step A: Take element-wise maximum of local and received vectors
            for (int k = 0; k < size; k++) {
                vector_clock[k] = max(vector_clock[k], incoming_vector[k]);
            }
            // Step B: Increment own clock
            vector_clock[rank]++;
            
            format_vector(vector_clock, size, clock_str);
            printf("[Process %d] RECEIVED msg from Process %d | Vector Clock: %s\n", 
                   rank, status.MPI_SOURCE, clock_str);
                   
            // Probe again in case multiple messages arrived while we were processing
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
        }

        // 4. Prepare to send
        int target = rand() % size;
        while (target == rank) {
            target = rand() % size;
        }

        // Vector Clock Send Rule: Increment own clock before sending
        vector_clock[rank]++;
        
        // Snapshot the current state of the vector clock into the dedicated send buffer
        for (int k = 0; k < size; k++) {
            send_buffers[i][k] = vector_clock[k];
        }

        format_vector(vector_clock, size, clock_str);
        printf("[Process %d] SENDING msg to Process %d   | Vector Clock: %s (Delay: %d ms)\n", 
               rank, target, clock_str, delay);

        // Send the snapshot array asynchronously
        MPI_Isend(send_buffers[i], size, MPI_INT, target, 0, MPI_COMM_WORLD, &requests[i]);
    }

    // 5. Drain remaining in-flight messages (graceful termination)
    usleep(1500 * 1000); 
    
    int has_message;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
    while (has_message) {
        MPI_Recv(incoming_vector, size, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int k = 0; k < size; k++) {
            vector_clock[k] = max(vector_clock[k], incoming_vector[k]);
        }
        vector_clock[rank]++;
        format_vector(vector_clock, size, clock_str);
        printf("[Process %d] LATE RECEIVE from Process %d | Vector Clock: %s\n", 
               rank, status.MPI_SOURCE, clock_str);
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);
    }

    // Wait for all non-blocking sends to complete before freeing memory
    MPI_Waitall(num_events, requests, MPI_STATUSES_IGNORE);

    // Cleanup memory
    free(vector_clock);
    free(incoming_vector);
    for (int i = 0; i < num_events; i++) {
        free(send_buffers[i]);
    }
    free(send_buffers);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}