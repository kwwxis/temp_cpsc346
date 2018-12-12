/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj7.c
Pgm Desc: Readers-Writers Problem with Linux Pipes and Pthreads
Compile: gcc proj7.c -lpthread
Usage: ./a.out <int>

NOTE: will usually use all 3 of the reader threads, but not necessarily
every time. May have to run a few times.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>

#define NUM_THREADS 4
#define P_READ 0
#define P_WRITE 1

void* writer_thread(void*);
void* reader_thread(void*);
int is_prime(int);
int randInt(int, int);

int amount_of_numbers; // amount of numbers for the writer to generate
int pipefd[2]; // the pipe file descriptors...

/**
 * Entrypoint. Creates a pipe and runs 1 writer thread and 3 reader threads.
 */
int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Error: argument required\n");
        fprintf(stderr, "usage: %s <int>\n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (argc > 2) {
        fprintf(stderr, "Error: too many arguments\n");
        fprintf(stderr, "usage: %s <int>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    amount_of_numbers = atoi(argv[1]);
    
    if (pipe(pipefd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    pthread_t threads[NUM_THREADS];
    int tid = 0;
    
    // Create writer thread
    pthread_create(&threads[tid], NULL, writer_thread, (void*) (intptr_t) tid++);

    // Create reader threads
    for (; tid < NUM_THREADS; tid++)
    {
        pthread_create(&threads[tid], NULL, reader_thread, (void*) (intptr_t) tid);
    }
    
    // Join all threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    // Close pipe and return
    close(pipefd[0]);
    close(pipefd[1]);
    return 0; 
}

/**
 * The writer thread, writes a bunch of large integers into the pipe.
 */
void* writer_thread(void* argp)
{
    int tid = (int) (intptr_t) argp;
    
    for (int i = 0; i < amount_of_numbers; i++)
    {
        int num = randInt(1000, 100000); 
        
        printf("Writer: generated number, %i\n", num);
        
        if (write(pipefd[P_WRITE], &num, sizeof(num)) < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    
    close(pipefd[P_WRITE]);
    pthread_exit(NULL);
}

/**
 * The reader thread, reads integers from the pipe and checks if they're prime.
 */
void* reader_thread(void* argp)
{
    int tid = (int) (intptr_t) argp, num;
    
    usleep(randInt(0, 500) * 1000); // sleep between 0 and 500ms before starting read loop
    
    while (read(pipefd[P_READ], &num, sizeof(num)) > 0)
    {
        usleep(randInt(0, 1000) * 1000); // sleep between 0 and 1000ms
        printf("Reader %i: %i is %s\n", tid, num, is_prime(num) ? "prime" : "not prime");
    }
    
    pthread_exit(NULL);
}

/**
 * Checks if an integer is prime
 *
 * @param int num the integer to check if prime
 * @return int 1 if prime, 0 otherwise
 */
int is_prime(int num)
{
    if (num <= 1) return 0;
    if (num % 2 == 0 && num > 2) return 0;
    for(int i = 3; i < num / 2; i += 2)
    {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

/**
 * Generate a random integer between the min and max.
 *
 * @param int min the minimum bound
 * @param int max the maximum bound
 * @return int a number between the minimum and maximum bounds
 */
int randInt(int min, int max) {
    return rand() % (max + 1 - min) + min;
}