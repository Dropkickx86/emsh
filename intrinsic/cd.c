#include <asm-generic/errno-base.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>

#include <emgc.h>
#include "../helpers.h"
#include "../nls.h"

#include "intrinsic.h"
#include "../environment.h"
#include "../helpers.h"

// Option flags
bool opt_e = false;
char opt = 'L';

// Global pointers/vars
const struct ShVar *pwd;
bool print_pwd;

// Local  
// Parse given arguments, populate option flags, return directory operand
char *arg_resolve(int in_c, char **in_v){
    char *arg_path = nullptr;
    bool opt_stop = false;
    for (int i = 1; i < in_c; i++){
        if (!opt_stop && in_v[i][0] == '-' && strlen(in_v[i]) > 1){
            if (strcmp(in_v[i], "--") == 0){
                opt_stop = true;
                continue;
            }
            for (unsigned long j = 1; j < strlen(in_v[i]); j++){
                if (in_v[i][j] == 'L'){
                    opt = 'L';
                }
                else if (in_v[i][j] == 'P'){
                    opt = 'P';
                }
                else if (in_v[i][j] == 'e'){
                    opt_e = true;
                }
                else {
                    nls_message(2, "cd: Unsupported option");
                    return nullptr;
                }
            }
        }
        else if (!arg_path){
            arg_path = string_copy(in_v[i], 'l');
        }
    }
    if (!arg_path){
        const struct ShVar *const home = sh_var_get("HOME");
        if (!home){
            nls_message(3, "cd: Directory not specified");
            return nullptr;
        }
        arg_path = string_copy(home->value, 'l');
    }
    else if (strcmp(arg_path, "-") == 0){
        const struct ShVar *const oldpwd = sh_var_get("OLDPWD");
        if (!oldpwd){
            nls_message(4, "cd: OLDPWD not set");
            return nullptr;
        }
        arg_path = string_copy(oldpwd->value, 'l');
        print_pwd = true;
    }
    if (arg_path[0] == '\0'){
        if (print_pwd){
            nls_message(4, "cd: OLDPWD not set");
        }
        else {
            nls_message(3, "cd: Directory not specified");
        }
        return nullptr;
    }
    if (arg_path[strlen(arg_path) - 1] != '/'){
        arg_path = string_concat(arg_path, "/", 'l');
    }
    return arg_path;
}

// Local  
// Print value of curpath to stdout  
// If curpath is nullptr, PWD is printed instead
void pwd_print(){
    const struct ShVar *const pwd_local = sh_var_get("PWD");
    if (!pwd_local){
        return;
    }
    if (strchr(pwd_local->value, '\n')){
        return;
    }
    fprintf(stdout, "%s\n", pwd_local->value);
}

// Local  
// Update value of PWD and OLDPWD
int pwd_update(char *path){
    const struct ShVar *const oldpwd = sh_var_get("PWD");
    char *oldpwd_value = nullptr;
    if (oldpwd){
        oldpwd_value = string_copy(oldpwd->value, 'l');
    }
    if (opt != 'P'){
        if (sh_var_set("PWD", path) == -1){
            nls_message(5, "cd: Could not update PWD");
            return 2;
        }
        else {
            if (!oldpwd_value || sh_var_set("OLDPWD", oldpwd_value) == -1){
                nls_message(6, "cd: Could not update OLDPWD");
            }
            if (print_pwd){
                pwd_print();
            }
        }
        return 0;
    }
    char *cwd = gc_malloc('l', 16 * sizeof(char));
    do {
        cwd = gc_realloc(cwd, gc_get_alloc_size(cwd) * 2);
        errno = 0;
        cwd = getcwd(cwd, gc_get_alloc_size(cwd));
    } while (!cwd && errno == ERANGE);
    if (!cwd){
        sh_var_set("PWD", "");
        if (opt_e){
            return 1;
        }
        return 0;
    }
    if (sh_var_set("PWD", cwd) == -1){
        nls_message(5, "cd: Could not update PWD");
    }
    else {
        if (!oldpwd_value || sh_var_set("OLDPWD", oldpwd_value) == -1){
            nls_message(6, "cd: Could not update OLDPWD");
        }
        if (print_pwd){
            pwd_print();
        }
    }
    return 0;
}

// Local  
// Check if dir is relative to cwd
char *dotslash_test(char *arg_path){
    char *tmp = string_copy("./", 'l');
    tmp = string_concat(tmp, arg_path, 'l');
    errno = 0;
    DIR *dir = opendir(tmp);
    if (dir){
        closedir(dir);
    }
    if (errno == ENOENT || errno == ENOTDIR){
        return nullptr;
    }
    return tmp;
}

// Local  
// Check if dir is present in cdpath or ./
char *cdpath_resolve(char *arg_path){
    char *tmp = nullptr;
    const struct ShVar *const cdpath = sh_var_get("CDPATH");
    if (cdpath && strlen(cdpath->value) > 0){
        if (cdpath->value[0] == ':'){
            tmp = dotslash_test(arg_path);
            if (tmp){
                return tmp;
            }
        }
        char **cdpath_token = gc_malloc('l', 8 * sizeof(char*));
        int token_amount = 0;
        cdpath_token = string_tokenize(cdpath_token, &token_amount, cdpath->value, ":", 'l');
        int index_dotslash = 0;
        bool index_dotslash_found = false;
        for (unsigned long i = 1; i < strlen(cdpath->value) - 1; i++){
            if (cdpath->value[i] == ':'){
                index_dotslash++;
            }
            if (strncmp(cdpath->value + i, "::", 2) == 0){
                index_dotslash_found = true;
                break;
            }
        }
        for (int i = 0; i < token_amount; i++){
            if (index_dotslash_found && i == index_dotslash){
                tmp = dotslash_test(arg_path);
                if (tmp){
                    return tmp;
                }
            }
            tmp = string_insert(tmp, cdpath_token[i], 0, strlen(cdpath_token[i]), true, 'l');
            if (cdpath_token[i][strlen(cdpath_token[i]) - 1] != '/'){
                tmp = string_concat(tmp, "/", 'l');
            }
            tmp = string_concat(tmp, arg_path, 'l');
            DIR *dir = opendir(tmp);
            if (dir){
                closedir(dir);
            }
            if (errno == ENOENT || errno == ENOTDIR){
                continue;
            }
            print_pwd = true;
            return tmp;
        }
        if (cdpath->value[strlen(cdpath->value) - 1] == ':'){
            return dotslash_test(arg_path);
        }
    }
    else {
        return dotslash_test(arg_path);
    }
    return nullptr;
}

// Local  
// Convert path to canonical form
char *path_canonicalize(char *curpath){
    char *tmp = nullptr;
    if (curpath[0] != '/'){
        curpath = string_insert(curpath, pwd->value, 0, strlen(pwd->value), false, 'l');
        if (pwd->value[strlen(pwd->value) - 1] != '/'){
            curpath = string_insert(curpath, "/", strlen(pwd->value), 1, false, 'l');
        }
    }
    if (strncmp(curpath, "///", 3) == 0){
        for (unsigned long i = 3; i < strlen(curpath); i++){
            if (curpath[i] != '/'){
                i--;
                curpath = string_insert(curpath, curpath + i, 0, strlen(curpath + i), true, 'l');
                break;
            }
        }
    }
    for (unsigned long i = 0; i < strlen(curpath); i++){
        if (strncmp(curpath + i, "//", 2) == 0 && i > 0){
            curpath = string_insert(curpath, curpath + i + 1, i, strlen(curpath + i + 1), true, 'l');
        }
        else if (strncmp(curpath + i, "/./", 3) == 0){
            curpath = string_insert(curpath, curpath + i + 2, i, strlen(curpath + i + 2), true, 'l');
        }
        else if (strncmp(curpath + i, "/../", 4) == 0 && i != 0){
            if (strncmp(curpath + i - 3, "/..", 3) == 0){
                continue;
            }
            unsigned long index_parent = i;
            for (unsigned long j = i; j > 0; j--){
                if (curpath[j - 1] == '/'){
                    index_parent = j - 1;
                    tmp = string_insert(tmp, curpath, 0, j, true, 'l');
                    break;
                }
            }
            if (index_parent == i){
                nls_message(7, "cd: Could not resolve parent directory");
                return nullptr;
            }
            errno = 0;
            DIR *dir = opendir(tmp);
            int errno_save = errno;
            if (dir){
                closedir(dir);
            }
            if (errno_save == ENOENT || errno_save == ENOTDIR){
                nls_message(8, "cd: Illegal move to parent directory");
                return nullptr;
            }
            curpath = string_insert(curpath, curpath + i + 3, index_parent, strlen(curpath + i + 3), true, 'l');
            i = index_parent;
        }
    }
    if (strcmp(curpath, "/") != 0 && strcmp(curpath, "//") != 0 && curpath[strlen(curpath) - 1] == '/'){
        curpath[strlen(curpath) - 1] = '\0';
    }
    if (strlen(curpath) == 0){
        return nullptr;
    }
    return curpath;
}

// Local  
// Create realtive path if possible
char *path_relativize(char *curpath){
    DIR *dir;
    char *curpath_relative = nullptr;
    char *tmp = string_copy(pwd->value, 'l');
    if (tmp[strlen(tmp) - 1] != '/'){
        tmp = string_concat(tmp, "/", 'l');
    }
    if (strncmp(tmp, curpath, strlen(tmp)) == 0){
        curpath_relative = string_copy(curpath + strlen(tmp), 'l');
    }
    else if (strncmp(curpath, tmp, strlen(curpath)) == 0){
        for (unsigned long i = strlen(tmp); i > strlen(curpath) - 2; i--){
            if (tmp[i] == '/'){
                tmp[i] = '\0';
                errno = 0;
                dir = opendir(tmp);
                if (errno == ENOENT || errno == ENOTDIR){
                    return nullptr;
                }
                else if (dir){
                    closedir(dir);
                }
                curpath_relative = string_concat(curpath_relative, "../", 'l');
            }
        }
    }
    else {
        unsigned long index = 0;
        while (tmp[index] == curpath[index]){
            index++;
        }
        for (unsigned long i = strlen(tmp); i > index - 2; i--){
            if (tmp[i] == '/'){
                tmp[i] = '\0';
                errno = 0;
                dir = opendir(tmp);
                int errno_save = errno;
                if (dir){
                    closedir(dir);
                }
                if (errno_save == ENOENT || errno_save == ENOTDIR){
                    return nullptr;
                }
                curpath_relative = string_concat(curpath_relative, "../", 'l');
            }
        }
        curpath_relative = string_concat(curpath_relative, curpath + index, 'l');
    }
    return curpath_relative;
}

int cd(int in_c, char **in_v){ // "Main" function
    print_pwd = false;
    char *curpath = nullptr;
    char *curpath_relative = nullptr;
    if (pwd_validate()){
        pwd = sh_var_get("PWD");
    }
    if (!pwd){
        return 2;
    }
    nls_catalog_set("emsh_cd");
    char *arg_path = arg_resolve(in_c, in_v);
    if (!arg_path){
        return 2;
    }
    if (arg_path[0] == '/'){
        curpath = string_copy(arg_path, 'l');
    }
    else if (strncmp(arg_path, "./", 2) == 0 || strncmp(arg_path, "../", 3) == 0){
        curpath = string_copy(arg_path, 'l');
    }
    else {
        curpath = cdpath_resolve(arg_path);
        if (!curpath){
            curpath = string_copy(arg_path, 'l');
        }
    }
    if (opt != 'P'){
        curpath = path_canonicalize(curpath);
        if (strlen(curpath) + 1 > PATH_MAX){
            curpath_relative = path_relativize(curpath);
        }
        if (curpath_relative && strlen(curpath) < strlen(curpath_relative)){
            curpath_relative = nullptr;
        }
    }
    // chdir()
    int status;
    if (curpath_relative){
        status = chdir(curpath_relative);
    }
    else {
        status = chdir(curpath);
    }
    if (status == -1){
        perror("cd");
        return 2;
    }
    // Update PWD, OLDPWD
    return pwd_update(curpath);
}