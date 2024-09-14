#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>
#include <bits/time.h>



#define Row 100
#define Columns 100
#define Processes_NUM 3
#define Threads_NUM 3
#define DETACH_THREADS 1


int matrixA[Row][Columns];
int matrixB[Row][Columns];
int resultMatrix[Row][Columns];


void *threadFunction(void* arg);
void generateMatrix(int number, int matrix[Row][Columns]);
void multiplyMatrices();
void multiplyMatrices2(int start_row, int end_row);
void generateMatrix(int number , int matrix[Row][Columns]);
void childProcesses(int pipe_fd[2], int start_row, int end_row);



int main()
{
    int student_number = 1210582; 
    int birthYear = 2003;
    int i, j, k;

    generateMatrix(student_number , matrixA);
    long long int number = (long long int)student_number * birthYear;

    //printf("%lld",number);
    generateMatrix(number , matrixB);

    printf("Number of Child Processes:%d\n",Processes_NUM);
    printf("Number of Threads:%d\n",Threads_NUM);


    // Approach A: Native approach (single process)
                                     
struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
        multiplyMatrices();
    clock_gettime(CLOCK_MONOTONIC, &end);

printf("Time taken for Native Approach : %f seconds\n",
       (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);


for (i = 0; i < Row; ++i) {
        for (j = 0; j < Columns; ++j) {
            resultMatrix[i][j] = 0;
        }
    }


    //Approach B: Multiple child processes

int pipe_fd[Processes_NUM][2];

    struct timeval startB, endB;
    gettimeofday(&startB, NULL);

    // Fork child processes
    for (int i = 0; i < Processes_NUM; ++i) {
        if (pipe(pipe_fd[i]) == -1) {
            printf("Pipe creation failed");
        }

        pid_t child_pid = fork();

        if (child_pid == -1) {
            printf("Fork failed");
        } else if (child_pid == 0) {
            int chunk_size = Row / Processes_NUM;
            int start_row = i * chunk_size;
            int end_row = (i == Processes_NUM - 1) ? Row : (start_row + chunk_size);

            close(pipe_fd[i][0]);
            childProcesses(pipe_fd[i], start_row, end_row);
            close(pipe_fd[i][1]);
            exit(0);
        } else {
            close(pipe_fd[i][1]);
        }
    }

    for (int i = 0; i < Processes_NUM; ++i) {
        wait(NULL);
        int chunk_size = Row / Processes_NUM;
        int startRead = i * chunk_size;
        int end_rowR = (i == Processes_NUM - 1) ? Row : ((i + 1) * (Row / Processes_NUM));

        for (int k = startRead; k < end_rowR; k++) {
            read(pipe_fd[i][0], &resultMatrix[startRead][0], (end_rowR - startRead) * Columns * sizeof(int));
        }
        close(pipe_fd[i][0]);
    }

    gettimeofday(&endB, NULL);
    printf("Time taken for Child processes : %f seconds\n", (endB.tv_sec - startB.tv_sec) + (endB.tv_usec - startB.tv_usec) / 1e6);


  

    // Initialize the resultMatrix with zeros
    for (i = 0; i < Row; ++i) {
        for (j = 0; j < Columns; ++j) {
            resultMatrix[i][j] = 0;
        }
    }



    //Approach C: Multi threads

    /*JOINABLE THREADS*/

    pthread_t threads[Threads_NUM];
    int thread_ids[Threads_NUM];

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    
    struct timeval startTH , endTH ;
    gettimeofday(&startTH , NULL);


    for (int i = 0; i < Threads_NUM; ++i) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], &attr, threadFunction, (void*)&thread_ids[i]) != 0) {
            perror("Thread creation failed");
            return 1;         // Exit the program with an error code
        }
    }

    for (int i = 0; i < Threads_NUM; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&attr);   // Cleanup thread attributes

    gettimeofday(&endTH ,NULL);


   //printMatrix(resultMatrix);
    double timeExec = (endTH.tv_sec - startTH.tv_sec)+ (endTH.tv_usec - startTH.tv_usec)/1.0e6;
    printf("Time taken for Joinable Threads: %f seconds\n", timeExec);


for (i = 0; i < Row; ++i) {                   
        for (j = 0; j < Columns; ++j) {
            resultMatrix[i][j] = 0;
        }
    }


    /*DETACHED THREADS*/

    struct timeval startDetach, endDetach;
    gettimeofday(&startDetach, NULL);


    pthread_t threadsDetach[Threads_NUM];
    int thread_idsDetach[Threads_NUM];

    for (int i = 0; i < Threads_NUM; ++i) {
        thread_idsDetach[i] = i;
        if (pthread_create(&threadsDetach[i], NULL, threadFunction, (void*)&thread_idsDetach[i]) != 0) {
            perror("Thread creation failed");
            return 1; // Exit the program with an error code
        }
        pthread_detach(threadsDetach[i]);
        // Detach the thread immediately after creation
    }


         gettimeofday(&endDetach ,NULL);


      //printMatrix(resultMatrix);
    
    double timeExecD = (endDetach.tv_sec - startDetach.tv_sec)+ (endDetach.tv_usec - startDetach.tv_usec)/1.0e6;
    printf("Time taken for Detached Threads: %f seconds\n", timeExecD);

    

for (i = 0; i < Row; ++i) {
        for (j = 0; j < Columns; ++j) {
            resultMatrix[i][j] = 0;
        }
    }



  //Detach & Joinable (MIX)
 pthread_t threads_MIXED[Threads_NUM];
    int thread_ids_M[Threads_NUM];

    pthread_attr_t attr_MIXED;
    pthread_attr_init(&attr_MIXED);

    struct timeval startJD, endJD;
    gettimeofday(&startJD, NULL);

    // Create detached threads first
    for (int i = 0; i < DETACH_THREADS; ++i) {
        thread_ids_M[i] = i;
        if (pthread_create(&threads_MIXED[i],NULL , threadFunction, (void*)&thread_ids_M[i]) != 0) {
            perror("Thread creation failed");
            return 1;                       // Exit the program with an error code
        }
        pthread_detach(threads_MIXED[i]);
    }

    // Create joinable threads after detached threads
    for (int i = DETACH_THREADS; i < Threads_NUM; ++i) {
        thread_ids_M[i] = i;
        if (pthread_create(&threads_MIXED[i], &attr_MIXED, threadFunction, (void*)&thread_ids_M[i]) != 0) {
            perror("Thread creation failed");
            return 1; 
        }
    }

    pthread_attr_destroy(&attr);

    // Wait for joinable threads to finish
    for (int i = DETACH_THREADS; i < Threads_NUM; ++i) {
        pthread_join(threads_MIXED[i], NULL);
    }

    gettimeofday(&endJD ,NULL);
    double timeExecJD = (endJD.tv_sec - startJD.tv_sec)+ (endJD.tv_usec - startJD.tv_usec)/1.0e6;
    printf("Time taken for Mixed Threads(JOINALBE & DETACHED): %f seconds\n", timeExecJD);



    return 0;
}



void generateMatrix(int number , int matrix[Row][Columns]){

     int digitIndex = 0;
     char numStr[100];                      
     sprintf(numStr, "%d", number);

    int numLength = strlen(numStr);


      for (int i = 0; i < Row; ++i) {                     
        for (int j = 0; j < Columns; ++j) {
            // Repeat the sequence of digits
            matrix[i][j] = numStr[digitIndex] - '0';
            digitIndex = (digitIndex + 1) % numLength;
        }
    }


 }


//mutliply function for native approach since single proces no need to divide the matrix 
void multiplyMatrices() {
    
    // Multiply matrixA and matrixB
    for (int i = 0; i < Row; ++i) {
        for (int j = 0; j < Columns; ++j) {
            for (int k = 0; k < Row; ++k) {
                resultMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
}



void printMatrix(int matrix[Row][Columns]) {
    for (int i = 0; i < Row; ++i) {
        for (int j = 0; j < Columns; ++j) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void* threadFunction(void* arg) {
    int thread_id = *((int*)arg);

    int size = Row / Threads_NUM;
    int start_row = thread_id * size;
    int end_row = (thread_id == Threads_NUM - 1) ? Row : (start_row + size);

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < Columns; ++j) {
            int result = 0;
            for (int k = 0; k < Columns; ++k) {
                result += matrixA[i][k] * matrixB[k][j];
            }

            resultMatrix[i][j] = result;
        }
    }

    pthread_exit(NULL);
}
 

//this multiply function for multi-processes and multi-threaded approches 
 void multiplyMatrices2(int start_row, int end_row) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < Columns; ++j) {
            int result = 0;
            for (int k = 0; k < Columns; ++k) {
                result += matrixA[i][k] * matrixB[k][j];
            }
            resultMatrix[i][j] = result;
        }
    }
}

void childProcesses(int pipe_fd[2], int start_row, int end_row) {
    // Close the read end of the pipe
    close(pipe_fd[0]);

    multiplyMatrices2(start_row, end_row);

    int rows_to_send = end_row - start_row;

    write(pipe_fd[1], &resultMatrix[start_row][0], rows_to_send * Columns * sizeof(int));

    // Close the write end of the pipe
    close(pipe_fd[1]);

    exit(0);
}