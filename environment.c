#include <errno.h>
#include <linux/limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "emsh.h"
#include "environment.h"
#include "emgc.h"
#include "helpers.h"

ShVar *shell_vars;

ShVar *sh_var_get(const char *name){
    for (int i = 0; strlen(shell_vars[i].name) > 0; i++){
        if (strcmp(name, shell_vars[i].name) == 0){
            return &shell_vars[i];
        }
    }
    return nullptr;
}

int sh_var_set(const char *name, const char *value){
    ShVar *var_cur = sh_var_get(name);
    if (var_cur && var_cur->ro){
        fprintf(stderr, "emsh: %s: variable is read only", name);
        return -1;
    }
    if (!var_cur){
        int index = -1;
        while (strlen(shell_vars[++index].name) > 0);
        if ((unsigned int) index == (gc_get_alloc_size(shell_vars) / sizeof(ShVar)) - 1){
            shell_vars = gc_realloc(shell_vars, gc_get_alloc_size(shell_vars) * 2);
        }
        shell_vars[index + 1].name = "";
        var_cur = &shell_vars[index];
        var_cur->name = string_copy(name, 'p');
        var_cur->export = false;
        var_cur->ro = false;
        var_cur->protected = false;
    }
    var_cur->value = string_insert(var_cur->value, value, 0, strlen(value), true, 'p');
    if (var_cur->export){
        setenv(name, value, 1);
    }
    return 0;
}

int sh_var_protect(char *name){
    ShVar *tmp = sh_var_get(name);
    if (!tmp){
        return -1;
    }
    tmp->protected = true;
    return 0;
}

int sh_vars_populate(){
    if (!shell_vars){
        shell_vars = gc_malloc('p', 32 * sizeof(ShVar));
        shell_vars[0].name = "";
    }
    if (!environ){
        return 0;
    }
    short index = 0;
    char **name_val = gc_malloc('i', 2 * sizeof(char*));
    if (!name_val){
        return -1;
    }
    char **environ_local = environ;
    // Read and save environment variables
    while (*environ_local){
        int token_amount;
        name_val = string_tokenize(name_val, &token_amount, *environ_local, "=", 'i');
        environ_local++;
        // Check if accepted variable
        if (token_amount != 2 || name_val[0][0] < '\101' || (name_val[0][0] > '\132' && name_val[0][0] != '\137')){
            continue;
        }
        for (size_t i = 1; i < strlen(name_val[0]); i++){
            if ((name_val[0][i] < '\060' || name_val[0][i] > '\071') && (name_val[0][0] < '\101' || name_val[0][0] > '\132') && name_val[0][i] != '\137'){
                token_amount = 0;
                break;
            }
        }
        if (token_amount == 0){
            continue;
        }
        // Save variable
        if ((unsigned int) index == (gc_get_alloc_size(shell_vars) / sizeof(ShVar)) - 1){
            shell_vars = gc_realloc(shell_vars, gc_get_alloc_size(shell_vars) * 2);
        }
        shell_vars[index].name = string_copy(name_val[0], 'p');
        shell_vars[index].value = string_copy(name_val[1], 'p');
        shell_vars[index].export = true;
        shell_vars[index].ro = false;
        shell_vars[index].protected = false;
        index++;
    }
    shell_vars[index].name = "";
    return 0;
}

void sh_vars_init(){
    // Create shell_vars if nonexistent
    if (!shell_vars){
        shell_vars = gc_malloc('p', 32 * sizeof(ShVar));
        shell_vars[0].name = "";
    }
    // Link short options for future use
    if ((shell_var_short.ENV = sh_var_ptr_link("ENV"))){
        sh_var_protect("ENV");
    }
    if ((shell_var_short.HOME = sh_var_ptr_link("HOME"))){
        sh_var_protect("HOME");
    }
    if ((shell_var_short.LANG = sh_var_ptr_link("LANG"))){
        sh_var_protect("LANG");
    }
    if ((shell_var_short.LC_ALL = sh_var_ptr_link("LC_ALL"))){
        sh_var_protect("LC_ALL");
    }
    if ((shell_var_short.LC_COLLATE = sh_var_ptr_link("LC_COLLATE"))){
        sh_var_protect("LC_COLLATE");
    }
    if ((shell_var_short.LC_CTYPE = sh_var_ptr_link("LC_CTYPE"))){
        sh_var_protect("LC_CTYPE");
    }
    if ((shell_var_short.LC_MESSAGES = sh_var_ptr_link("LC_MESSAGES"))){
        sh_var_protect("LC_MESSAGES");
    }
    if ((shell_var_short.NLSPATH = sh_var_ptr_link("NLSPATH"))){
        sh_var_protect("NLSPATH");
    }
    if ((shell_var_short.PATH = sh_var_ptr_link("PATH"))){
        sh_var_protect("PATH");
    }
    // IFS fill
    if (sh_var_set("IFS", " \t\n") == 0){
        if ((shell_var_short.IFS = sh_var_ptr_link("IFS"))){
            sh_var_protect("IFS");
        }
    }
    // LINENO init
    if (sh_var_set("LINENO", "") == 0){
        if ((shell_var_short.LINENO = sh_var_ptr_link("LINENO"))){
            sh_var_protect("LINENO");
        }
    }
    // PPID fill
    pid_t ppid = getppid();
    char *s_ppid = gc_malloc('i', (int)((ceil(log10(ppid))+1)*sizeof(char)));
    sprintf(s_ppid, "%d", ppid);
    if (sh_var_set("PPID", s_ppid) == 0){
        if ((shell_var_short.PPID = sh_var_ptr_link("PPID"))){
            sh_var_protect("PPID");
        }
    }
    // PS1 fill
    char *tmp;
    if (geteuid() == 0){
        tmp = string_copy("# ", 'p');
    }
    else {
        tmp = string_copy("$ ", 'p');
    }
    if (sh_var_set("PS1", tmp) == 0){
        if ((shell_var_short.PS1 = sh_var_ptr_link("PS1"))){
            sh_var_protect("PS1");
        }
    }
    // PS2 fill
    if (sh_var_set("PS2", "> ") == 0){
        if ((shell_var_short.PS2 = sh_var_ptr_link("PS2"))){
            sh_var_protect("PS2");
        }
    }
    // PS4 fill
    if (sh_var_set("PS4", "+ ") == 0){
        if ((shell_var_short.PS4 = sh_var_ptr_link("PS4"))){
            sh_var_protect("PS4");
        }
    }
    // PWD fill/validate
    char *cwd = gc_malloc('i', 16 * sizeof(char));
    do {
        cwd = gc_realloc(cwd, gc_get_alloc_size(cwd) * 2);
        errno = 0;
        cwd = getcwd(cwd, gc_get_alloc_size(cwd));
    } while (!cwd && errno == ERANGE);
    shell_var_short.PWD = sh_var_ptr_link("PWD");
    if (!cwd){
        fprintf(stderr, "emsh: could not get cwd\nvalue of PWD is not verified\n");
    }
    else if (shell_var_short.PWD && strlen(shell_var_short.PWD) == 0){
        sh_var_set("PWD", cwd);
        sh_var_protect("PWD");
    }
    else if (shell_var_short.PWD){
        sh_var_protect("PWD");
        bool invalid = false;
        if (shell_var_short.PWD[0] != '/' || strlen(shell_var_short.PWD) > PATH_MAX){
            invalid = true;
        }
        if (!invalid){
            for (int i = 0; i < (int)strlen(shell_var_short.PWD); i++){
                if (shell_var_short.PWD[i] == '.' && shell_var_short.PWD[i + 1] == '/'){
                    if ((shell_var_short.PWD[i - 1] == '.' && shell_var_short.PWD[i - 2] == '/') || shell_var_short.PWD[i - 1] == '/'){
                        invalid = true;
                        break;
                    }
                }
            }
        }
        if (!invalid){
            struct stat s_cwd, s_pwd;
            if (stat(cwd, &s_cwd) == -1){
                fprintf(stderr, "emsh: could not stat cwd\nvalue of PWD is not verified\n");
            }
            else if (stat(shell_var_short.PWD, &s_pwd) == -1){
                fprintf(stderr, "emsh: could not stat PWD\nvalue of PWD is not verified\n");
            }
            else if (s_cwd.st_dev != s_pwd.st_dev || s_cwd.st_ino != s_pwd.st_ino){
                invalid = true;
            }
        }
        if (invalid){
            if (sh_var_set("PWD", cwd) == -1){
                fprintf(stderr, "emsh: could not update PWD\nvalue of PWD is invalid\n");
            }
        }
    }
}