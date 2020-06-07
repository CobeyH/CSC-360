/*
 * CSHELL
 * Cobey Hollier
 * V00893715
 * May 15th, 2020
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

#define MAX_COMMAND_LEN 256
#define PROCESS_LIMIT 5

struct process {
    pid_t pid;
    char *command;
    int running;
};

// Splits the user input into an array of strings for easier processing
char** parseString(char* cmd, char *tokens[], int *bg) {
    char *ptr = strtok(cmd, " ");
    int i = 0;
    while(ptr != NULL) {
        tokens[i++] = ptr;
        ptr = strtok(NULL, " ");
    }
    tokens[i] = NULL;
    if(tokens[0] != NULL) {
        *bg = strcmp(tokens[0], "bg"); 
        if(*bg == 0) {
            return &tokens[1];
        }
    }
    return &tokens[0];
}


// Iterates through the list of processes, removing the PIDS of terminated processes and rearanging process array.
void cleanupProcessList(struct process processes[], int *jobCount) {
    //Set up processes struct
    struct process *p = processes;
    for(int j = 0; j < PROCESS_LIMIT; j++) {
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
            free(processes[j].command);
            *jobCount = *jobCount - 1;
            // Still running case
        } else {
            if(p->pid != processes[j].pid) {
                p->pid = processes[j].pid;
                p->command = processes[j].command;
                p->running = processes[j].running;
                processes[j].pid = 0;
            }
            p++;
        }
    }
}

// Returns the working directory of the parent
void getWorkingDirectory(char wd[MAX_COMMAND_LEN]) {
    if(getcwd(wd, MAX_COMMAND_LEN) == NULL) {
        perror("Failed to get working directory\n");
    }
}

// Handles user commands that don't need to be handled by the parent
void handleChildProccess(char *tokens[], struct process processes[], int jobCount) {
    if(strcmp(tokens[0], "pwd") == 0) {
        char wd[MAX_COMMAND_LEN];
        getWorkingDirectory(wd);
        printf("%s \n", wd);
    } else if(strcmp(tokens[0], "bglist") == 0) {
        for(int i = 0; i < jobCount; i++) {
            char status = processes[i].running == 0 ? 'S' : 'R';
            printf("%d[%c]: %s\n", i, status, processes[i].command);
        }
        printf("Total Background Jobs: %d\n", jobCount);
    }  else {
         execvp(tokens[0], tokens);
    }
}

// Terminated a job running in the background
void killJob(char *tokens[], int jobCount, struct process processes[]) {
    if(tokens[1] == NULL) {
        printf("Incorrect arguments passed. bgkill requires a process number to kill\n");
        return;
    } else if(atoi(tokens[1]) >= jobCount) {
        printf("The job you have specified with index %d does not exist\n there are %d jobs running\n", atoi(tokens[1]), jobCount);
        return;
    }
    int jobToKill = atoi(tokens[1]);
    if(jobToKill > 4) {
        printf("ERROR: Job specified is greater than the maximum number of jobs\n");
        return;
    }
    int killStatus = kill(processes[jobToKill].pid, SIGKILL);
    if(killStatus < 0) {
        printf("Failed to kill job\n");
    }

}

// Stops a process with the passed in number if the process isn't already stopped
void stopJob(char* tokens[], struct process processes[]) {
    int jobToStop = atoi(tokens[1]);
    struct process process = processes[jobToStop];
    if(process.running == 0) {
        printf("The job you have specified is already stopped\n");
        return;
    } else {
        int status = kill(process.pid, SIGSTOP);
        if(status < 0) {
            printf("Failed to stop process\n");
            return;
        } else {
            processes[jobToStop].running = 0;
        }
    }
}

// Starts a process with the passed in number if the process isn't already running
void startJob(char* tokens[], struct process processes[]) {
    int jobToStart = atoi(tokens[1]);
    struct process process = processes[jobToStart];
    if(process.running == 1) {
        printf("The job you have specified is already running\n");
        return;
    } else {
        int status = kill(process.pid, SIGCONT);
        if(status < 0) {
            printf("Failed to restart process");
            return;
        } else {
            processes[jobToStart].running = 1;
        }
    }
}

int main ( void ) {
    char display[MAX_COMMAND_LEN + 3];
    getWorkingDirectory(display);
    struct process processes[5];
    for(int i = 0; i < 5; i++) {
        processes[i].pid = 0; 
        processes[i].running = 0;
    }
    int jobCount = 0;
    while(1) {
        // Handle user input parsing into an array of strings
        getWorkingDirectory(display);
        strcat(display, " > ");
        char *cmd = readline (display);
        pid_t pid;
        char *tokens[17];
        int bg = 0;
        char **parsedCommand = parseString(cmd, tokens, &bg);
        cleanupProcessList(processes, &jobCount);
        if(tokens[0] == NULL) continue;
        if(strcmp(tokens[0], "cd") == 0) {
            if(chdir(tokens[1]) != 0) {
                perror("Failed to change directory\n");
            }
        } else if(strcmp(tokens[0], "stop") == 0) {
            stopJob(tokens, processes);
        } else if(strcmp(tokens[0], "start") == 0) {
            startJob(tokens, processes);
        } else if(strcmp(tokens[0], "bgkill") == 0) {
            killJob(tokens, jobCount, processes);
        } else {
            pid = fork();
            if(pid < 0) {
                printf("Fork Failed.\n");
                return -1;
            }
            if (pid == 0) {
                handleChildProccess(parsedCommand, processes, jobCount);
                return 1;
            } else {
                // Case for foreground operation in the parent
                if(bg != 0) {
                    int status;
                    wait(&status);
                    // Case for background operation
                } else {
                    char* commandCopy = malloc(strlen(parsedCommand[0]));
                    strcpy(commandCopy, parsedCommand[0]);
                    processes[jobCount].command = commandCopy;
                    processes[jobCount].pid = pid;
                    processes[jobCount].running = 1;
                    jobCount++;
                }
            }
        }
        free (cmd);
    }
}

