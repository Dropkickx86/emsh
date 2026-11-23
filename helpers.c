#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <emgc.h>

#include "environment.h"
#include "helpers.h"
#include "nls.h"

size_t file_number_print(FILE *restrict stream, const size_t n, char *restrict const src){
    const char index_char = src[n];
    src[n] = '\0';
    int b_written = fprintf(stream, "%s", src);
    src[n] = index_char;
    return b_written;
}

char *string_insert(char *restrict dest, const char *restrict src, const unsigned long index, const size_t n, const bool termination, unsigned char scope){
    unsigned int dest_len;
    // Malloc if nullptr
    if (!dest){
        dest_len = 0;
        dest = gc_malloc(scope, (n + 1) * sizeof(char));
    }
    else if (termination && index == 0){
        dest_len = 0;
    }
    else {
        dest_len = strlen(dest);
    }
    if (index > dest_len){
        nls_message(3, "emsh: Invalid value for internal function");
        return nullptr;
    }
    if (n > strlen(src)){
        nls_message(3, "emsh: Invalid value for internal function");
        return nullptr;
    }
    // Realloc if dest is too small
    if (gc_get_alloc_size(dest) < (dest_len + n + 1) * sizeof(char)){
        char *tmp = string_copy(src, 'l');
        src = tmp;
        tmp = gc_realloc(dest, (dest_len + n + 1) * sizeof(char));
        dest = tmp;
    }
    // Insert into dest
    if (n == 0){
        if (termination){
            dest[index] = '\0';
        }
        return dest;
    }
    if (dest_len == 0){
        memcpy(dest, src, n);
        dest[n] = '\0';
        return dest;
    }
    if (!termination){
        memmove(dest + index + n, dest + index, dest_len - index);
    }
    if ((src >= dest || src + n >= dest) && (src <= (dest + dest_len) || src + n <= (dest + dest_len))){
        memmove(dest + index, src, n);
    }
    else{
        memcpy(dest + index, src, n);
    }
    if (termination){
        dest[index + n] = '\0';
    }
    return dest;
}

char *string_copy(const char *const src, unsigned char scope){
    return string_insert(nullptr, src, 0, strlen(src), true, scope);
}

char *string_concat(char *dest, const char *const src, unsigned char scope){
    return string_insert(dest, src, strlen(dest), strlen(src), true, scope);
}

char **string_tokenize(char **output, unsigned int *out_c, const char *const input, const char *ifs, unsigned char scope){
    if (!output || !out_c || !input || !ifs){
        nls_message(3, "emsh: Invalid value for internal function");
        return nullptr;
    }
    unsigned long cursor = 0;
    unsigned long ss_start = 0;
    *out_c = 0;
    do {
        char delimiter = 0;
        for (unsigned short i = 0; ifs[i] != '\0'; i++){
            if (input[cursor] == ifs[i]){
                delimiter = ifs[i];
                break;
            }
        }
        if (input[cursor] == delimiter){
            if (cursor == 0 || cursor == ss_start){
                ss_start = cursor + 1;
                continue;
            }
            if ((gc_get_alloc_size(output) / sizeof(char*)) == *out_c){
                output = gc_realloc(output, (*out_c * 2) * sizeof(char*));
            }
            output[*out_c] = string_insert(output[*out_c], input + ss_start, 0, cursor - ss_start, true, scope);
            if (!output[*out_c]){
                return nullptr;
            }
            (*out_c)++;
            ss_start = cursor + 1;
        }
    } while (input[cursor++] != '\0');
    return output;
}

bool pwd_validate(){
    const struct ShVar *pwd = sh_var_get("PWD");
    char *cwd = gc_malloc('l', 16 * sizeof(char));
    do {
        cwd = gc_realloc(cwd, gc_get_alloc_size(cwd) * 2);
        errno = 0;
        cwd = getcwd(cwd, gc_get_alloc_size(cwd));
    } while (!cwd && errno == ERANGE);
    if (!cwd){
        nls_message(6, "emsh: Could not validate PWD");
        return false;
    }
    else if (!pwd){
        goto update;
    }
    else if (pwd->value && strlen(pwd->value) == 0){
        goto update;
    }
    else {
        if (pwd->value[0] != '/'){
            goto update;
        }
        for (unsigned long i = 0; i < strlen(pwd->value); i++){
            if (strncmp(pwd->value + i, "//", 2) == 0 && i > 0){
                goto update;
            }
            else if (strncmp(pwd->value + i, "/./", 3) == 0){
                goto update;
            }
            else if (strncmp(pwd->value + i, "/../", 4) == 0){
                goto update;
            }
        }
        struct stat s_cwd, s_pwd;
        if (stat(cwd, &s_cwd) == -1){
            nls_message(6, "emsh: Could not validate PWD");
            return false;
        }
        else if (stat(pwd->value, &s_pwd) == -1){
            nls_message(6, "emsh: Could not validate PWD");
            return false;
        }
        else if (s_cwd.st_dev != s_pwd.st_dev || s_cwd.st_ino != s_pwd.st_ino){
            goto update;
        }
    }
    return true;
    update:
    if (sh_var_set("PWD", cwd) == -1){
        nls_message(6, "emsh: Could not validate PWD");
        return false;
    }
    return true;
}