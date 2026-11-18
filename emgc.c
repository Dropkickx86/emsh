#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "emgc.h"

typedef struct {
    void **ptr_array;
    unsigned int *size_array;
    unsigned int cursor;
    unsigned int size;
} AllocArray;

static AllocArray *scope_array = NULL;
static unsigned char *scopes = NULL;
static int no_scopes = 0;

static int exit_on_fail = 0;
static int null_init = 0;

// Create new scope
int gc_construct(unsigned char scope){
    // Init if first construct
    if (!scope_array){
        scope_array = (AllocArray*) malloc(sizeof(AllocArray) * 256);
        if (scopes){
            free(scopes);
        }
        scopes = malloc(sizeof(char) * 257);
        if (!scope_array || !scopes){
            fprintf(stderr, "emgc: init failed\n");
            if (exit_on_fail){
                exit(EXIT_FAILURE);
            }
            else {
                return -1;
            }
        }
        scopes[0] = '\0';
    }
    // Check if scope exists
    for (int i = 0; scopes[i] != '\0'; i++){
        if (scopes[i] == scope){
            fprintf(stderr, "emgc: %c: scope already constructed\n", scope);
            return -1;
        }
    }
    // Create scope
    scope_array[scope].ptr_array = malloc(PTRARRAY_INITSIZE * sizeof(void**));
    scope_array[scope].size_array = malloc(PTRARRAY_INITSIZE * sizeof(unsigned int*));
    if (!scope_array[scope].ptr_array || !scope_array[scope].size_array){
        fprintf(stderr, "emgc: failed to allocate memory\n");
        if (exit_on_fail){
            exit(EXIT_FAILURE);
        }
        else {
            return -1;
        }
    }
    scope_array[scope].cursor = 0;
    scope_array[scope].size = PTRARRAY_INITSIZE;
    scopes[no_scopes++] = scope;
    scopes[no_scopes] = '\0';
    return 0;
}

// Destroy scope
int gc_destruct(unsigned char scope){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return -1;
    }
    // Check if scope exists
    if (!scope_array[scope].ptr_array){
        fprintf(stderr, "emgc: %c: scope not constructed\n", scope);
        return -1;
    }
    // Destroy scope
    gc_clean(scope);
    free(scope_array[scope].ptr_array);
    free(scope_array[scope].size_array);
    scope_array[scope].ptr_array = NULL;
    scope_array[scope].size_array = NULL;
    for (int i = 0; scopes[i] != '\0'; i++){
        if (scopes[i] == scope){
            memmove(scopes + i, scopes + i + 1, no_scopes - i);
            break;
        }
    }
    no_scopes--;
    return 0;
}

// Malloc gc-aware memory
void *gc_malloc(unsigned char scope, size_t size){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return NULL;
    }
    // Check if scope exists
    if (!scope_array[scope].ptr_array){
        fprintf(stderr, "emgc: %c: scope not constructed\n", scope);
        return NULL;
    }
    // Check that scope allocation is not full
    AllocArray *scope_cur = &scope_array[scope];
    if (scope_cur->cursor == scope_cur->size){
        if (scope_cur->size == UINT_MAX){
            fprintf(stderr, "emgc: %c: scope at max size\n", scope);
            return NULL;
        }
        else if (scope_cur->size > (UINT_MAX / 2)){
            scope_cur->size = UINT_MAX;
        }
        else {
            scope_cur->size *= 2;
        }
        void **tmp_ptr = realloc(scope_cur->ptr_array, (scope_cur->size * sizeof(void**)));
        void *tmp_size = realloc(scope_cur->size_array, (scope_cur->size * sizeof(unsigned int*)));
        if (!tmp_ptr || !tmp_size){
            if (tmp_ptr){
                free(tmp_ptr);
            }
            fprintf(stderr, "emgc: failed to allocate memory\n");
            if (exit_on_fail){
                exit(EXIT_FAILURE);
            }
            else {
                return NULL;
            }
        }
        scope_cur->ptr_array = tmp_ptr;
        scope_cur->size_array = tmp_size;
    }
    // Check that size is valid
    if (size < 1){
        fprintf(stderr, "emgc: %zu: invalid size\n", size);
        return NULL;
    }
    // Malloc
    void *tmp = malloc(size);
    if (!tmp){
        fprintf(stderr, "emgc: failed to allocate memory\n");
        if (exit_on_fail){
            exit(EXIT_FAILURE);
        }
        else {
            return NULL;
        }
    }
    if (null_init){
        memset(tmp, 0, size);
    }
    scope_cur->ptr_array[scope_cur->cursor] = tmp;
    scope_cur->size_array[scope_cur->cursor] = size;
    return scope_cur->ptr_array[scope_cur->cursor++];
}

// Calloc gc-aware memory
void *gc_calloc(unsigned char scope, size_t nmemb, size_t size){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return NULL;
    }
    // Check if scope exists
    if (!scope_array[scope].ptr_array){
        fprintf(stderr, "emgc: %c: scope not constructed\n", scope);
        return NULL;
    }
    // Check that scope allocation is not full
    AllocArray *scope_cur = &scope_array[scope];
    if (scope_cur->cursor == scope_cur->size){
        if (scope_cur->size == UINT_MAX){
            fprintf(stderr, "emgc: %c: scope at max size\n", scope);
            return NULL;
        }
        else if (scope_cur->size > (UINT_MAX / 2)){
            scope_cur->size = UINT_MAX;
        }
        else {
            scope_cur->size *= 2;
        }
        void **tmp_ptr = realloc(scope_cur->ptr_array, (scope_cur->size * sizeof(void**)));
        void *tmp_size = realloc(scope_cur->size_array, (scope_cur->size * sizeof(unsigned int*)));
        if (!tmp_ptr || !tmp_size){
            if (tmp_ptr){
                free(tmp_ptr);
            }
            fprintf(stderr, "emgc: failed to allocate memory\n");
            if (exit_on_fail){
                exit(EXIT_FAILURE);
            }
            else {
                return NULL;
            }
        }
        scope_cur->ptr_array = tmp_ptr;
        scope_cur->size_array = tmp_size;
    }
    // Check that size is valid
    if (size < 1){
        fprintf(stderr, "emgc: %zu: invalid size\n", size);
        return NULL;
    }
    else if (nmemb < 1){
        fprintf(stderr, "emgc: %zu: invalid number of elements\n", nmemb);
        return NULL;
    }
    // Calloc
    void **ptr_main = gc_malloc(scope, nmemb);
    if (!ptr_main){
        fprintf(stderr, "emgc: failed to allocate memory\n");
        if (exit_on_fail){
            exit(EXIT_FAILURE);
        }
        else {
            return NULL;
        }
    }
    for (size_t i = 0; i < nmemb; i++){
        ptr_main[i] = gc_malloc(scope, size);
        if (!ptr_main[i]){
            fprintf(stderr, "emgc: failed to allocate memory\n");
            if (exit_on_fail){
                exit(EXIT_FAILURE);
            }
            else {
                return NULL;
            }
        }
    }
    return ptr_main;
}

// Realloc gc-aware memory
void *gc_realloc(void *ptr, size_t size){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return NULL;
    }
    // Find ptr
    AllocArray *scope_cur = NULL;
    unsigned int ptr_cursor;
    for (int i = 0; scopes[i] != '\0'; i++){
        for (unsigned int j = 0; j < scope_array[scopes[i]].cursor; j++){
            if (scope_array[scopes[i]].ptr_array[j] == ptr){
                scope_cur = &scope_array[scopes[i]];
                ptr_cursor = j;
                break;
            }
        }
        if (scope_cur){
            break;
        }
    }
    if (!scope_cur){
        fprintf(stderr, "emgc: pointer not found\n");
        return NULL;
    }
    // Check that size is valid
    if (size < 1){
        fprintf(stderr, "emgc: %zu: invalid size\n", size);
        return NULL;
    }
    // Realloc
    void *tmp = realloc(scope_cur->ptr_array[ptr_cursor], size);
    if (!tmp){
        fprintf(stderr, "emgc: failed to allocate memory\n");
        if (exit_on_fail){
            exit(EXIT_FAILURE);
        }
        else {
            return NULL;
        }
    }
    if (null_init && size > scope_cur->size_array[ptr_cursor]){
        memset((char*) tmp + scope_cur->size_array[ptr_cursor], 0, size - scope_cur->size_array[ptr_cursor]);
    }
    scope_cur->ptr_array[ptr_cursor] = tmp;
    scope_cur->size_array[ptr_cursor] = size;
    return scope_cur->ptr_array[ptr_cursor];
}

// Remove all entries in scope
void gc_clean(unsigned char scope){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return;
    }
    // Check if scope exists
    if (!scope_array[scope].ptr_array){
        fprintf(stderr, "emgc: %c: scope not constructed\n", scope);
        return;
    }
    // Empty scope
    for (; scope_array[scope].cursor > 0; --scope_array[scope].cursor){
        free(scope_array[scope].ptr_array[scope_array[scope].cursor]);
        scope_array[scope].ptr_array[scope_array[scope].cursor] = NULL;
    }
}

// Free everything related to gc
void gc_end(void){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return;
    }
    // Remove all
    while (scopes[0] != '\0'){
        gc_destruct(scopes[0]);
    }
    free(scopes);
    free(scope_array);
    scopes = NULL;
    scope_array = NULL;
}

// Get allocation size of ptr
unsigned int gc_get_alloc_size(void *ptr){
    // Check init
    if (!scope_array){
        fprintf(stderr, "emgc: no scopes constructed\n");
        return 0;
    }
    // Find ptr
    for (int i = 0; scopes[i] != '\0'; i++){
        for (unsigned int j = 0; j < scope_array[scopes[i]].cursor; j++){
            if (scope_array[scopes[i]].ptr_array[j] == ptr){
                return scope_array[scopes[i]].size_array[j];
            }
        }
    }
    fprintf(stderr, "emgc: pointer not found\n");
    return 0;    
}

void gc_exit_on_alloc_fail(int setting){
    if (setting >= 0 && setting <= 1){
        exit_on_fail = setting;
    }
}

void gc_null_init_memory(int setting){
    if (setting >= 0 && setting <= 1){
        null_init = setting;
    }
}