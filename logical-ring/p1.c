#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, size;
    
    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("A ring requires at least 2 processes. Run with: mpirun -np 3 ./ring\n");
        }
        MPI_Finalize();
        return 0;
    }

    // 1. Form the Logical Ring (No discovery needed)
    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    printf("[Process %d] Logically connected to: Prev=%d, Next=%d\n", rank, prev, next);
    
    // Ensure all processes have printed their connections before starting the token passing
    MPI_Barrier(MPI_COMM_WORLD);

    // 2. Token Passing to verify the ring
    int token;

    if (rank == 0) {
        // Process 0 initiates the token ring
        token = 100; 
        printf("\n---> [Process 0] Initiating token ring with value: %d\n", token);
        
        // Send to successor
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        
        // Wait to receive the token back from the predecessor (the last process)
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("---> [Process 0] Token received back from Process %d. Final value: %d. Ring Complete!\n", prev, token);
    } 
    else {
        // All other processes wait for the token from their predecessor
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("     [Process %d] Received token %d from Process %d\n", rank, token, prev);
        
        // Modify the token to prove this process touched it
        token += 1; 
        
        // Pass it along to the successor
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    }

    // Clean up and exit
    MPI_Finalize();
    return 0;
}