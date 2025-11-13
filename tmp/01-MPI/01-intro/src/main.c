#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "ipadress.h"

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    char ip[INET_ADDRSTRLEN];
    get_ip_address(ip, sizeof(ip));

    printf("Hello from rank %d/%d on processor %s (%s)\n",
        world_rank, world_size, processor_name, ip);
    
    char hostname2[256];
    if (gethostname(hostname2, sizeof(hostname2)) == 0) {
        printf("Hostname from gethostname: %s\n", hostname2);
    } else {
        perror("gethostname");
    }
    // Finalize the MPI environment.
    MPI_Finalize();
}