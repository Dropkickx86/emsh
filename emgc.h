#ifndef EMGC_H
#define EMGC_H

#include <stddef.h>

#define PTRARRAY_INITSIZE 8

int gc_construct(unsigned char scope);
int gc_destruct(unsigned char scope);
void *gc_malloc(unsigned char scope, size_t size);
void *gc_calloc(unsigned char scope, size_t nmemb, size_t size);
void *gc_realloc(void *ptr, size_t size);
void gc_clean(unsigned char scope);
void gc_end(void);
unsigned int gc_get_alloc_size(void *ptr);
void gc_exit_on_alloc_fail(int setting);
void gc_null_init_memory(int setting);

#endif
