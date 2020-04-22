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

    // We are creating a function to replace MPI_Bcast (as is the one we used before to send n
    // to all the processes). So, we will use the following parameters for the function, taking into
    // account the ones that conform MPI_Bcast:

    // int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
    // Broadcasts a message from the process with rank root to all other processes of the group.
    int MPI_BinomialColective(void *buf, int count, MPI_Datatype dt, int root, MPI_Comm comm){
		int i, org, dest;
		MPI_Status st;
		MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
		MPI_Comm_rank(MPI_COMM_WORLD, &procID);
		
        // number of steps we'll need, if we have 4 procs -> 2 steps, 8 procs -> 3 steps.
        // and we ceil to catch every number in between of those exact integers
		for(i = 1; i <= ceil(log2(numprocs)); i++){ 
			// In step “i” the processes with myrank < 2^i−1 communicate with the process myrank + 2^i−1
			if(procID < pow(2,i-1)){
				dest = procID + pow(2,i-1);
				if (dest < numprocs)
					MPI_Send(buf, count, dt, dest, 0, comm);
			} else { 
				// otherwise we want the proc to receive the parameter from buf, and after that,
				// this process will help our root process, transfer the buf value (in the upper part)
				if(procID < pow(2,i)){  
					org = procID - pow(2,i-1);
					MPI_Recv(buf, count, dt, org, 0, comm, &st);
				}
			}
			
		} 

		return MPI_SUCCESS;
	}

    while (!done) {

        t_ini = clock();

        // Only let the process #0 access this part of the code
        if (procID == 0){
            printf("Enter the maximum number to check for primes: (0 quits) \n");
            scanf("%d",&n);
        }

        MPI_BinomialColective(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        t_fin = clock();

        secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;

        if (procID==0) printf("Reading and sending n took: %.16g milliseconds\n", secs * 1000.0);
        
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

