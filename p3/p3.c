// Alessandro Aldrey Urresti - 46290561W
// Anxo Díaz Sande - 48116657J

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 1

/* 
	Translation of the DNA bases
	A -> 0
	C -> 1
	G -> 2
	T -> 3
	N -> 4
*/

#define M  10 // Number of sequences
#define N  10 // Number of bases per sequence

#define ROOT 0

// The distance between two bases
int base_distance(int base1, int base2){

	if((base1 == 4) || (base2 == 4)){
		return 3;
	}

	if(base1 == base2) {
		return 0;
	}

	if((base1 == 0) && (base2 == 3)) {
		return 1;
	}

	if((base2 == 0) && (base1 == 3)) {
		return 1;
	}

	if((base1 == 1) && (base2 == 2)) {
		return 1;
	}

	if((base2 == 2) && (base1 == 1)) {
		return 1;
	}

	return 2;
}

int main(int argc, char *argv[] ) {

	int i, j;
	int *data1, *data2;
	int *result;
	struct timeval  tv1, tv2, tv1_1, tv2_1, tvi, tvf;

	int numprocs, procID;
	int *microsecondsCompTotal, *result_total, *microsecondsCommTotal;

    MPI_Init(&argc, &argv); 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);

	data1 = (int *) malloc(M*N*sizeof(int));
	data2 = (int *) malloc(M*N*sizeof(int));
	result = (int *) malloc(M*sizeof(int));
	microsecondsCompTotal = malloc(numprocs *sizeof(int));
	result_total = (int *) malloc(M*sizeof(int));
	microsecondsCommTotal = malloc(numprocs *sizeof(int));

	if (procID == 0){
		/* Initialize Matrices */
		for(i=0;i<M;i++) { // enter the sequence i
			for(j=0;j<N;j++) { // once we are inside the sequence i we check for the base j
				data1[i*N+j] = (i+j)%5;
				data2[i*N+j] = ((i-j)*(i-j))%5;
			}
		}
	}

	int procs[numprocs];
	int rows = 0;
	int assignedRows = 0;

	if (procID == ROOT){
		// We need to compute the number of rows per process
		if ((M % numprocs) == 0){ // if M is a multiple of the numprocs
			
			for (i=0; i<numprocs; i++){
				procs[i] = M / numprocs; // we assign the number of
				assignedRows += procs[i]; 
			}

			/* Just to check if it was working properly
			if (procID==0) {
				printf("- Assigned rows: \n");
				
				for (i=0; i<numprocs; i++)
					printf(" Process %d: %d \n",i,procs[i]); 

				printf(" Total: %d", assignedRows); 
				printf("\n\n");
			}
			*/

		} else { // case it is not a multiple
			
			for (i=0; i<numprocs-1; i++){
				procs[i] = M / numprocs + 1; // (as if we were using a floor, but getting an integer)
				assignedRows += procs[i]; 
			}

			procs[numprocs-1] = M - assignedRows; // we assign the rest to the last process
			
			/* Just to check if it was working properly
			if (procID==0) {
				printf("- Assigned rows: \n");
				for (i=0; i<numprocs; i++)
					printf(" Process %d: %d \n",i,procs[i]); 

				printf(" Total: %d", assignedRows + procs[numprocs-1]); 
				printf("\n\n");
			}
			*/
		}
	}

	gettimeofday(&tvi, NULL);

	// Once the root process got the array with the number of rows assigned to each process (including itself)
	// we send this to all the processes, with MPI_Scatter.
	MPI_Scatter(&procs, 1, MPI_INT, &rows, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	gettimeofday(&tvf, NULL);

	int microsecondsScatter = (tvf.tv_usec - tvi.tv_usec)+ 1000000 * (tvf.tv_sec - tvi.tv_sec);

	// Once we have assigned the rows to each process, we proceed to compute the actual 
	// base_distance between the sequences (also measuring the time):

	gettimeofday(&tv1, NULL);

	for(i = 0; i < rows; i++) { // process entering the loop deals with its own assigned rows
		result[i]=0;
		for(j = 0; j < N; j++) {
			result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
		}
	}

	gettimeofday(&tv2, NULL);

	int microseconds = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

	MPI_Gather(&microseconds, 1, MPI_INT, microsecondsCompTotal, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	// Here we end measuring the time it took to compute the base_distance, assigning to it the 
	// value "microseconds"

	gettimeofday(&tv1_1, NULL);
	
	MPI_Gather(result, rows, MPI_INT, result_total, rows, MPI_INT, ROOT, MPI_COMM_WORLD);	

	gettimeofday(&tv2_1, NULL);

	int microsecondsGather = (tv2_1.tv_usec - tv1_1.tv_usec)+ 1000000 * (tv2_1.tv_sec - tv1_1.tv_sec);

	int microsecondsComm = microsecondsGather + microsecondsScatter;

	MPI_Gather(&microsecondsComm, 1, MPI_INT, microsecondsCommTotal, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	if (procID == 0){
		/*Display result */
		if (!DEBUG){
			for(i=0;i<M;i++) {
				printf(" %d \t ", result_total[i]);
			}
			printf("\n");
		} else {
			// Last task is to show the microseconds it took each process to:
			// 1st: to compute the actual distances (microseconds)
			// 2nd: the communication between processes, that is, the collective operations to send the values
			// 		()
			if (procID == 0){
				printf("Times of both the computation and communication of processes used in execution:\n\n");
				// Computation time
				int microseconds_comp_total = 0;
				for(i = 0; i < numprocs; i++){
					printf("Computation time for process %i was %lf microseconds\n", i, (double) microsecondsCompTotal[i]/1E6);
					microseconds_comp_total += microsecondsCompTotal[i];
				}

				printf("Total computation time was: %lf microseconds\n\n", (double) microseconds_comp_total/1E6);

				// Communication time
				int microseconds_comm_total = 0;
				for(i = 0; i < numprocs; i++){
					printf("Communication time for process %i was %lf microseconds\n", i, (double) microsecondsCommTotal[i]/1E6);
					microseconds_comm_total += microsecondsCommTotal[i];
				}

				printf("Total communication time was: %lf microseconds\n", (double) microseconds_comm_total/1E6);
			}
		}    
	}

	// One free per malloc to every value used
	if (procID == 0){
		free(data1); 
		free(data2); 
		free(result); 
		free(microsecondsCompTotal); 
		free(result_total);
		free(microsecondsCommTotal);
	}

	MPI_Finalize();

	return 0;
}
