/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj6.c
Pgm Desc: Producer/Consumer Problem with Linux Semaphores
Usage: ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// ------------------------------------------------------------------------------------------
// FUNCTION HEADERS & GLOBAL VARIABLES

void prod_cons();
void producer();
void consumer();
void criticalSection(int);

const int PROD = 0;
const int CONS = 1;

int mutex, empty, full;

// ------------------------------------------------------------------------------------------
// SEMAPHORE HELPER FUNCTIONS

/**
 * Create a semaphore set of 1 semaphore specified by key.
 */
int sem_init(int key)
{
    int id = semget(key, 1, 0777 | IPC_CREAT);
    if (id < 0)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    return id;
}

/**
 * Remove semaphore.
 */
void sem_remove(int semid)
{
    semctl(semid, 1, IPC_RMID, 0);
}

/**
 * Set semaphore value.
 */
void sem_set(int semid, int val) {
    if (semctl(semid, 0, SETVAL, val) < 0)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

/**
 * Get semaphore value.
 */
int sem_get(int semid)
{
    return semctl(semid, 0, GETVAL, 0);
}

/**
 * Semaphore down operation.
 */
void sem_wait(int semid)
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;

    if (semop(semid, &buf, 1) < 0)
    {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

/**
 * Semaphore up operation.
 */
void sem_signal(int semid)
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = 0;

    if (semop(semid, &buf, 1) < 0)
    {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

// ------------------------------------------------------------------------------------------
// MAIN STUFF

/**
 * Entrypoint.
 */
int main(int argc, char *argv[])
{
    prod_cons();
    return 0;
}

void prod_cons()
{
    // Create semaphores, set to global variables
    mutex = sem_init(0);
    empty = sem_init(1);
    full = sem_init(2);

    // Set semaphore initial values
    sem_set(mutex, 1);
    sem_set(empty, 100);
    sem_set(full, 0);

    printf("Initial semaphore values: mutex = %d, empty = %d, full = %d\n",
        sem_get(mutex), sem_get(empty), sem_get(full));

    pid_t pid1, pid2;
    int status;

    if ((pid1 = fork()) == 0)
    {
        producer(); // Run producer in child process
    }
    else if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if ((pid2 = fork()) == 0)
    {
        consumer(); // Run consumer in child process
    }
    else if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Wait for producer and consumer processes to finish
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    printf("Final semaphore values: mutex = %d, empty = %d, full = %d\n",
        sem_get(mutex), sem_get(empty), sem_get(full));

    // Clean and remove semaphores
    sem_remove(mutex);
    sem_remove(empty);
    sem_remove(full);
}

/**
 * Producer process.
 */
void producer()
{
    for (int i = 0; i < 5; i++)
    {
        sem_wait(empty); // semaphore down operation
        sem_wait(mutex);

        criticalSection(PROD);

        sem_signal(mutex); // semaphore up operation
        sem_signal(full); // experiment with putting delays after this line

        sleep(2);
    }

    exit(0);
}

/**
 * Consumer process.
 */
void consumer()
{
    for (int i = 0; i < 5; i++)
    {
        sem_wait(full); // semaphore down operation
        sem_wait(mutex);

        criticalSection(CONS);

        sem_signal(mutex); // sempahore up operation
        sem_signal(empty); // experiment with putting delays after this line

        sleep(3);
    }

    exit(0);
}

void criticalSection(int who)
{
    if (who == PROD)
    {
        printf("Producer making an item\n");
    }
    else
    {
        printf("Consumer consuming an item\n");
    }
}