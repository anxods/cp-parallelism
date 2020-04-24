// Alessandro Aldrey Urresti - 46290561W
// Anxo Díaz Sande - 48116657J

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h> 

int main(int argc, char *argv[]){

    clock_t t_ini, t_fin;
    double secs;

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

    // We are creating a function to replace MPI_Reduce (as is the one we used before to collect all
    // the results from the processes, sum them and send the final result to the root proc) named 
    // MPI_FlattreeCollective, which will have the same behaviour as MPI_Reduce.
    // So, we will use the following parameters for the function, taking into account the ones
    // that conform MPI_Reduce:

    // int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
    //        MPI_Datatype datatype, MPI_Op op, int root,
    //        MPI_Comm comm)
    // 
    // Reduces values on all processes within a group.
    //
    // As it is specified in the assignment that the MPI_Op is MPI_SUM we omit that parameter

    int MPI_FlattreeCollective(const void *sendbuf, void *recvbuf, int count, MPI_Datatype dt, int root, MPI_Comm comm){
        int i, procID, numprocs;
        MPI_Status st;
        MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
        MPI_Comm_rank(MPI_COMM_WORLD, &procID);
        
        // If the proc entering is not the root, we make it send the value that it got to the root
        if(procID != root){
            MPI_Send(sendbuf, count, dt, root, 0, comm); 
        } else { // Otherwise, we proceed to sum the values collected into a final result
            // we get the first address of the pointer to the sending buffer
            int totalCount = ((int*) sendbuf)[0]; 
            
            // as in the p1, we sum all the resulting numbers from "i" process
            for (i = 0; i < numprocs; i++){ 
                // as we are sending this values to the root proc, we omit this one (as it already
                // has its own localCount)
                if (i != root){
                    MPI_Recv(recvbuf, count, dt, i, 0, comm, &st);
                    totalCount += ((int*)recvbuf)[0];
                }
            }
		
            // finally we send the obtained value to the first address position of the receiving buffer
            ((int*)recvbuf)[0] = totalCount;
        }             

        return MPI_SUCCESS;
    }

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

            t_ini = clock();

            MPI_FlattreeCollective(&count, &global_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

            t_fin = clock();

            secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;

            if (procID == 0){
                printf("Reading and sending the number of primes lower than n (count) and sending it to global_count took: %.16g milliseconds\n", secs * 1000.0);
            }

            if (procID == 0){
                printf("The number of primes lower than %d is %d (process %d)\n", n, global_count, procID);
                printf("\n");
            }

        } else {
		    printf("Error: invalid input number (negative)\n");
        }
    }

    MPI_Finalize();
}

