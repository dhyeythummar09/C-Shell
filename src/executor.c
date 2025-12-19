#include "executor.h"
#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <strings.h>
#include <termios.h> 

// Make shell_home_dir accessible here
extern char shell_home_dir[PATH_MAX];

// Global variables
static char previous_cwd[PATH_MAX] = "";
static int has_previous_cwd = 0;

BackgroundJob bg_jobs[MAX_BG_JOBS];
static int next_job_id = 1;
pid_t g_foreground_pid = -1;

// ############## LLM Generated Code Begins ##############
//  Helper Functions 
int is_builtin(char* cmd) {
    if (!cmd) return 0;
    if (strcmp(cmd, "hop") == 0 || strcmp(cmd, "reveal") == 0 ||
        strcmp(cmd, "log") == 0 || strcmp(cmd, "replay") == 0 ||
        strcmp(cmd, "activities") == 0 || strcmp(cmd, "ping") == 0 ||
        strcmp(cmd, "fg") == 0 || strcmp(cmd, "bg") == 0){
        return 1;
    }
    return 0;
}
// ############## LLM Generated Code Ends ##############

// Forward declaration
int execute_command(char **tokens);


// Job Control Functions 
void handle_activities(){
    BackgroundJob active_jobs[MAX_BG_JOBS];
    int count=0;
    for(int i=0;i<MAX_BG_JOBS;i++){
        if (bg_jobs[i].pid != 0){
            active_jobs[count++]=bg_jobs[i];
        }
    }

    // bubble sort for lexicographical order
    for(int i=0;i<count-1;i++) {
        for(int j=0;j<count-i-1;j++){
            if(strcmp(active_jobs[j].command_name, active_jobs[j + 1].command_name) > 0) {
                BackgroundJob temp = active_jobs[j];
                active_jobs[j] = active_jobs[j + 1];
                active_jobs[j+1] = temp;
            }
        }
    }

    // print all the jobs
    for(int i=0;i<count;i++){
        printf("[%d]: %s - %s\n",
               active_jobs[i].pid,
               active_jobs[i].command_name,
               active_jobs[i].state == RUNNING ? "Running" : "Stopped");
    }
}

// ############## LLM Generated Code Begins ##############
void handle_ping(char **tokens) {
    if (tokens[1] == NULL || tokens[2] == NULL) {
        fprintf(stderr, "ping: missing arguments\n");
        return;
    }
    pid_t pid = atoi(tokens[1]);
    int sig_num = atoi(tokens[2]) % 32;
    if (kill(pid, sig_num) == -1) {
        if (errno == ESRCH) {
            fprintf(stderr, "No such process found\n");
        } else {
            perror("ping");
        }
    } else {
        printf("Sent signal %d to process with pid %d\n", sig_num, pid);
    }
}

// reaps any background process that has terminated
void reap_background_processes(){
    int status;
    pid_t pid;
    for(int i = 0; i < MAX_BG_JOBS; i++){
        if(bg_jobs[i].pid != 0){ // Check any active job slot, not just RUNNING ones
            pid=waitpid(bg_jobs[i].pid, &status, WNOHANG);
            if(pid > 0) { // If waitpid returns a PID, the process has terminated
                if(WIFEXITED(status)){
                    fprintf(stderr, "\n%s with pid %d exited normally\n", bg_jobs[i].full_command, pid);
                } 
                else if (WIFSIGNALED(status)) {
                    fprintf(stderr, "\n%s with pid %d exited abnormally\n", bg_jobs[i].full_command, pid);
                }
                bg_jobs[i].pid = 0; // free the slot
            }
        }
    }
}
// ############## LLM Generated Code Ends ##############

void initialize_jobs(){
    for (int i=0;i<MAX_BG_JOBS;i++){
        bg_jobs[i].pid=0;
    }
}

BackgroundJob* find_job_by_id(int job_id) {
    for(int i=0;i<MAX_BG_JOBS;i++) {
        if((bg_jobs[i].pid != 0) && (bg_jobs[i].job_id == job_id)){
            return &bg_jobs[i];
        }
    }
    return NULL;            // if no such job found
}

BackgroundJob* find_most_recent_job() {
    int max_job_id=-1;
    BackgroundJob* recent_job = NULL;
    for (int i=0;i<MAX_BG_JOBS;i++) {
        if ((bg_jobs[i].pid != 0) && (bg_jobs[i].job_id > max_job_id)){
            max_job_id = bg_jobs[i].job_id;
            recent_job = &bg_jobs[i];
        }
    }
    return recent_job;
}

// ############## LLM Generated Code Begins ##############
void wait_for_pid(pid_t pid, const char* cmd_name, const char* full_command){
    int status;
    g_foreground_pid = pid;
   
    if (waitpid(pid, &status, WUNTRACED) > 0) {
        if (WIFSTOPPED(status)) {
            for (int i = 0; i < MAX_BG_JOBS; i++) {
                if (bg_jobs[i].pid == 0) {
                    bg_jobs[i].pid = pid;
                    bg_jobs[i].job_id = next_job_id++;
                    strncpy(bg_jobs[i].command_name, cmd_name, sizeof(bg_jobs[i].command_name) -1);
                    bg_jobs[i].command_name[sizeof(bg_jobs[i].command_name) - 1] = '\0';
                    strncpy(bg_jobs[i].full_command, full_command, sizeof(bg_jobs[i].full_command) -1);
                    bg_jobs[i].full_command[sizeof(bg_jobs[i].full_command) - 1] = '\0';
                    bg_jobs[i].state = STOPPED;
                    fprintf(stderr, "\n[%d] Stopped %s\n", bg_jobs[i].job_id, bg_jobs[i].command_name);
                    break;
                }
            }
        }
    }
    g_foreground_pid = -1;
}
// ############## LLM Generated Code Ends ##############


void handle_bg(char **tokens){
    if(tokens[1]==NULL){ 
        fprintf(stderr, "bg: missing job number\n");
        return; 
    }
    BackgroundJob* job = find_job_by_id(atoi(tokens[1]));
    if(job==NULL){ 
        fprintf(stderr, "No such job\n"); 
        return; 
    }
    if(job->state == RUNNING){
        fprintf(stderr, "Job already running\n"); return; 
    }
    if(kill(job->pid, SIGCONT) < 0){ 
        perror("bg: kill"); return; 
    }
    job->state = RUNNING;
    fprintf(stderr, "[%d] %s &\n", job->job_id, job->command_name);
}

void handle_fg(char **tokens){
    BackgroundJob* job=NULL;
    if(tokens[1]==NULL){
        job=find_most_recent_job();
    } 
    else{
        job=find_job_by_id(atoi(tokens[1]));
    }
    if(job==NULL){
        fprintf(stderr, "No such job\n"); 
        return; 
    }
    pid_t pid = job->pid;
    char cmd_name[1024];
    char full_command[1024];
    strcpy(cmd_name, job->command_name);
    strcpy(full_command, job->full_command);


    if(job->state == STOPPED){
        if(kill(pid, SIGCONT) < 0){
            perror("fg: kill");
            return;
        }
    }
    printf("%s\n", cmd_name);
   
    job->pid=0;
    wait_for_pid(pid, cmd_name, full_command);
}

// Main Execution Logic
// ############## LLM Generated Code Begins ##############
int execute_command(char **tokens){
    if(tokens[0]==NULL){
        return 1;
    }
   
    int saved_stdout = -1;
    char* outfile = NULL;
    int append = 0;

    if (is_builtin(tokens[0])) {
        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], ">") == 0) {
                outfile = tokens[i+1];
                append = 0;
            } else if (strcmp(tokens[i], ">>") == 0) {
                outfile = tokens[i+1];
                append = 1;
            }
        }
        if (outfile) {
            saved_stdout = dup(STDOUT_FILENO);
            int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
            int fd = open(outfile, flags, 0644);
            if (fd < 0) {
                fprintf(stderr, "Unable to create file for writing\n");
                return 1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            for (int i = 0; tokens[i] != NULL; i++) {
                if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0) {
                    tokens[i] = NULL;
                    break;
                }
            }
        }
    }

    if(strcmp(tokens[0],"hop") == 0){
        if (tokens[1] == NULL) {
            if (getcwd(previous_cwd, sizeof(previous_cwd)) != NULL) has_previous_cwd = 1;
            if (chdir(shell_home_dir) != 0) fprintf(stderr, "No such directory!\n");
        } else {
            for (int i = 1; tokens[i] != NULL; i++) {
                char current_dir_before_hop[PATH_MAX];
                if (getcwd(current_dir_before_hop, sizeof(current_dir_before_hop)) == NULL) {
                    perror("getcwd"); break;
                }
                char* target_path = tokens[i];
                if (strcmp(tokens[i], "~") == 0) target_path = shell_home_dir;
                else if (strcmp(tokens[i], "-") == 0) {
                    if (!has_previous_cwd) { continue; }
                    target_path = previous_cwd;
                }
                if (chdir(target_path) != 0) {
                    fprintf(stderr, "No such directory!\n"); break;
                }
                strcpy(previous_cwd, current_dir_before_hop);
                has_previous_cwd = 1;
            }
        }
    }
    else if(strcmp(tokens[0] ,"reveal") == 0){
        int show_all = 0, long_format = 0;
        char *path = ".";
        int path_args = 0;
       
        for (int i = 1; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "-") != 0 && tokens[i][0] == '-') {
                for (int j = 1; tokens[i][j] != '\0'; j++) {
                    if (tokens[i][j] == 'a') show_all = 1;
                    if (tokens[i][j] == 'l') long_format = 1;
                }
            } else {
                path_args++;
                path = tokens[i];
            }
        }

        if (path_args > 1) {
            fprintf(stderr, "reveal: Invalid Syntax!\n");
        } else {
            if (strcmp(path, "-") == 0) {
                if (!has_previous_cwd) {
                    fprintf(stderr, "No such directory!\n");
                    if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
                    return 1;
                } else {
                    path = previous_cwd;
                }
            }

            DIR *dir = opendir(path);
            if(dir == NULL){
                fprintf(stderr, "No such directory!\n");
            } else {
                struct dirent *entry;
                char *entries[2048];
                int count = 0;

                while((entry = readdir(dir)) != NULL && count < 2048) {
                    if (!show_all && entry->d_name[0] == '.') continue;
                    entries[count++] = strdup(entry->d_name);
                }
                closedir(dir);

                for (int i = 0; i < count - 1; i++) {
                    for (int j = 0; j < count - i - 1; j++) {
                        if (strcmp(entries[j], entries[j + 1]) > 0) {
                            char* temp = entries[j];
                            entries[j] = entries[j+1];
                            entries[j+1] = temp;
                        }
                    }
                }

                for (int i = 0; i < count; i++) {
                    printf("%s%s", entries[i], long_format ? "\n" : "  ");
                    free(entries[i]);
                }
                if (!long_format && count > 0) printf("\n");
            }
        }
    }   
    else if(strcmp(tokens[0],"log") == 0){
        char **history=get_history();
        int history_count=get_history_count();
        for(int i=0;i<history_count;i++){
            printf("%s\n", history[i]);
        }
    }
    else if(strcmp(tokens[0], "replay") == 0){
        if(tokens[1] == NULL){
            fprintf(stderr, "replay: missing argument\n");
        } else {
            int index = atoi(tokens[1]);
            char **history = get_history();
            int history_count = get_history_count();
            if(index <= 0 || index > history_count){
                fprintf(stderr, "replay: invalid index\n");
            } else {
                char *cmd = history[index - 1];
                printf("Replaying command: %s\n", cmd);
                char *cmd_copy = strdup(cmd);
                parse_command(cmd_copy);
                free(cmd_copy);
            }
        }
    }
    else if(strcmp(tokens[0], "activities")==0){ handle_activities(); }
    else if (strcmp(tokens[0], "ping") == 0) { handle_ping(tokens); }
    else if (strcmp(tokens[0], "fg") == 0) { handle_fg(tokens); }
    else if (strcmp(tokens[0], "bg") == 0) { handle_bg(tokens); }
    else{ 
        // External commands 
        int background = 0;
        int i;
        for(i=0; tokens[i]!=NULL; i++);
        if(i>0 && strcmp(tokens[i-1], "&")==0){
            background = 1;
            tokens[i-1] = NULL;
        }

        char full_command_str[1024] = "";
        for(int k=0; tokens[k] != NULL; k++) {
            strcat(full_command_str, tokens[k]);
            if(tokens[k+1] != NULL) strcat(full_command_str, " ");
        }

        pid_t pid=fork();
        if(pid==0){ 
            // Child process
            // Set the child in its own process group
            setpgid(0, 0);
            
            // For foreground processes, reset signal handlers to default
            if (!background) {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
            }
            
            if (background) {
                int fd = open("/dev/null", O_RDONLY);
                if (fd != -1) {
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            }

            for (int j=0;tokens[j]!=NULL;j++){
                if(strcmp(tokens[j],">")==0){
                    int fd=open(tokens[j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd<0){ fprintf(stderr, "Unable to create file for writing\n"); exit(1); }
                    dup2(fd,STDOUT_FILENO);
                    close(fd);
                    tokens[j]=NULL;
                } else if(strcmp(tokens[j],">>") == 0){
                    int fd=open(tokens[j+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if(fd<0){ fprintf(stderr, "Unable to create file for writing\n"); exit(1); }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    tokens[j]=NULL;
                } else if (strcmp(tokens[j], "<") == 0){
                    int fd=open(tokens[j+1], O_RDONLY);
                    if(fd<0){ fprintf(stderr, "No such file or directory\n"); exit(1); }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    tokens[j]=NULL;
                }
            }
            execvp(tokens[0],tokens);
            fprintf(stderr, "Command not found!\n");
            exit(1);
        } 
        else if(pid>0){ 
            // Parent process
            // Set the child's process group
            setpgid(pid, pid);
            
            if(!background)
            {
                // Give terminal control to the child process group
                tcsetpgrp(STDIN_FILENO, pid);
                wait_for_pid(pid, tokens[0], full_command_str);
                // Take terminal control back to the shell
                tcsetpgrp(STDIN_FILENO, getpgrp());
            } 
            else{
                char full_command_with_ampersand[1024];
                strncpy(full_command_with_ampersand, full_command_str, sizeof(full_command_with_ampersand) - 1);
                full_command_with_ampersand[sizeof(full_command_with_ampersand) - 1] = '\0';
                strncat(full_command_with_ampersand, " &", sizeof(full_command_with_ampersand) - strlen(full_command_with_ampersand) - 1);

                for (int j = 0; j < MAX_BG_JOBS; j++) {
                    if (bg_jobs[j].pid == 0) {
                        bg_jobs[j].pid = pid;
                        bg_jobs[j].job_id = next_job_id++;
                        strncpy(bg_jobs[j].command_name, tokens[0], sizeof(bg_jobs[j].command_name) - 1);
                        bg_jobs[j].command_name[sizeof(bg_jobs[j].command_name) - 1] = '\0';
                        strncpy(bg_jobs[j].full_command, full_command_with_ampersand, sizeof(bg_jobs[j].full_command) - 1);
                        bg_jobs[j].full_command[sizeof(bg_jobs[j].full_command) - 1] = '\0';
                        bg_jobs[j].state = RUNNING;
                        fprintf(stderr, "[%d] %d\n", bg_jobs[j].job_id, bg_jobs[j].pid);
                        break;
                    }
                }
            }
        } 
        else{
            perror("fork");
        }
    }
   
    if(saved_stdout != -1){
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }

    return 1;
}


// PIPE SUPPORT
int execute_pipeline(char ***cmds,int n){
    int in_fd = STDIN_FILENO;
    pid_t pids[n];

    for(int i = 0; i < n; i++){
        int pipefd[2];
        if (i < n - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return 1;
            }
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return 1;
        }

        if (pids[i] == 0){ 
            // Child process
            if (i > 0) { // If not the first command, redirect stdin
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (i < n - 1) { 
                // If not the last command, redirect stdout
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (is_builtin(cmds[i][0])) {
                execute_command(cmds[i]);
                exit(0);
            } 
            else{
                // Redirection for external commands within a pipe
                for (int j=0;cmds[i][j]!=NULL;j++){
                    if(strcmp(cmds[i][j],">")==0){
                        int fd=open(cmds[i][j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(fd<0){ fprintf(stderr, "Unable to create file for writing\n"); exit(1); }
                        dup2(fd,STDOUT_FILENO); close(fd); cmds[i][j] = NULL;
                    } else if(strcmp(cmds[i][j], ">>") == 0){
                        int fd=open(cmds[i][j+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if(fd<0){ fprintf(stderr, "Unable to create file for writing\n"); exit(1); }
                        dup2(fd, STDOUT_FILENO); close(fd); cmds[i][j] = NULL;
                    } else if(strcmp(cmds[i][j], "<") == 0){
                        int fd=open(cmds[i][j+1], O_RDONLY);
                        if(fd<0){ fprintf(stderr, "No such file or directory\n"); exit(1); }
                        dup2(fd, STDIN_FILENO); close(fd); cmds[i][j] = NULL;
                    }
                }
                execvp(cmds[i][0], cmds[i]);
                fprintf(stderr, "Command not found!\n");
                exit(1);
            }
        }

        // Parent process
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }
        if (i < n - 1) {
            close(pipefd[1]);
            in_fd = pipefd[0];
        }
    }

    for (int i = 0; i < n; i++) {
        char full_command[1024] = "";
        for(int k=0; cmds[i][k] != NULL; k++){
            strcat(full_command, cmds[i][k]);
            if(cmds[i][k+1] != NULL) strcat(full_command, " ");
        }
        wait_for_pid(pids[i], cmds[i][0], full_command);
    }
    
    return 1;
}
// ############## LLM Generated Code Ends ##############


// any command enters this function first for execution
int execute(char **tokens){
    int n=1;
    for(int i=0;tokens[i]!= NULL;i++){
        if(strcmp(tokens[i],"|")==0)    n++;
    }
    if(n==1){ 
        return execute_command(tokens); 
    }
    char ***cmds=malloc((n+1)*sizeof(char**));
    if(cmds== NULL){
        perror("malloc"); return 1; 
    }
    int cmd_index=0;
    cmds[cmd_index]=tokens;
    for(int i=0;tokens[i]!=NULL;i++) {
        if(strcmp(tokens[i],"|")==0){
            tokens[i]=NULL;
            cmd_index++;
            if(tokens[i+1]!=NULL){
                cmds[cmd_index]=&tokens[i+1];
            }
        }
    }
    cmds[cmd_index+1]=NULL;
    int result=execute_pipeline(cmds,n);
    free(cmds);
    return result;
}