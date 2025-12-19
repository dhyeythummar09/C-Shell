#include "parser.h"
#include "executor.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// ############## LLM Generated Code Begins ##############
#define MAX_HISTORY 15
static char *history[MAX_HISTORY];
static int history_count = 0;

void add_to_history(const char *input){
    if(input==NULL) return;   
    if(history_count > 0 && strcmp(history[history_count - 1], input) == 0){
        return;
    }
    if(history_count<MAX_HISTORY){
        history[history_count]=strdup(input);
        history_count++;    
    } else {
        free(history[0]);
        memmove(&history[0], &history[1], (MAX_HISTORY - 1) * sizeof(char *));
        history[MAX_HISTORY - 1] = strdup(input);
    }
}
// ############## LLM Generated Code Ends ################

char **get_history(){ 
    return history; 
}
int get_history_count(){
    return history_count; 
}
void free_history(){
    for(int i=0;i<history_count;i++)   free(history[i]);
}


// function to tokenize and execute a single (self contained) command
void execute_single_command(char* cmd_str) {
    // trim leading spaces
    while(isspace(*cmd_str))    cmd_str++;

    if(strlen(cmd_str) == 0) return;

    //  History Handling 
    // Check if the first word is 'log' before adding to history
    char temp[1024];
    strncpy(temp, cmd_str, sizeof(temp) - 1);
    temp[sizeof(temp)-1]='\0';
    char* first_word = strtok(temp, " \t\n");
    if(!first_word || strcmp(first_word,"log")!=0){
        add_to_history(cmd_str);
    }
    
    // Tokenize 
    char *tokens[100];
    int i = 0;
    char *saveptr;
    char *token = strtok_r(cmd_str, " \t\n", &saveptr);
    while(token && i < 99){
        tokens[i++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    tokens[i] = NULL;

    // Execute 
    if (tokens[0] != NULL) {
        execute(tokens);
    }
}

// to parse the command
int parse_command(char *input){
    // strip trailing newline
    input[strcspn(input,"\n")]='\0';
    
    char *full_input_copy = strdup(input); // for history logging
    
    // Syntax Validation
    // check for invalid sequences like "| ;"
    char* invalid_seq1 = strstr(input, "|;");
    char* invalid_seq2 = strstr(input, "| ;");
    if (invalid_seq1 || invalid_seq2) {
        printf("Invalid Syntax!\n");
        free(full_input_copy);
        return 0;
    }
    
    // main parsing loop now handles both ';' and '&' as top-level separators
    char *current = input;
    while (*current != '\0') {
        // find the next separator (';' or '&')
        char *separator = strpbrk(current,";&");
        
        char *cmd_str;
        int is_background = 0;

        if(separator != NULL){
            if(*separator == '&'){
                is_background = 1;
            }
            *separator = '\0'; // terminate the current command string
            cmd_str = current;
            current = separator + 1;
        } 
        else{
            cmd_str=current;
            current += strlen(current); // move to the end of the string
        }
        
        // Add the '&' back as a token if it's a background command
        if(is_background){
            char temp_cmd[1024];
            snprintf(temp_cmd, sizeof(temp_cmd), "%s &", cmd_str);
            execute_single_command(temp_cmd);
        } 
        else{
            execute_single_command(cmd_str);
        }
    }

    free(full_input_copy);
    return 1;
}


// ############## LLM Generated Code Begins ##############
// helper for replay: tokenize without execution
char **parse_command_from_string(const char *input){
    char *line = strdup(input);
    line[strcspn(line,"\n")]='\0';

    char **tokens=malloc(100 * sizeof(char*));
    int i=0;
    
    char *saveptr;
    char *token=strtok_r(line, " \t\n", &saveptr);
    while((token) && (i<99)){
        tokens[i++]=strdup(token);
        token=strtok_r(NULL, " \t\n", &saveptr);
    }
    tokens[i]=NULL;

    free(line);
    return tokens;
}
// ############## LLM Generated Code Ends ##############