#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void inicializaCadena(char *cadena, int n){
  int i;
  for(i=0; i<n/2; i++){
    cadena[i] = 'A';
  }
  for(i=n/2; i<3*n/4; i++){
    cadena[i] = 'C';
  }
  for(i=3*n/4; i<9*n/10; i++){
    cadena[i] = 'G';
  }
  for(i=9*n/10; i<n; i++){
    cadena[i] = 'T';
  }
}

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n");
    printf("es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
    exit(1); 
  }
    
  int i, n, count=0;
  char *cadena;
  char L;

  int process_Rank, numprocs;

  n = atoi(argv[1]);
  L = *argv[2];

  cadena = (char *) malloc(n*sizeof(char));
  inicializaCadena(cadena, n);


  MPI_Init(&argc, &argv);
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  int *contador = (int *) malloc(numprocs*sizeof(int));
  int aux = 0;


  if (process_Rank == 0) {
    for(int x = (numprocs-1); x>0; x--){
      MPI_Send(&n, 1, MPI_INT, x, 10, MPI_COMM_WORLD);
      MPI_Send(&L, 1, MPI_CHAR, x, 11, MPI_COMM_WORLD);
    }
  } else{
      MPI_Recv(&n,1,MPI_INT,0, 10, MPI_COMM_WORLD, &status); 
      MPI_Recv(&L,1,MPI_CHAR,0, 11, MPI_COMM_WORLD, &status);
    }


  for(i=process_Rank; i<n; i+= numprocs){
    if(cadena[i] == L){
      aux++;
    }
  }

  if (process_Rank == 0) {
    count = aux;
    for(int x = (numprocs-1); x>0; x--){
      MPI_Recv(&aux, 1, MPI_INT,MPI_ANY_SOURCE, 12, MPI_COMM_WORLD, &status);
      count +=aux;
    }
  } else{
      MPI_Send(&aux,1,MPI_INT,0, 12, MPI_COMM_WORLD); 
    } 
  if(process_Rank == 0){
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
  }


  MPI_Finalize();
  free(cadena);
  exit(0);
}
