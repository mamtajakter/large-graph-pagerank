
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
//#include <bits/stdc.h>
#define SIZE 1000000000

struct DataItem {
   long id;
   long index;
};

struct DataItem* mapArray[SIZE];
struct DataItem* dummyItem;
struct DataItem* neighborItem;
struct DataItem* item;

long mapCode(long id) {
   return id % SIZE;
}

struct DataItem *lookup(long id) {
   //get the map
   long mapIndex = mapCode(id);
   //move in array until an empty
   while(mapArray[mapIndex] != NULL) {
        if(mapArray[mapIndex]->id == id)
            return mapArray[mapIndex];
        //go to next cell
        ++mapIndex;
      //wrap around the table
        mapIndex %= SIZE;
   }
   return NULL;
}

void insert(long id,long index){//, int deg, float currentCredit, float previousCredit, int partitionID) {

   struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
   item->id = id;
   item->index=index;
   //get the map
   long mapIndex = mapCode(id);
   //move in array until an empty or deleted cell
   while(mapArray[mapIndex] != NULL && mapArray[mapIndex]->id != -1) {
      //go to next cell
      ++mapIndex;
      //wrap around the table
      mapIndex %= SIZE;
   }
   mapArray[mapIndex] = item;
}

void getCredits( long *edgeIDs, int currentRoundnumber, long totalSize, float **finalOutput, float *localCredits,float *glocalCredits, long actualSize, int world_rank) {
  // this function calculates credits of each ID in the edgeIDs array
    long i=0,j=0,k=0,m=0, neighborIndex=0,currentIDIndex=0, id;
    int neighborDeg=0,deg=0;
    float neighborCredit=0.0,currentCredit=0.0,previousCredit=0.0, partitionID=0; //neighborpartitionID=0;

    //for (i=0;i<totalSize;i++)
    for (i=totalSize;i--; )
        {//edgeIDs
         if ((i%2)==0)// checking the ID is at the left side in the edge
             k=i+1;// if it is, then calculate the credit of the ID at the right side(its neighbor )
         else//or at the right side of the edge
            k=i-1;// if it is, then calculate the credit of the ID at the left side(its neighbor )
         item=lookup(edgeIDs[i]);
         neighborItem=lookup(edgeIDs[k]);
         id=item->id;
         currentIDIndex=item->index;
         neighborIndex=neighborItem->index;
         //deg=finalOutput[1][currentIDIndex];
         currentCredit=finalOutput[3][currentIDIndex];
         previousCredit=finalOutput[4][currentIDIndex];
         partitionID=finalOutput[2][currentIDIndex];

         neighborCredit=finalOutput[4][neighborIndex];
         neighborDeg=finalOutput[1][neighborIndex];
         //neighborpartitionID=finalOutput[2][neighborIndex];

         if (world_rank==partitionID)// && (partitionID==neighborpartitionID))
              currentCredit = currentCredit+ (neighborCredit*(1.0/neighborDeg));
             //printf("world-rank: %d credit of ID %ld (neighbor:%ld) is %f\n",world_rank, id, neighborItem->id, currentCredit);
         finalOutput[3][currentIDIndex]=currentCredit;
         finalOutput[4][currentIDIndex]=previousCredit;

        }

    //MPI_Barrier(MPI_COMM_WORLD);

    //putting all calculated updated credits into the localCredits array to do Allreduce

    //for(i = 0; i<actualSize; i++) {
    for(i = actualSize; i--; ) {
        if(finalOutput[0][i]){
            localCredits[i]=finalOutput[3][i];
            }
     }

     MPI_Allreduce(localCredits, glocalCredits,actualSize+1, MPI_FLOAT, MPI_SUM,  MPI_COMM_WORLD);
     //MPI_Allreduce(finalOutput[3], finalOutput[3],actualSize+1, MPI_FLOAT, MPI_SUM,  MPI_COMM_WORLD);

    //finally storing the credits in finalOutput and reseting tempCredits to 0
    //for(i = 0; i<actualSize; i++) {
    for(i = actualSize; i--; ) {
        if(finalOutput[0][i]){
            currentCredit=glocalCredits[i];
            //currentCredit=finalOutput[3][i];
            previousCredit=finalOutput[4][i];
            previousCredit=currentCredit;
            currentCredit=0.0;
            finalOutput[3][i]=currentCredit;
            finalOutput[4][i]=previousCredit;
            if (currentRoundnumber==0){
                finalOutput[5][i]=previousCredit;
                }
            else{
               finalOutput[5+currentRoundnumber][i]=previousCredit;
                }
              }
            }
    //MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char *argv[])
{

    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc<5){
        printf("Not enough argument");
        exit(EXIT_FAILURE);
      }

    FILE * fp;
    FILE * fp2;
    int  round=atoi(argv[3]), partition=atoi(argv[4]);
    long i=0,j=0,k=0;
    char * line = NULL;

    char * line2 = NULL;
    long tempID, tempID1, tempID2;
    long totalSize=0,actualSize=0;
    size_t len = 0;
    ssize_t read;
    size_t len2 = 0;
    ssize_t read2;
    int tempDeg=0;

    dummyItem = (struct DataItem*) malloc(sizeof(struct DataItem));
    dummyItem->id = -1;
    dummyItem->index = -1;

    neighborItem = (struct DataItem*) malloc(sizeof(struct DataItem));
    neighborItem->id = -1;
    neighborItem->index = -1;

    long *edgeIDs = malloc(sizeof(long) * SIZE);

    clock_t t;
    double time_taken, time_taken1, time_taken2;


    fp = fopen(argv[1], "r");
    if (fp == NULL){
         if (world_rank==0)
            printf("%s File not found", argv[1]);
         exit(EXIT_FAILURE);
      }

      if (world_rank==0)
         t = clock();

    totalSize=0;
    while( fscanf(fp, "%ld\t%ld\n", &tempID1, &tempID2) != EOF )
    {
         edgeIDs[totalSize]= tempID1;
         totalSize++;
         edgeIDs[totalSize]= tempID2;
         totalSize++;
     }
     if (world_rank==0){
         t = clock() - t;
         time_taken1 = ((double)t)/CLOCKS_PER_SEC; // in seconds
         //printf("\nTime to read input file = %lf sec\n", time_taken);
       }
    fclose(fp);



    float *localCredits = malloc(sizeof(float) * totalSize);
    float *glocalCredits = malloc(sizeof(float) * totalSize);

    // for (i=0;i<totalSize;i++)
    //      localCredits[i]=0.0;
    //memset(localCredits, 0.0, totalSize);
    //memset(glocalCredits, 0.0, totalSize);

    // double **finalOutput=malloc(sizeof *finalOutput * totalSize);;
    // for (i = 0; i < totalSize; i++)
    //      {
    //      finalOutput[i] = malloc(sizeof *finalOutput[i] * (round+5));
    //       }
    float **finalOutput=malloc(sizeof *finalOutput * (round+5));;
    for (i = 0; i < (round+5); i++)
         {
         finalOutput[i] = malloc(sizeof *finalOutput[i] * totalSize);
          }


    //int tokenNum=0;
    int tempPartition;
    //long workpileCounter=0;
    fp2 = fopen(argv[2], "r");
    if (fp2 == NULL){
        if (world_rank==0)
            printf("%s File not found", argv[2]);
         exit(EXIT_FAILURE);
      }

    actualSize=0;
    if (world_rank==0)
       t = clock();
        //long *t1, *t2, *t3;
    while( fscanf(fp2, "%ld\t%d\t%d\n", &tempID1, &tempDeg, &tempPartition) != EOF )
        {
          finalOutput[0][actualSize]=tempID1;
          finalOutput[1][actualSize]=tempDeg;
          finalOutput[2][actualSize]=tempPartition;
          finalOutput[3][actualSize]=0.0;
          finalOutput[4][actualSize]=1.0;
          insert(tempID1,actualSize);
          actualSize++;
         }
         if (world_rank==0){
             t = clock() - t;
             time_taken2 = ((double)t)/CLOCKS_PER_SEC; // in seconds
             printf("\nTime to read input file = %lf sec\n", time_taken1+time_taken2);
           }

    fclose(fp2);




     // calling the getCredits function the number of times as per the round number given by user

     for(i=0; i<round; i++)
        {
          if (world_rank==0)
               t = clock();

          getCredits( edgeIDs,  i, totalSize,finalOutput,localCredits,glocalCredits,actualSize,world_rank);
       // calling getCredits function to update finalOutput's second to last columns by the credits of each round
           if (world_rank==0)
              {
               t = clock() - t;
               time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
               printf("\tTime for round %ld = %lf sec\n", i+1, time_taken);
              }
            }

      if (world_rank==0){

          FILE *fp1;
          char output[]="Output.txt";
          long b1;
          int b2,b3;
          //int sp_round;
          fp1=fopen(output,"w");

          t = clock();
          //for(i = 0; i<actualSize; i++) {
          for(i = actualSize; i--; ) {

              for(j=0;j<(round+5);j++){// ID+DEG+roundnumber
                  /*if (j==(round+4))
                     sp_round=j;
                  switch (j){
                    case 0:         b1=finalOutput[j][i];
                                    fprintf(fp1,"%ld",b1);
                                    fprintf(fp1,"\t");
                                    break;
                    case 1:         b2= finalOutput[j][i];
                                    fprintf(fp1,"%d",b2);
                                    fprintf(fp1,"\t");
                                    break;
                    case 2:         b3= finalOutput[j][i];
                                    fprintf(fp1,"%d",b3);
                                    fprintf(fp1,"\t");
                                    break;
                    case 3:         break;
                    case 4:         break;
                    // case sp_round:  fprintf(fp1,"%0.6lf",finalOutput[j][i]);
                    //                 break;
                    default:        fprintf(fp1,"%0.6lf",finalOutput[j][i]);
                                    fprintf(fp1,"\t");

                  }*/
                  if(j==0)
                    {
                      b1=finalOutput[j][i];
                      fprintf(fp1,"%ld",b1);
                      fprintf(fp1,"\t");
                      }
                  else if (j==1)
                    {
                      b2= finalOutput[j][i];
                      fprintf(fp1,"%d",b2);
                      fprintf(fp1,"\t");
                      }
                  else if (j==2)
                    {
                      b3= finalOutput[j][i];
                      fprintf(fp1,"%d",b3);
                      fprintf(fp1,"\t");
                      }
                  else {

                      if ((j>=5) && (j<(round+4))){
                          fprintf(fp1,"%0.6f",finalOutput[j][i]);
                          fprintf(fp1,"\t");
                        }
                      if(j==(round+4)){
                        fprintf(fp1,"%0.6f",finalOutput[j][i]);
                      }
                  }
              }
              fprintf(fp1,"\n");
            }
            fclose(fp1);
            t = clock() - t;
            time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
            printf("Time to write the output file = %lf sec\n\n", time_taken);
          }

    if (line)
        free(line);
    if (line2)
        free(line2);
    free(edgeIDs);
    free(localCredits);
    free (glocalCredits);
    for(i=0; i<(round+5); i++){
        free(finalOutput[i]);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    exit(EXIT_SUCCESS);
}
//mpicc part2-version2.c -o hello
//mpirun --mca shmem posix --oversubscribe -np 2 hello graph1.txt part1.txt 5 2
