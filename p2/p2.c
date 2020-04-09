#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]){

    int i, j, prime, done = 0, n, count, global_count;

    int numprocs, procID;

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
        }

        // int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
        // Broadcasts a message from the process with rank root to all other processes of the group.
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (n == 0) break;

        // valid input to check primes
        if (n > 0){

            // computation number of primes
            count = 0;

            // as all the processes must participate (including the #0), and the nº procs is numprocs
            // we set i to procID+2, which will be 2, 3, 4 and 5 if there are 4 processes
            // in the case we wanted to check for 12 (n=12):
            // proc #0 will cover 2,6,10
            // proc #1 will cover 3,7,11
            // proc #2 will cover 4,8
            // proc #3 will cover 5,9
            
            for (i = procID+2; i < n; i+=numprocs) {
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

            // Reduces values on all processes within a group.
            // int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
            //        MPI_Datatype datatype, MPI_Op op, int root,
            //        MPI_Comm comm)
            MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

            if (procID == 0){
                printf("The number of primes lower than %d is %d (process %d)\n", n, global_count, procID);
            }

        } else {
            printf("Error: invalid input number (negative)\n");
        }

    }

    MPI_Finalize();
}
