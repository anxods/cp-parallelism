# cp-parallelism

## 3rd exercise: Similarity vector for DNA datasets. Use of domain decomposition 

## Datasets of DNA sequences
  - M text sequences
  - Each one with N bases (characters A, C, G, T, N):

    1. A: adenine
    2. C: cytosine
    3. G: guanine
    4. T: thymine
    5. N: nucleotide
    
  More info: https://en.wikipedia.org/wiki/Nucleic_acid_sequence

## Similarity vector
  - One vector for two datasets of DNA sequences
  - Metric that indicates how similar two sequences are, namely the i − th sequence of each dataset
    - M results: both datasets must have the same number of sequences (M)
    - Simplification: all sequences have the same number of bases (N)

## Domain decomposition
  - Divide the matrices among p processes, each one with rows = M/p rows (for simplicity, start with the scenario where the number of processes is multiple of M: M mod p = 0).
  - Each task will be in charge of calculating M/p rows of the resut vector.
  - Later, modify the code to consider the general case, not only multiples of M.

## Parallel approach

- [ ] SPMD implementation

- [x] I/O (scanf/printf) is made by process 0

- [x] Process 0 performs the initialization of the matrices

- [ ] Data is distributed to all the processes using collective operations

- [ ] Gather the result using collective operations

- [ ] Print separately the communication time and the computation time of each process

### Given code
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define DEBUG 1

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/


#define M  1000 // Number of sequences
#define N  200000  // Number of bases per sequence

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
  struct timeval  tv1, tv2;

  data1 = (int *) malloc(M*N*sizeof(int));
  data2 = (int *) malloc(M*N*sizeof(int));
  result = (int *) malloc(M*sizeof(int));

  /* Initialize Matrices */
  for(i=0;i<M;i++) {
    for(j=0;j<N;j++) {
      data1[i*N+j] = (i+j)%5;
      data2[i*N+j] = ((i-j)*(i-j))%5;
    }
  }

  gettimeofday(&tv1, NULL);

  for(i=0;i<M;i++) {
    result[i]=0;
    for(j=0;j<N;j++) {
      result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
    }
  }

  gettimeofday(&tv2, NULL);
    
  int microseconds = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

  /*Display result */
  if (DEBUG){
    for(i=0;i<M;i++) {
      printf(" %d \t ",result[i]);
    }
  } else {
    printf ("Time (seconds) = %lf\n", (double) microseconds/1E6);
  }    

  free(data1); free(data2); free(result);

  return 0;
}

```
