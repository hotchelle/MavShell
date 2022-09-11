// The MIT License (MIT)
//
// Copyright (c) 2016 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11 // Mav shell only supports ten arguments

// This is the value that will keep track of updating eaching process in pid_history[]
int pid_index = 0;

// tracks the processes that are being used up to 15
pid_t pid_history[15];

// track the command_history array
int history_index = 0;

// keeps track of commands used
char command_history[15][MAX_COMMAND_SIZE];

// This function doesn't take any parameters and doesn't return anything.
// This function is responsible for printing out the pids in the pid_history array
void print_pid_history()
{
    int count = 0;
    int i;
    for (i = 0; i < 15; i++)
    {
        // printing the pids that were are filled within the array
        if (pid_history[i] != -1)
        {
            printf("%d: %d\n", i, pid_history[i]);
        }
        // incrementing the count index with the number of pids
        count++;

        if (count > 14)
        {
            count = 0;
        }
    }
}

int main()
{
    int i;

    // initializing pid_history values to -1
    for (i = 0; i < 15; i++)
    {
        pid_history[i] = -1;
    }

    // initializing command_history values to NULL
    for (i = 0; i < 15; i++)
    {
        memset(command_history[i], '\0', MAX_COMMAND_SIZE);
    }

    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);

    while (1)
    {
        // Print out the msh prompt
        printf("msh> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
            ;

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *argument_ptr;

        // allows command history to be called again when using '!'
        strncpy(command_history[history_index], command_string, 255);

        // This case handles the copies the command
        // from the command history to execute it again
        if (command_string[0] == '!')
        {
            int index = atoi(&command_string[1]);
            strncpy(command_string, command_history[index], 255);
        }

        char *working_string = strdup(command_string);

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;

        // Tokenize the input strings with whitespace used as the delimiter
        while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        // Now print the tokenized input as a debug check
        // TODO Remove this code and replace with your shell functionality
        int token_index = 0;
        // Ignores spaces and skipping lines
        if (token[0] == '\0')
        {
            continue;
        }
        strncpy(command_history[history_index++], token[0], MAX_COMMAND_SIZE);

        if (!strcmp(token[0], "cd"))
        {
            // checking if the directory exits
            int ret = chdir(token[1]);
            if (ret == -1)
            {
                printf("cd: no such file or directory: %s\n", token[1]);
            }
        }

        // Exiting from the program
        else if (!strcasecmp(token[0], "exit") || !strcasecmp(token[0], "quit"))
        {
            exit(0);
        }

        // prints out the pidhistory
        else if (!strcmp("pidhistory", token[0]))
        {
            print_pid_history();
        }

        // prints out command history
        else if (!strcmp("history", token[0]))
        {
            int count = 0;
            for (i = 0; i < 15; i++)
            {
                // print only values that are not initialized to '\0'
                if (command_history[i][0] != '\0')
                {
                    printf("%d: %s\n", i, command_history[i]);
                }
                // makes sure that commands are under 15
                if (count > 14)
                {
                    count = 0;
                }
                count++;
            }
        }
        // checking if in range for command history
        else if (history_index > 14)
        {
            // only keeps track of 15 commands
            for (i = 0; i < 15; i++)
            {
                strncpy(command_history[i], command_history[i + 1], MAX_COMMAND_SIZE);
            }
            // replaces the last element to work as a queue in terms of storing commands
            strncpy(command_history[14], command_string, MAX_COMMAND_SIZE);
            history_index = 0;
        }

        else
        {

            // to track the value of exec
            int exec_ok;

            // create new process
            pid_t pid = fork();
            pid_history[pid_index++] = pid;

            // only keeps track of 15 processes
            if (pid_index > 14)
            {
                pid_index = 0;
            }

            // Checks for the child
            if (pid == 0)
            {
                // calls exec to copy the process and stores return value in case it failed.
                exec_ok = execvp(token[0], &token[0]);

                // if exec fails
                if (exec_ok == -1)
                {
                    printf("%s: Command not found.\n", token[0]);
                    exit(0);
                }
            }
            else
            {
                // this value's address is passed in wait, to make the parent process wait
                int status;
                wait(&status);
            }
        }

        free(head_ptr);
    }
    return 0;
    // e2520ca2-76f3-11ec-90d6-0242ac120003
}
