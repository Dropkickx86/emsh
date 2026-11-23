#ifndef GENERAL_H
#define GENERAL_H

// Typedef for function pointers
typedef int (*fptr)(unsigned int, char**);

// Read input from user  
// Returns number of arguments and populates in_v with arguments  
// in_v must be pre-allocated on function call, but can be reallocated to increase size
char **input_read(char **in_v, unsigned int *in_c);
// Initialize platform specific settings at start of program  
// Returns 0 on success and -1 on failure
int platform_init();
// Run executable defined by argv[0] with argc amount of arguments inluding executed program  
// Returns 0 on success and -1 on failure
int program_exec(unsigned int argc, char **argv);

#endif