#ifndef EXECUTOR_H
#define EXECUTOR_H
#define MAX_BG_JOBS 20

#include <unistd.h>

// ############## LLM Generated Code Begins ##############
typedef enum {
    RUNNING,
    STOPPED
} JobState;

// Struct to hold info about a background job
typedef struct{
    pid_t pid;
    char command_name[1024];
    char full_command[1024]; // Added to store the full command
    int job_id;
    JobState state; 
}BackgroundJob;
// ############## LLM Generated Code Begins ##############

int execute_command(char **tokens);
int execute(char **tokens);
int execute_pipeline(char ***cmds, int n);
void reap_background_processes();
void initialize_jobs();
void wait_for_pid(pid_t pid, const char* cmd_name, const char* full_command); 

#endif