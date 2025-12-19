#ifndef PROMPT_H        // If PROMPT_H is NOT defined yet
#define PROMPT_H        // if not then define it now, and compile everything until the #endif
// PROMPT_H ==> macro name to prevent declaration of header files more than once

#include <limits.h> // Required for PATH_MAX

// function declaration
void print_prompt();

extern char shell_home_dir[PATH_MAX];

#endif                 