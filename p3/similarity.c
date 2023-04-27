#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <mpi.h>

#define DEBUG 1

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  1006 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

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

  int i, j, timeF = 0;
  int process_Rank, numprocs;
  int *data1, *data2;
  int *result;
  struct timeval  tv1, tv2;

  data1 = (int *) malloc(M*N*sizeof(int));
  data2 = (int *) malloc(M*N*sizeof(int));
  result = (int *) malloc(M*sizeof(int));

  for(i=0;i<M;i++) {
    result[i] = 0;
  }

  /* Initialize Matrices */
  for(i=0;i<M;i++) {
    for(j=0;j<N;j++) {
      /* random with 20% gap proportion */
      data1[i*N+j] = fast_rand();
      data2[i*N+j] = fast_rand();
    }
  }

  MPI_Init(&argc, &argv);
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  
  int *aux1 = (int *) malloc(floor((M)/numprocs)*N*sizeof(int));
  int *aux2 = (int *) malloc(floor((M)/numprocs)*N*sizeof(int));
  int *aux3 = (int *) malloc(floor((M)/numprocs)*sizeof(int));

  MPI_Scatter(data1, floor((M)/numprocs)*N, MPI_INT,aux1,floor((M)/numprocs)*N, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Scatter(data2, floor((M)/numprocs)*N, MPI_INT,aux2,floor((M)/numprocs)*N, MPI_INT, 0, MPI_COMM_WORLD);
  

  gettimeofday(&tv1, NULL);

  if(process_Rank == 0){
    if((M % numprocs)!=0){
        for(i=(M-1); i>=(M-(M % numprocs)); i--){
          result[i]=0;
          for(j=0;j<N;j++){
            result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
          }
        }
      }
  }

  for(i=0;i<ceil((M)/numprocs);i++) {
    aux3[i]=0;
    for(j=0;j<N;j++) {
    aux3[i] += base_distance(aux1[i*N+j], aux2[i*N+j]);
    }
  }

  gettimeofday(&tv2, NULL);
    
  int microseconds = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
  
  MPI_Reduce(&microseconds, &timeF, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Gather(aux3, floor((M)/numprocs), MPI_INT, result, floor((M)/numprocs), MPI_INT, 0, MPI_COMM_WORLD);

  
  
  /* Display result */
  if (process_Rank == 0){
    timeF = timeF/N;

    if (DEBUG == 1) {
      int checksum = 0;
      for(i=0;i<M;i++) {
        checksum += result[i];
      }
      printf("Checksum: %d\n ", checksum);
    } else if (DEBUG == 2) {
      for(i=0;i<M;i++) {
        printf(" %d \t ",result[i]);
      }
    } else {
      printf ("Time (seconds) = %lf\n", (double) timeF/1E6);
    }

    free(aux1); free(aux2); free(aux3);
    free(data1); free(data2); free(result);
  }

  MPI_Finalize();

  return 0;
}
