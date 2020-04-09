#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]){

    int i, j, prime, done = 0, n, count;

    int numprocs, procID, aux;

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

            for (i = 1; i<numprocs; i++)
                // int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
                // MPI_Send - Performs a standard-mode blocking send.
                MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        } else {
            // int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
            // MPI_Recv - Performs a standard-mode blocking receive.
            MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }
        
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

            if (procID!=0){

                MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            
            } else { // output must be performed by process #0

                for (i = 1; i < numprocs; i++){
                    MPI_Recv(&aux, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                                        count += aux;
                }

                printf("The number of primes lower than %d is %d (process %d)\n", n, count, procID);
                }

        } else {
            printf("Error: invalid input number (negative)\n");
        }

    }

    MPI_Finalize();
}
