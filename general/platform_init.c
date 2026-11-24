#include <signal.h>
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

    if (option.interactive){
        sigint_handling();
        signal_ignore(SIGQUIT);
        signal_ignore(SIGTERM);
        // IMPROVEMENT: check argument -m instead
        if (true){
            signal_ignore(SIGTTIN);
            signal_ignore(SIGTTOU);
            signal_ignore(SIGTSTP);
        }
    }
    // TODO: remove hardcoding when argument handling is implemented
    option.vi = true;
    return 0;
}