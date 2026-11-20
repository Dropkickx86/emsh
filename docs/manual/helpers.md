# Program-wide helper functions - helpers.c

This document describes the implemented helper functions and their usage.

## Macros

The following macros are defined in helpers.h to simpify output handling:

curs_save()         Save cursor position as home position  
curs_home()         Move cursor to home position  
curs_sol()          Move cursor to start of line  
curs_eol()          Move cursor to end of line  
erase_scrfw()       Clear screen after the cursor  
curs_up(n)          Move cursor up {n > 0} steps  
curs_down(n)        Move cursor down {n > 0} steps  
curs_right(n)       Move cursor right {n > 0} steps  
curs_left(n)        Move cursor left {n > 0} steps  
curs_goto_col(n)    Move cursor to column n

## size_t **file_number_print**(FILE *restrict stream, const size_t n, char *restrict const src)

Print up to {n} characters from {src} to {stream}.

### Input

{stream} is a FILE pointer to the output stream.  
{n} is an unsigned long containing the number of characters to print.  
{src} is a char pointer to the string to output.

### Return value

The function returns the number of bytes written to {stream}.

### Error information

The function never fails. Code calling the function have to check the return value to ensure the specified number och characters was output.

## char \***string_insert**(char *restrict dest, const char *restrict src, const int index, const size_t n, const bool termination, unsigned char scope)

Insert n characters from src into dest at index

### Input

{dest} is a char pointer to the destination string.  
{src} is a char pointer to the source string.  
{index} is a integer detailing at what index of {dest} to insert {src}.  
{n} is an unsigned long detailing how many bytes of {src} to insert into {dest}.  
{termination} is a bool detailing whether a null terminator should be inserted at the end of {src} after inserting it into {dest}.  
{scope} is a char detailing in what scope memory shall be allocated, should memory allocation be needed.

### Output

The function returns a char pointer to the first char of {dest}.

### Notes on usage

If {dest} does not have enough memory allocated to insert {n} characters from {src}, then {dest} will be reallocated.

if {dest} is a nullpointer, memory will be allocated to {dest} in {scope} before inserting {src}.

### Error information

The function will return a nullpointer if {index} or {n} are invalid.

## char \***string_copy**(const char *const src, unsigned char scope)

Wrapper function for string_insert, allocates and returns a char pointer containing the value of {src}.

### Input

{src} is a char pointer to the source string.  
{scope} is a char detailing in what scope memory shall be allocated.

### Output

The function returns a char pointer to the first adress of the allocated memory.

### Error information

See the entry on string_insert.

## char \***string_concat**(char *dest, const char *const src, unsigned char scope)

Wrapper function for string_insert, adds {src} to end of {dest}.

### Input

{dest} is a char pointer to the destination string.  
{src} is a char pointer to the source string.  
{scope} is a char detailing in what scope memory shall be allocated, should memory allocation be needed.

### Output

The function returns a char pointer to the first char of {dest}.

### Notes on usage

If {dest} does not have enough memory allocated to add all characters from {src}, then {dest} will be reallocated.

if {dest} is a nullpointer, memory will be allocated to {dest} in {scope} before inserting {src}.

### Error information

See the entry on string_insert.

## char \*\***string_tokenize**(char **output, int *out_c, const char *const input, const char *ifs, unsigned char scope)

Split {input} at {ifs} and put value into {output}, no tokens in {out_c}.

### Input

{output} is a preallocated char pointer to pointer for storing tokens.  
{out_c} is a int pointer to a variable for storing amount of tokens.  
{input} is a char pointer to the string which to tokenize.  
{ifs} is a char pointer to a string containing the defined delimiters.  
{scope} is a char detailing in what scope memory shall be allocated, should memory allocation be needed.

### Output

The function returns a char pointer to the first element of {output}.

### Notes on usage

If {output} does not have enough memory allocated for all tokens, then {output} will be reallocated.

If an element of {output} is a nullpointer, memory will be allocated in {scope} before the token is added into the element.

### Error information

The function will return a nullpointer on error.