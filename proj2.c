/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj2.c
Pgm Desc: Illustrates a common way to read from the keyboard in C
Usage: ./a.out
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 80

char* get_inp();
void disp_inp(char*);
int wc(char*);

int main()
{
    char* inp = get_inp();
    printf("Here is your input: %s\n",inp);

    disp_inp(inp);

    printf("Here is the number of words: %i\n", wc(inp));

    free(inp);
    return 0;
}


char* get_inp()
{
    char* inp = (char*) malloc(MAX_LINE);
    char* start = inp;
    char c;

    printf("Enter text <= 80 characters in length\n");
    while ((c = getchar()) != '\n')
        *inp++ = c;
    *inp = '\0';
    return start;
}

void disp_inp(char* out)
{
    while(*out)
        putchar(*out++);
    putchar('\n');
}

/**
 * Count the number of words in a cstring.
 *
 * @param char* inp the address of an 80 byte block of memory 
 * @return int the number of words stored in the block of memory.
 *   A word is any sequence of characters that is
 *   1) stored at the beginning of the block or
 *   2) stored at end of the block or
 *   3) terminated by the end-of-line character or
 *   4) preceded by and ended by one or more spaces
 */
int wc(char* inp)
{
    int numWords = 0;
    int inWord = 0;

    while(*inp) {
        char c = *inp++;

        if (c == ' ' || c == '\n') {
            if (inWord)
                numWords++;
            inWord = 0;
        } else {
            inWord = 1;
        }
    }

    if (inWord == 1)
        numWords++; // word at end of block

    return numWords;
}


