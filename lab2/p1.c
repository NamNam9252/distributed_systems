#include <mpi.h>
#include <stdio.h>



int main(int argc, char** argv) {
    int rank, size;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int left, right;
    int topology[7] = {0, 2, 4, 6, 1, 3, 5};
    int position;

    for (position = 0; position < 7; position++) {
        if (topology[position] == rank) {
            break;
        }
    }

    right = topology[(position + 1) % 7];
    left = topology[(position + 6) % 7];

    int send_value,recv_value;
    send_value = 99 ;

    if (rank == 0) {
        MPI_Send(&send_value,1,MPI_INT,right,0,MPI_COMM_WORLD);
        MPI_Recv(&recv_value,1,MPI_INT, left,0,MPI_COMM_WORLD,&status);
        printf("MY-Rank %d received%d from rank %d and I-sent to rank %d\n", rank, recv_value, left, right);
    } else {
        MPI_Recv(&recv_value,1,MPI_INT,left,0,MPI_COMM_WORLD,&status);
        printf("MY-Rank %d received%d from rank %d and I-sent to rank %d\n", rank, recv_value, left, right);
        MPI_Send(&send_value,1,MPI_INT,right,0,MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
