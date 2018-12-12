/*
Class: CPSC 346-01
Team Member 1: Matthew Lee
Team Member 2: N/A
GU Username of project lead: mlee8
Pgm Name: proj4.c
Pgm Desc: Building a Shell
Compile: gcc -std=c11 proj4.c -o a.out
Usage: ./a.out
*/

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE 80
#define MAX_TOKENS 41
#define TOKEN_DELIM " \t\r\n\a"
#define MAX_HISTORY 10

char** getInput();
char** parseInput(char*);
void dispOutput(char**);
int countArgs(char**);

int myshell_execute(char** args);
int myshell_launch(int, char** args);

int myshell_cd(int, char** args);
int myshell_quit(int, char** args);
int myshell_history(int, char** args);
int myshell_history_execute(int);
void history_add(char** args);

char** history[MAX_HISTORY];
unsigned history_count = 0;

int main(int argc, char* argv[])
{
    int status = 1;

    while (status)
    {
        printf("myShell> ");
        fflush(stdin);
        fflush(stdout);

        char** args = getInput();
        //dispOutput(args);

        // Execute command
        status = myshell_execute(args);

        // Add new line after command executed
        printf("\n");

        // Add command to history once finished
        // (built-in commands (not added to history) return 2,
        // other commands return 1)
        if (status == 1 && args != NULL && args[0] != NULL)
        {
            history_add(args);
        }

        free(args);
    }

    return 0;
}

// --------------------------------------------------------------------------------
// COMMAND EXECUTION FUNCTIONS
// --------------------------------------------------------------------------------

int myshell_execute(char** args)
{
    if (args == NULL || args[0] == NULL)
    {
        return 1;
    }

    int argc = countArgs(args);

    // Check for built-in commands:
    // (these don't get added to history)

    if (!strcmp(args[0], "cd"))
    {
        return myshell_cd(argc, args);
    }
    else if (!strcmp(args[0], "history"))
    {
        return myshell_history(argc, args);
    }
    else if (!strcmp(args[0], "exit"))
    {
        return myshell_quit(argc, args);
    }
    else if (!strcmp(args[0], "quit"))
    {
        return myshell_quit(argc, args);
    }
    else if (!strcmp(args[0], "!!"))
    {
        return myshell_history_execute(-1);
    }
    else if (!strncmp("!", args[0], 1))
    {
        char numstr[strlen(args[0])];
        strcpy(numstr, args[0] + 1);

        int num = atoi(numstr);
        return myshell_history_execute(num);
    }

    // Run non-built-in commands:
    return myshell_launch(argc, args);
}

int myshell_launch(int argc, char** args)
{
    int runInBackground = 0;

    // Search for "&"
    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(args[i], "&"))
        {
            runInBackground = 1;
            args[i] = NULL;
            break;
        }
    }

    int status;
    pid_t pid = fork();
    pid_t wpid;

    if (pid == 0)
    {
        // Child process block
        // Execute the command in the child process

        if (execvp(args[0], args) == -1)
        {
            perror("myShell");
        }

        exit(0);
    }
    else if (pid < 0)
    {
        // There was an error trying to fork
        perror("fork");
    }
    else
    {
        // Parent process block
        if (!runInBackground)
        {
            waitpid(pid, &status, 0); // wait for child process to finish
        }
    }

    return 1;
}

// --------------------------------------------------------------------------------
// HISTORY HANDLING & COMMANDS
// --------------------------------------------------------------------------------

void history_add(char** args)
{
    // First, duplicate "args" into "dup"
    int n = countArgs(args);
    char** dup = malloc(sizeof(char*) * n+1);

    for (int i = 0; i < n; i++)
    {
        dup[i] = strdup(args[i]);
    }
    dup[n] = 0; // null terminator

    if (history_count < MAX_HISTORY)
    {
        // Add item
        history[history_count++] = dup;
    }
    else
    {
        char** ptr = history[0];

        // Each element must be freed because "strdup()"
        for (char* c = *ptr; c; c=*++ptr)
        {
            free( c );
        }

        // Free the entire array
        free( history[0] );

        // Shift everything down
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            history[i - 1] = history[i];
        }

        // Add item
        history[MAX_HISTORY - 1] = dup;
    }
}

int myshell_history_execute(int item)
{
    if (item == -1) // Will be -1 if "!!"
    {
        item = 1; // Set to first item
    }

    if (!item || item > MAX_HISTORY || item < 0)
    {
        printf("myshell: invalid N for \"!N\" -- must be an integer between 1 and %i\n", MAX_HISTORY);
        return 2;
    }

    if (item > history_count)
    {
        printf("myshell: no command at that Nth history item yet\n", item);
        return 2;
    }

    int idx = history_count - item;

    // Return 2, not the status of execute so that we don't add the
    // "!!" or "!N" to history
    myshell_execute(history[idx]);
    return 2;
}

int myshell_history(int argc, char** args)
{
    if (history_count == 0)
    {
        printf("myshell: no history yet\n");
        return 2;
    }

    for (int i = 0; i < history_count; i++)
    {
        printf("%i ", history_count - i, i);

        char** itemArgs = history[i];

        for (int j = 0; ; j++)
        {
            if (itemArgs[j] == NULL)
                break;
            printf("%s ", itemArgs[j]);
        }

        printf("\n");
    }

    return 2;
}

// --------------------------------------------------------------------------------
// BUILT-IN COMMANDS
// --------------------------------------------------------------------------------

int myshell_cd(int argc, char** args)
{
    if (args[1] == NULL)
    {
        printf("myshell: expected argument to \"cd\"");
        return 2;
    }

    if (chdir(args[1]) != 0)
    {
        perror("myShell");
    }

    return 2;
}

int myshell_quit(int argc, char** args)
{
    return 0;
}

// --------------------------------------------------------------------------------
// I/O AND PARSING FUNCTIONS
// --------------------------------------------------------------------------------

/**
 * Reads input string from the keyboard, then invokes and returns
 * the result of parseInput.
 */
char** getInput()
{
    char* line;

    if (fgets(line, MAX_LINE, stdin) != NULL)
    {
        return parseInput(line);
    }

    return NULL;
}

/**
 * Parses the cstring keyboard input and returns an array of
 * cstrings holding each argument.
 */
char** parseInput(char* line)
{
    int bufsize = MAX_TOKENS, pos = 0;
    char** tokens = malloc(bufsize * sizeof(char*));

    char* token = strtok(line, TOKEN_DELIM);

    while (token != NULL)
    {
        tokens[pos++] = token;
        token = strtok(NULL, TOKEN_DELIM);
    }

    tokens[pos] = NULL;

    return tokens;
}

/**
 * Displays the arguments entered by the user and parsed by getInput
 */
void dispOutput(char** args)
{
    int i = 0;

    for (i = 0; ; i++)
    {
        if (args[i] == NULL)
            break;

        printf("args[%i] = %s\n", i, args[i]);
    }
}

/**
 * Returns the number of arguments in args.
 */
int countArgs(char** args)
{
    if (args == NULL || args[0] == NULL)
    {
        return 0;
    }

    int i;

    for (i = 0; ; i++)
    {
        if (args[i] == NULL)
            break;
    }

    return i;
}