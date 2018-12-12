/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj5.c
Pgm Desc: Shared Memory, Peterson Algorithm
Usage: ./a.out <time_parent> <time_child> <time_parent_non_cs> <time_child_non_cs>
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PETERSON_KEY ((key_t) 38870391) // my GU student id...

void parent(int, int);
void child(int, int);
void cs(char, int);
void non_cs(int);

// the struct to use for shared memory variable
typedef struct {
    int turn;
    int pr_child;
    int pr_parent;
} peterson;

/**
 * Command-Line Args: time_parent, time_child, time_parent_non_cs, time_child_non_cs
 */
void main(int argc, char *argv[])
{
    int time_parent, time_child, time_parent_non_cs, time_child_non_cs;

    if (argc <= 1)
    {
        // If no args, initialize to default
        time_parent = 1;
        time_child = 1;
        time_parent_non_cs = 2;
        time_child_non_cs = 2;
    }
    else if (argc == 5)
    {
        // If four args
        time_parent = atoi(argv[1]);
        time_child = atoi(argv[2]);
        time_parent_non_cs = atoi(argv[3]);
        time_child_non_cs = atoi(argv[4]);
    }
    else
    {
        // Bad number of args
        fprintf(stderr, "Error: Must have 0 or 4 arguments\n");
        fprintf(stderr, "usage: %s <time_parent> <time_child> ", argv[0]);
        fprintf(stderr, "<time_parent_non_cs> <time_child_non_cs>\n");
        exit(EXIT_FAILURE);
    }

    // Create a shared memory segment provided one does not exist already
    int shmid = shmget(PETERSON_KEY, sizeof(peterson), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach segment to this process
    peterson* shmptr = (peterson*) shmat(shmid, NULL, 0);
    if (shmptr < 0) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory for peterson
    shmptr->pr_child = 0;
    shmptr->pr_parent = 0;
    shmptr->turn = 0;

    pid_t pid_child;
    pid_t pid_parent;
    int status;

    if ((pid_child = fork()) == 0) {
        child(time_child, time_child_non_cs);
        exit(0);
    } else if (pid_child < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if ((pid_parent = fork()) == 0) {
        parent(time_parent, time_parent_non_cs);
        exit(0);
    } else if (pid_parent < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    waitpid(pid_child, &status, 0);
    waitpid(pid_parent, &status, 0);

    shmdt(shmptr); // Detach the shared memory segment
    shmctl(shmid, IPC_RMID, (struct shmid_ds*) NULL); // Remove the shared memory segment
}

/**
 * Parent Process.
 */
void parent(int time_crit_sect, int time_non_crit_sect)
{
    // Get the shared memory segment
    int shmid = shmget(PETERSON_KEY, sizeof(peterson), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach it to this process
    peterson* shmptr = (peterson*) shmat(shmid, NULL, 0);
    if (shmptr < 0) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++)
    {
        // Update peterson states
        shmptr->pr_parent = 1;
        shmptr->turn = 1;

        // busy wait
        while (shmptr->pr_child && shmptr->turn == 1);

        // critical section
        cs('p', time_crit_sect);
        shmptr->pr_parent = 0;

        // non-critical section
        non_cs(time_non_crit_sect);
    }

    // Detach the shared memory segment
    shmdt(shmptr);
}

/**
 * Child Process.
 */
void child(int time_crit_sect, int time_non_crit_sect)
{
    // Get the shared memory segment
    int shmid = shmget(PETERSON_KEY, sizeof(peterson), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach it to this process
    peterson* shmptr = (peterson*) shmat(shmid, NULL, 0);
    if (shmptr < 0) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++)
    {
        // Update peterson states
        shmptr->pr_child = 1;
        shmptr->turn = 0;

        // busy wait
        while (shmptr->pr_parent && shmptr->turn == 0);

        // critical section
        cs('c', time_crit_sect);
        shmptr->pr_child = 0;

        // non-critical section
        non_cs(time_non_crit_sect);
    }

    // Detach the shared memory segment
    shmdt(shmptr);
}

/**
 * Critical section.
 */
void cs(char process, int time_crit_sect)
{
    if (process == 'p')
    {
        printf("parent in critical section\n");
        sleep(time_crit_sect);
        printf("parent leaving critical section\n");
    }
    else
    {
        printf("child in critical sction\n");
        sleep(time_crit_sect);
        printf("child leaving critical section\n");
    }
}

/**
 * Non-critical section.
 */
void non_cs(int time_non_crit_sect)
{
    sleep(time_non_crit_sect);
}