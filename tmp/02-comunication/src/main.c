# include <mpi.h>
# include <stdio.h>
# include <string.h>
# define STD_TAG 0

int main(int argc, char **argv) {

    int i, my_rank, n_procs;
    char msg[100];

    // Status variable to get information about the received message
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    // All processes except the master (rank 0) send a message to it
    if(my_rank != 0) {
        sprintf(msg, "I’m alive");
        MPI_Send(msg, strlen(msg)+1, MPI_CHAR, 0, STD_TAG, MPI_COMM_WORLD);
    }
    else {
        for (i = 1; i < n_procs; i++) {
            // the msg must be continuously allocated
            // if it's a matrix you need to use a single array
            MPI_Recv(msg, 100, MPI_CHAR, i, MPI_ANY_TAG,
            //MPI_Recv(msg, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
            printf("Proc %d: %s\n", status.MPI_SOURCE, msg);
        }
    }
    MPI_Finalize();
    return 0;
}