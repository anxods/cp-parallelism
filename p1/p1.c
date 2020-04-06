#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]){

    int i, j, prime, done = 0, n, count;

    int numprocs, procID, procs;

    MPI_Status status;

    // Initialize the MPI with MPI_Init
    // MPI_Init - Initializes the MPI execution environment 
    MPI_Init(&argc, &argv); 

    // This   function   indicates  the  number  of  processes  involved  in  a  communicator.
    // MPI_Comm_size  - Returns the size of the group associated with a communicator.
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    // MPI_Comm_rank - Determines the rank of the calling process in the communicator.
    // This  function  gives  the rank of the process in the particular communicator's group.
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);

    while (!done) {

        // Only let the process #0 access this part of the code
        if (procID == 0){
            printf("Enter the maximum number to check for primes: (0 quits) \n");
            scanf("%d",&n);
            printf("Proc %d", procID);

            for (procs = 1; procs<numprocs; procs++)
                // int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
                // MPI_Send - Performs a standard-mode blocking send.
                MPI_Send(&n, 1, MPI_INT, procs, 0, MPI_COMM_WORLD);
        } else {
            // int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
            // MPI_Recv - Performs a standard-mode blocking receive.
            MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }
        
        if (n == 0) break;

        if (n > 0){

        }

        // computation number of primes
        count = 0;

        for (i = 2; i < n; i++) {
            prime = 1;

            // Check if any number lower than i is multiple
            for (j = 2; j < i; j++) {
                if((i%j) == 0) {
                prime = 0;
                break;
                }
            }
            count += prime;
        }

        printf("The number of primes lower than %d is %d\n", n, count);
    }

    MPI_Finalize();
}
