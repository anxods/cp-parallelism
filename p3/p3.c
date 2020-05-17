// Alessandro Aldrey Urresti - 46290561W
// Anxo Díaz Sande - 48116657J

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <mpi.h>

#define DEBUG 0

/* 
	Translation of the DNA bases
	A -> 0
	C -> 1
	G -> 2
	T -> 3
	N -> 4
*/

#define M  1000 // Number of sequences
#define N  200000 // Number of bases per sequence

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
	int *data1_aux, *data2_aux, *data1, *data2;
	int *result;
	struct timeval tv1, tv2, tv1_1, tv2_1, tvi, tvf;

	int numprocs, procID;
	int *microsecondsCompTotal, *result_total, *microsecondsCommTotal;

    MPI_Init(&argc, &argv); 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);

	int rows = ceil((float) M / numprocs);
	int rowsProcess;

	data1_aux = (int *) malloc(numprocs * rows *N*sizeof(int));
	data1 = (int *) malloc(rows *N*sizeof(int));
	data2_aux = (int *) malloc(numprocs * rows *N*sizeof(int));
	data2 = (int *) malloc(rows *N*sizeof(int));
	result = (int *) malloc(M*sizeof(int));
	microsecondsCompTotal = malloc(numprocs *sizeof(int));
	result_total = (int *) malloc(M*sizeof(int));
	microsecondsCommTotal = malloc(numprocs *sizeof(int));

	if (procID == 0){
		/* Initialize Matrices */
		for(i=0;i<M;i++) { // enter the sequence i
			for(j=0;j<N;j++) { // once we are inside the sequence i we check for the base j
				data1_aux[i*N+j] = (i+j)%5;
				data2_aux[i*N+j] = ((i-j)*(i-j))%5;
			}
		}
	}

	gettimeofday(&tvi, NULL);

	// To perform the base_distance computation, we firstly assign (rows)*N elements to each process
	// from both data1_aux and data2_aux. Afterwards, we will perform the proper domain decomposition,
	// deleting some possible "garbage" elements if necessary.

	MPI_Scatter(data1_aux,rows*N,MPI_INT,data1,rows*N,MPI_INT,ROOT,MPI_COMM_WORLD);
	MPI_Scatter(data2_aux,rows*N,MPI_INT,data2,rows*N,MPI_INT,ROOT,MPI_COMM_WORLD);

	gettimeofday(&tvf, NULL);

	int microsecondsScatter = (tvf.tv_usec - tvi.tv_usec)+ 1000000 * (tvf.tv_sec - tvi.tv_sec);

	// Here's where we assign the corresponding rows to each process (domain decomposition).
	// As we said before, this part of the code is useful to eliminate some possible "garbage" data assigned
	// before. For example, if M is not a multiple of numprocs, we will be assigning some extra data
	// to the last process; with the following part, we ensure that, at least, this last process does not
	// deal with any "garbage" data, and so the program works as expected without bug numbers at the output.

	if (procID == numprocs-1)
		rowsProcess =  M - rows * (numprocs - 1);
	else
		rowsProcess = rows;


	// Once we have assigned the rowsProcess to each process, we proceed to compute the actual 
	// base_distance between the sequences (also measuring the time):

	gettimeofday(&tv1, NULL);

	// Process entering the loop deals with its own assigned rowsProcess (by means of data1 and data2)
	for(i = 0; i < rowsProcess; i++) { 
		result[i]=0;
		for(j = 0; j < N; j++) {
			result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
		}
	}

	gettimeofday(&tv2, NULL);

	int microsecondsComp = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

	MPI_Gather(&microsecondsComp, 1, MPI_INT, microsecondsCompTotal, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	// Here we end measuring the time it took to compute the base_distance, assigning to it the 
	// value "microseconds"

	gettimeofday(&tv1_1, NULL);
	
	MPI_Gather(result, rowsProcess, MPI_INT, result_total, rowsProcess, MPI_INT, ROOT, MPI_COMM_WORLD);	

	gettimeofday(&tv2_1, NULL);

	int microsecondsGather = (tv2_1.tv_usec - tv1_1.tv_usec)+ 1000000 * (tv2_1.tv_sec - tv1_1.tv_sec);

	int microsecondsComm = microsecondsGather + microsecondsScatter;

	MPI_Gather(&microsecondsComm, 1, MPI_INT, microsecondsCommTotal, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	if (procID == 0){
		/* Display result */
		if (DEBUG){
			
			for(i=0;i<M;i++) {
				printf(" %d \t ", result_total[i]);
			}
			printf("\n");

		} else {

			if (procID == 0){
				printf("Times of both the computation and communication of processes (%d) used in execution of the base_distance program:\n\n", numprocs);
				
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

	// One free per malloc to every value used (performed by the process #0)
	if (procID == 0){
		free(data1); 
		free(data2);
		free(data1_aux);
		free(data2_aux); 
		free(result); 
		free(microsecondsCompTotal); 
		free(result_total);
		free(microsecondsCommTotal);
	}

	MPI_Finalize();

	return 0;
}
