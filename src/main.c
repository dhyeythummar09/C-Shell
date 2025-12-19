#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // Required for getcwd
#include <limits.h>      // Required for PATH_MAX
#include "prompt.h"
#include "parser.h"
#include "executor.h"
#include <signal.h>
#include <string.h>

// Global variable to store the shell's starting directory
char shell_home_dir[PATH_MAX];

// reference the global PID from executor.c
extern pid_t g_foreground_pid;
// Reference the job list to clean up on exit
extern BackgroundJob bg_jobs[MAX_BG_JOBS];

// ############## LLM Generated Code Begins ##############

void sigint_handler(int signum) {
    if (g_foreground_pid > 0) {
        // Send SIGINT to the process group, not just the process
        kill(-g_foreground_pid, SIGINT);
    }
    // Don't print newline here - let the shell prompt handle it naturally
}

void sigtstp_handler(int signum) {
    if (g_foreground_pid > 0) {
        // Send SIGTSTP to the process group, not just the process
        kill(-g_foreground_pid, SIGTSTP);
    }
    // Don't print anything here - the executor will handle job notifications
}

// ############## LLM Generated Code Ends ##############

int main(){
    char *command=NULL;
    size_t len=0;
    ssize_t read;

    // Store the starting directory as the shell's "home"
    if(getcwd(shell_home_dir, sizeof(shell_home_dir))==NULL){
        perror("getcwd");
        return 1;
    }

//############## LLM Generated Code Begins ##############

    // Install signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    
    // Ignore SIGTTOU to prevent issues with background job control
    signal(SIGTTOU, SIG_IGN);

    // Initialize job control
    initialize_jobs();

//############## LLM Generated Code Ends ##############

    while(1){
        // reap any completed background processes
        reap_background_processes();
        
        // print shell prompt
        print_prompt();
        
        // read command from the user
        read=getline(&command, &len, stdin);
        
        // Handle Ctrl-D or EOF
        if(read==-1){
            // Kill all background processes
            for(int i=0;i<MAX_BG_JOBS;i++){
                if(bg_jobs[i].pid != 0){
                    kill(bg_jobs[i].pid, SIGKILL);
                }
            }
            printf("logout\n");
            break;
        }
        
        // Remove trailing newline if present
        if(command[read-1]=='\n'){
            command[read-1]='\0';
        }
        
        // skip empty commands
        if(strlen(command)==0){
            continue;
        }
        
        // parse and execute the command
        parse_command(command);
    }
    
    // memory cleanup
    free(command);
    free_history();
    return 0;
}


