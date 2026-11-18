#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Terminal action escape codes
#define curs_save()         fprintf(stderr, "\0337")
#define curs_home()         fprintf(stderr, "\0338")
#define curs_sol()          fprintf(stderr, "\033[1G")
#define curs_eol()          fprintf(stderr, "\033[9999C")
#define erase_scrfw()       fprintf(stderr, "\033[0J")
#define curs_up(n)          fprintf(stderr, "\033[%dA", n)
#define curs_down(n)        fprintf(stderr, "\033[%dB", n)
#define curs_right(n)       fprintf(stderr, "\033[%dC", n)
#define curs_left(n)        fprintf(stderr, "\033[%dD", n)
#define curs_goto_col(n)    fprintf(stderr, "\033[%dG", n)

// Print up to n characters from src to stream  
// Returns number of bytes printed
int file_number_print(FILE *restrict stream, const size_t n, char *restrict const src);
// Insert n characters from src into dest at index  
// Termination inserts null terminator into dest at index + n  
// Returns dest on success and nullptr on failure  
// Realloc if dest is too small, malloc to scope if dest is nullptr
char *string_insert(char *restrict dest, const char *restrict src, const int index, const size_t n, const bool termination, unsigned char scope);
// Returns pointer containing string src malloc'd to scope, nullptr on failure
char *string_copy(const char *const src, unsigned char scope);
// Inserts src at end of dest  
// Returns pointer to dest on success and nullptr on failure
// Realloc if dest is too small, malloc to scope if dest is nullptr
char *string_concat(char *dest, const char *const src, unsigned char scope);
// Split input at ifs and put value into output, no tokens in out_c  
// Returns output on success and nullptr on failure  
// output must be pre-allocated, but will be reallocated if too small. *output will be allocated to scope if nullptr
char **string_tokenize(char **output, int *out_c, const char *const input, const char *ifs, unsigned char scope);

#endif