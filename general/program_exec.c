#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <emgc.h>

#include "general.h"
#include "../emsh.h"
#include "../helpers.h"
#include "../nls.h"
#include "../intrinsic/intrinsic.h"

int temp_exit_function(unsigned int argc, char **argv){
    (void) argc;
    (void) argv;
    flag.exit = true;
    return 0;
}

// Local  
// Choose how to interpret command  
// Returns function pointer for builtins and nullptr for unrecognized
fptr check_command(char *cmd){
    if (strcmp(cmd, "exit") == 0){
        return temp_exit_function;
    }
    if (strcmp(cmd, "cd") == 0){
        return cd;
    }
    return nullptr;
}

// Local  
// Returns pointer to path for executable arg  
// Returns nullptr on failure and empty string if executable was not found
char *path_expand(char *arg){
    long arg_len = strlen(arg);
    char *path = nullptr;
    // Check if already path
    for (long i = arg_len; i >= 0; i--){
        if (arg[i] == '/'){
            path = string_copy(arg, 'l');
            arg = string_copy(arg + i + 1, 'l');
            return path;
        }
    }
    // Check PATH for executable
    char *env_path = getenv("PATH");
    char **env_path_token = gc_malloc('l', 8 * sizeof(char*));
    unsigned int env_path_c;
    env_path_token = string_tokenize(env_path_token, &env_path_c, env_path, ":", 'l');
    if (env_path_c == 0){
        path = gc_malloc('l', sizeof(char));
        path[0] = '\0';
        return path;
    }
    for (unsigned int i = 0; i < env_path_c; i++){
        char *fullpath = string_copy(env_path_token[i], 'l');
        if (fullpath[strlen(fullpath) - 1] != '/'){
            string_concat(fullpath, "/", 'l');
        }
        string_concat(fullpath, arg, 'l');
        struct stat sb;
        if (stat(fullpath, &sb) == -1){
            continue;
        }
        if (!S_ISREG(sb.st_mode)){
            continue;
        }
        path = string_copy(fullpath, 'l');
        break;
    }
    // Return empty path if executable not found
    if (!path){
        path = gc_malloc('l', sizeof(char));
        path[0] = '\0';
    }
    return path;
}

int program_exec(unsigned int argc, char **argv){ // "Main" function
    if (argc <= 0 || !argv || !argv[0]){
        nls_message(3, "emsh: Invalid value for internal function");
        return -1;
    }
    int (*cmd)(unsigned int, char**) = check_command(argv[0]);
    if (cmd){
        return (*cmd)(argc, argv);
    }
    char *path = path_expand(argv[0]);
    if (strlen(path) == 0){
        nls_message(4, "emsh: Not found");
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1){
        perror("emsh");
        flag.failure = true;
        return -1;
    }
    if (pid == 0){ //child
        execv(path, (char *const *) argv);
        perror("emsh");
        _exit(EXIT_FAILURE);
    }
    else { //parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)){
            return WEXITSTATUS(status);
        }
    }
    return 0;
}