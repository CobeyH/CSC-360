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

#define MAX_COMMAND_LEN 256
#define PROCESS_LIMIT 5

struct process {
    pid_t pid;
    char *command;
    int running;
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
        return &tokens[1];
    }
    return &tokens[0];
}


// Iterates through the list of processes, removing the PIDS of terminated proccesses and rearanging proccess array.
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
                    p->pid = childpid;
                    p->command = processes[j].command;
                    p->running = processes[j].running;
                    processes[j].pid = 0;
                }
                p++;
            }
        }
}

void getWorkingDirectory(char wd[MAX_COMMAND_LEN]) {
    if(getcwd(wd, MAX_COMMAND_LEN) == NULL) {
        perror("Failed to get working directory\n");
    }
}

void handleChildProccess(char *tokens[], struct process processes[], int jobCount) {
    if(strcmp(tokens[0], "pwd") == 0) {
            char wd[MAX_COMMAND_LEN];
            getWorkingDirectory(wd);
            printf("%s \n", wd);
    } else if(strcmp(tokens[0], "bglist") == 0) {
        for(int i = 0; i < jobCount; i++) {
            char status;
            if(processes[i].running == 0) {
                status = 'S';
            } else {
                status = 'R';
            }
            printf("%d[%c]: %s\n", i, status, processes[i].command);
        }
        printf("Total Background Jobs: %d\n", jobCount);
    }  else {
        int rc;
        rc = execvp(tokens[0], tokens);
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
        //TODO: Update the display with the working directory
        //TODO: Fix case where nothing is passed
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
        if(strcmp(tokens[0], "cd") == 0) {
                if(chdir(tokens[1]) != 0) {
                    perror("Failed to change directory\n");
                }
        } else if(strcmp(tokens[0], "stop") == 0) {
            int jobToStop = atoi(tokens[1]);
            struct process process = processes[jobToStop];
            if(process.running == 0) {
                printf("The job you have specified is already stopped\n");
                continue;
            } else {
                int status = kill(process.pid, SIGSTOP);
                if(status < 0) {
                    printf("Failed to stop process\n");
                    continue;
                } else {
                    processes[jobToStop].running = 0;
                }
            }
        } else if(strcmp(tokens[0], "start") == 0) {
            int jobToStart = atoi(tokens[1]);
            struct process process = processes[jobToStart];
            if(process.running == 1) {
                printf("The job you have specified is already running\n");
                continue;
            } else {
                int status = kill(process.pid, SIGCONT);
                if(status < 0) {
                    printf("Failed to restart process");
                    continue;
                } else {
                    processes[jobToStart].running = 1;
                }
            }
        } else if(strcmp(tokens[0], "bgkill") == 0) {
            if(tokens[1] == NULL) {
                printf("Incorrect arguments passed. bgkill requires a process number to kill\n");
                continue;
            } else if(atoi(tokens[1]) >= jobCount) {
                printf("The job you have specified with index %d does not exist\n there are %d jobs running\n", atoi(tokens[1]), jobCount);
                continue;
            }
            int jobToKill = atoi(tokens[1]);
            if(jobToKill > 4) {
                printf("ERROR: Job specified is greater than the maximum number of jobs\n");
                continue;
            }
            int killStatus = kill(processes[jobToKill].pid, SIGKILL);
            if(killStatus < 0) {
                printf("Failed to kill job\n");
            }
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
                    pid_t anotherpid;
                    anotherpid = wait(&status);
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

