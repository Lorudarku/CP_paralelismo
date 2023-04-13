#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


int MPI_FlattreeColectiva(void *buff, void *rcvBuff, int count, MPI_Datatype type, MPI_Op op, int root, MPI_Comm WORLD){
    int process_Rank, numprocs;
    MPI_Comm_rank(WORLD, &process_Rank);
    MPI_Comm_size(WORLD, &numprocs);
    int aux=0;
    if (process_Rank == root) {
      aux = *(int *)buff;
      for(int x = (numprocs-1); x>0; x--){
        MPI_Recv(buff, count, type,MPI_ANY_SOURCE, 12, WORLD, NULL);
        aux +=*(int *)buff;
    }
  } else{
      MPI_Send(buff,count, type, root, 12, WORLD); 
    } 
    *(int *) rcvBuff = aux;
  return 0;
}

int MPI_BinomialColectiva(void *buff,int count,MPI_Datatype type,int process, MPI_Comm WORLD){
  int target, numprocs;
  
  MPI_Comm_size(WORLD, &numprocs);

    for(int i = 1; i < ceil(log2(numprocs)); i++){
      printf("manda %d", process);
      if (process < (pow(2,i-1))){ 
          if ((process + pow(2,i-1)) < numprocs){
          target = (int)(process + pow(2,i-1));
          printf("a %d\n", target);
          MPI_Send(buff, count, type, target, 10, WORLD);
          printf("ENVIADO\n");
          }
        }
      else{
        printf("recive %d", process);
         MPI_Recv(buff,count,type,MPI_ANY_SOURCE, 10, WORLD, NULL); 
      }
    }
  return 0;
}

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

  //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  //MPI_Bcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_BinomialColectiva(&n, 1, MPI_INT, process_Rank, MPI_COMM_WORLD);

  for(i=process_Rank; i<n; i+= numprocs){
    if(cadena[i] == L){
      aux++;
    }
  }

  MPI_Reduce(&aux, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_FlattreeColectiva(&aux, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if(process_Rank == 0)  printf("El numero de apariciones de la letra %c es %d\n", L, count);
  


  MPI_Finalize();
  free(cadena);
  exit(0);
}
