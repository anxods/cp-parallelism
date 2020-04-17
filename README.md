# cp-parallelism

Parallelism practical exercises - UDC

## How to compile and run MPI C files

### Compile
```bash
mpicc -o p1 -Wall p1.c
```
##### Note

To compile the p2v3.c (pow, log and ceil functions included):
```bash
mpicc -o p2 -Wall p2v3.c -lm
```

### Run 
```bash
mpirun -np 4 p1
```
