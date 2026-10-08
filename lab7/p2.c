#include <stdio.h>
#include <mpi.h>
int main(int argc, char **argv) {
    int rank,size;

    int local[100][100] = {0};
    int result[100][100] = {0};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank <3 and rank >=0) {

        int len , a[100][100], b[100][100]; 
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&a, len * len, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&b, len * len, MPI_INT, 0, MPI_COMM_WORLD);

        for (int j = 0; j < 3; j++) {
            local[rank][j] = A[rank][j] + B[rank][j];
        }

    }
    MPI_Allreduce(local, result, 9, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0) {
        int a[100][100];
        int b[100][100];
        int len ;
        printf("Enter the length of the matrix: ");
        scanf("%d", &len);
        printf("Enter the first matrix: \n");
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                scanf("%d", &a[i][j]);
            }
        }
        printf("Enter the second matrix: \n");
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                scanf("%d", &b[i][j]);
            }
        }
        int c[100][100] = {0};

        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&a, len * len, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&b, len * len, MPI_INT, 0, MPI_COMM_WORLD);



        printf("The added matrix is- \n");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                printf("%d ", result[i][j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}