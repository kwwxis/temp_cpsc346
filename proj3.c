/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj3.c
Pgm Desc: Exploration of the proc file system
Usage: 1) standard:  ./a.out -s
Usage: 2) history:  ./a.out -h
Usage: 3) load:  ./a.out -l
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

void standard();
void history();
void load();

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Error: Option required\n");
        fprintf(stderr, "usage: ./a.out -s|-h|-l\n");
        exit(EXIT_FAILURE);
    }
    if (argc > 2) {
        fprintf(stderr, "Error: Too many options\n");
        fprintf(stderr, "usage: ./a.out -s|-h|-l\n");
        exit(EXIT_FAILURE);
    }

    int flagS = !strcmp(argv[1],"-s");
    int flagH = !strcmp(argv[1],"-h");
    int flagL = !strcmp(argv[1],"-l");

    if (!flagS && !flagH && !flagL) {
        fprintf(stderr, "Error: Unknown option: %s\n", argv[1]);
        fprintf(stderr, "usage: ./a.out -s|-h|-l\n");
        exit(EXIT_FAILURE);
    }

    if (flagS)
        standard();
    if (flagH)
        history();
    if (flagL)
        load();
}

/**
 * Displays CPU vendor_id, model name, and OS version.
 */
void standard()
{
    char ch;
    FILE* ifp;
    char str[80];

    ifp = fopen("/proc/cpuinfo","r");
    while (fgets(str,80,ifp) != NULL)
        if (strstr(str,"vendor_id") || strstr(str,"model name"))
            puts(str);
    fclose(ifp);

    ifp = fopen("/proc/version","r");
    while ((ch = getc(ifp)) != EOF)
        putchar(ch);
    fclose(ifp);
}

/**
 * Displays time since the last reboot (DD:HH:MM:SS), time when the system was last booted
 * (MM/DD/YY - HH:MM), number of processes that have been created since the last reboot
 */
void history()
{
    FILE* ifp;
    char str[80];

    ifp = fopen("/proc/uptime","r");
    fgets(str,sizeof(str),ifp);
    fclose(ifp);

    int uptime = (int) atof(strtok(str, " ")); // seconds since last boot
    time_t current_time = time(NULL); // current time in seconds
    time_t boot_time = current_time - (time_t) uptime; // unix timestamp of last boot

    char timeString[15];
    strftime(timeString, sizeof(timeString), "%m/%d/%y %H:%M", localtime(&boot_time));

    printf("Last Boot Time: %s\n", timeString);
    printf("Time Since Last Boot: %02d:%02d:%02d:%02d\n",
          (uptime % (86400 * 30) / 86400), // days
          (uptime % 86400 / 3600), // hours
          (uptime % 3600 / 60), // minutes
          (uptime % 60) // seconds
        );

    ifp = fopen("/proc/stat","r");
    while (fgets(str,sizeof(str),ifp) != NULL) {
        if (strstr(str,"processes")) {
            strtok(str, " ");
            printf("Processes Since Boot: %s\n", strtok(NULL, "\n"));
        }
    }
    fclose(ifp);
}

/**
 * Displays total memory, available memory, load average (avg. number of processes over the last minute).
 */
void load()
{
    FILE* ifp;
    char str[80];

    ifp = fopen("/proc/meminfo","r");
    while (fgets(str,sizeof(str),ifp) != NULL) {
        char* label = strtok(str, ":");

        if (!strcmp("MemTotal", label)) {
            char* value = strtok(NULL, "\n");
            while (isspace(*value)) value++;
            printf("Total Memory: %s\n", value);
        }

        if (!strcmp("MemFree", label)) {
            char* value = strtok(NULL, "\n");
            while (isspace(*value)) value++;
            printf("Available Memory: %s\n", value);
        }
    }
    fclose(ifp);

    ifp = fopen("/proc/loadavg","r");
    fgets(str,sizeof(str),ifp);
    strtok(str, " ");
    strtok(NULL, " ");
    strtok(NULL, " ");
    printf("Load Average: %s\n", strtok(NULL, " ")); // get 4th column
    fclose(ifp);
}
