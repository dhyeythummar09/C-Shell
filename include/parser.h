#ifndef PARSER_H
#define PARSER_H

int parse_command(char *input);
void free_history();
int get_history_count();
char **get_history();
char **parse_command_from_string(const char *input);

#endif