#define _POSIX_C_SOURCE 200809L     
#include "prompt.h"
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

void print_prompt(){
    char hostname[_POSIX_HOST_NAME_MAX + 1];    // array for storing hostname
    char cwd[PATH_MAX];                         // current working directory
    char display_path[PATH_MAX];
    


    // fetching user info
    struct passwd *pw = getpwuid(getuid());             // get user info from user's id
    char *username = pw ? pw->pw_name : "user";         // get username  
    gethostname(hostname,sizeof(hostname));            // get hostname
    getcwd(cwd,sizeof(cwd));                           // get the home directory path from pw struct



// ############## LLM Generated Code Begins ##############
    // check if cwd starts with the home directory path
    if(strcmp(cwd, shell_home_dir)==0){
        // If the current directory is the shell's home, just show "~"
        snprintf(display_path, sizeof(display_path), "~");
    } 
    else if(strncmp(cwd, shell_home_dir, strlen(shell_home_dir)) == 0) {
        // If the current directory is a SUBDIRECTORY of shell's home, show "~/subdir"
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(shell_home_dir));
    }
    else{
        // Otherwise show the full absolute path
        snprintf(display_path, sizeof(display_path),"%s",cwd);
    }
// ############## LLM Generated Code Ends ##############

    // display the final prompt
    printf("<%s@%s:%s> ", username, hostname, display_path);
    fflush(stdout);
}

