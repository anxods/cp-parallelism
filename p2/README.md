# cp-parallelism

## 2nd exercise: Calculation of the number of primes lower than a certain value. Use of MPI Collective Communication

We must implement three versions of the program:
- [x] With MPI_Bcast and MPI_Reduce operations.
- [ ] Creating MPI_FlattreeColective operation (which basically is the for loop of the p1 but adding the option of selecting which process we want the root to be).
- [x] Creating MPI_BinomialColective operation (same as before but with a binomial tree behaviour).

#### Notes

To compile the p2v3.c (which includes the MPI_BinomialColective function), we must put -lm flag, as it includes mathematical operations such as log, pow...

### Given code
```c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]){
  int i, j, prime, done = 0, n, count;
    while (!done){
      printf("Enter the maximum number to check for primes: (0 quits) \n");
      scanf("%d",&n);
      if(n==0) break;
      count=0;
      for(i=2; i<n; i++){
        prime=1;
        // Check if any number lower than i is multiple
        for(j=2; j<i; j++){
          if((i%j)==0){
            prime=0;
            break;
          }
        }
        count += prime;
      }
    printf("The number of primes lower than %d is %d\n", n, count);
    }
}
```
