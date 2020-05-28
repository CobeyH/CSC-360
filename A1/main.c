/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

struct process {
    pid_t pid;
    char *command;
};

char** parseString(char* cmd, char *tokens[], int *bg) {
    char *ptr = strtok(cmd, " ");
    int i = 0;
	while(ptr != NULL) {
        tokens[i++] = ptr;
		ptr = strtok(NULL, " ");
	}
    tokens[i] = NULL;
    *bg = strcmp(tokens[0], "bg"); 
    if(*bg == 0) {
        printf("Going to run in background\n");
        return &tokens[1];
    }
    return &tokens[0];
}


// Iterates through the list of processes, removing the PIDS of terminated proccesses and rearanging proccess array.
void cleanupProcessList(struct process processes[], int *jobCount) {
        //Set up processes struct
        struct process *p = processes;
        for(int j = 0; j < 5; j++) {
            if(processes[j].pid == 0) {
                continue;
            }
            int status;
            int childpid = waitpid(processes[j].pid, &status, WNOHANG);
            if(childpid < 0) {
                printf("Error %d checking status of child with pid: %d\n", childpid, processes[j].pid);
            // Terminated Case
            } else if(childpid > 0) {
                processes[j].pid = 0;
                *jobCount = *jobCount - 1;
            // Still running case
            } else {
                if(p->pid != processes[j].pid) {
                    p->pid = childpid;
                    processes[j].pid = 0;
                    free(processes[j].command);
                }
                p++;
            }
        }
}

void handleChildProccess(char *tokens[], struct process processes[], int jobCount) {
    if(strcmp(tokens[0], "cd") == 0) {
        if(chdir(tokens[1]) != 0) {
            perror("Failed to change directory\n");
        }
    } else if(strcmp(tokens[0], "pwd") == 0) {
            char wd[256];
            if(getcwd(wd, sizeof(wd)) != 0) {
                perror("Failed to get working directory\n");
            }
            printf("%s \n", wd);
    } else if(strcmp(tokens[0], "bglist") == 0) {
        for(int i = 0; i < jobCount; i++) {
            printf("%d: %s\n", i, processes[i].command);
        }
        printf("Total Background Jobs: %d\n", jobCount);
    } else if(strcmp(tokens[0], "bgkill") == 0) {
        if(tokens[1] == NULL) {
            printf("Incorrect arguments passed. bgkill requires a process number to kill\n");
        } else if((int)tokens[1] < jobCount) {
            printf("The job you have specified does not exist\n");
        }
        int jobToKill = atoi(tokens[1]);
        if(jobToKill > 4) {
            return;
        }
        printf("Job to kill: %d\n", jobToKill);
        int killStatus = kill(processes[jobToKill].pid, SIGKILL);
        if(killStatus < 0) {
            printf("Failed to kill job\n");
        } else {
            processes[jobToKill].pid = 0;
            cleanupProcessList(processes, &jobCount);
        }
    } else {
        int rc;
        rc = execvp(tokens[0], tokens);
    }
}

int main ( void ) {
        char * display = "shell > ";
        struct process processes[5];
        for(int i = 0; i < 5; i++) {
            processes[i].pid = 0; 
        }
        int jobCount = 0;
        //TODO: Update the display with the working directory
        //TODO: Fix case where nothing is passed
        while(1) {
        // Handle user input parsing into an array of strings
		char *cmd = readline (display);
        pid_t pid;
        char *tokens[17];
        int bg = 0;
        char **parsedCommand = parseString(cmd, tokens, &bg);
        cleanupProcessList(processes, &jobCount);
        pid = fork();
        fflush(stdin);
        if(pid < 0) {
            printf("Fork Failed.\n");
            return -1;
        }
        fflush(stdin);
        if (pid == 0) {
             handleChildProccess(parsedCommand, processes, jobCount);
             return 1;
        } else {
            // Case for foreground operation in the parent
            if(bg != 0) {
                int status;
                pid_t anotherpid;
                anotherpid = wait(&status);
            // Case for background operation
            } else {
                char* commandCopy = malloc(strlen(parsedCommand[0]));
                strcpy(commandCopy, parsedCommand[0]);
                processes[jobCount].command = commandCopy;
                processes[jobCount].pid = pid;
                jobCount++;
            }
        }
		free (cmd);
    }
}

