#include <unistd.h>

#include "general.h"
#include "../emsh.h"
#include "../environment.h"

// Internal flags
FlagRegister flag = {};

// Options
ShellOpts option = {};

// Shell variables
ShVarsShortcut shell_var_short = {};

int platform_init(){ // "Main" function
    sh_vars_populate();
    sh_vars_init();
    // TODO: remove hardcoding when argument handling is implemented
    option.vi = true;
    return 0;
}