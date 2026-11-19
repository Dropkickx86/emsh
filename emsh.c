/*HEADER*/

// Includes system
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <emgc.h>

// Includes local
#include "emsh.h"
#include "general/general.h"

// Enums

// Structs

// Globals

int init_loop();
int init_program();
void exit_handler();

// Initialize loop  
// Returns 0 on success, -1 on failure
int init_loop(){
    return 0;
}

// Initialize emsh  
// Returns 0 on success, -1 on failure
int init_program(){
    atexit(exit_handler);
    gc_exit_on_alloc_fail(1);
    gc_null_init_memory(1);
    if (gc_construct('i') == -1){
        return -1;
    }
    if (gc_construct('p') == -1){
        return -1;
    }
    if (gc_construct('l') == -1){
        return -1;
    }
    platform_init();
    (void) gc_destruct('i');
    return 0;
}

// Exit handler
void exit_handler(){
    gc_end();
}

// Main loop
void emsh_main(){
    while (!flag.exit && !flag.failure){
        init_loop();
        int in_c = 0;
        char **in_v = gc_malloc('l', 4 * sizeof(char*));
        in_v = input_read(in_v, &in_c);
        if (!flag.exit && in_c > 0){
            program_exec(in_c, in_v);
        }
        gc_clean('l');
    }
}

// Launch emsh
int main(){
    if (init_program() == -1){
        flag.failure = true;
    }
    emsh_main();
    if (flag.failure){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}