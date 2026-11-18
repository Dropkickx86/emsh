#include <unistd.h>

#include "general.h"
#include "../emsh.h"
#include "../environment.h"

// Internal flags
struct FlagRegister flag = {};

// Options
struct ShellOpts option = {};

int platform_init(){ // "Main" function
    sh_vars_populate();
    sh_vars_init();
    // TODO: remove hardcoding when argument handling is implemented
    option.vi = true;
    return 0;
}