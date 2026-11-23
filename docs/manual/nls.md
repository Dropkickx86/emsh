# Native Language Support - nls.c

EMSH utilizes message catalog files for NLS, one file per utility and one general error message file.

## Catalog file structure

Every message catalog has one set, which is set 1. The first entry is the name of the utility. Each following entry is a short descriptive error message to be printed to STDERR on errors.

## void nls_catalog_set(const char *catalog)

Set the current message catalog from which to read messages.

### Input

{catalog} is a char pointer to a string containing the name of the message catalog without file extension.

### Notes on usage

When called, the currently open message catalog is closed and one attempt to open the specified message catalog is made.

### Error information

If the specified message catalog cannot be opened the catalog variable will be set to (nl_catd)-1 to signal that no message catalog is open.

## void nls_catalog_close();

Close the currently open message catalog.

### Notes on usage

When the message catalog is closed, the catalog variable is set to (nl_catd)-1 to signal that no message catalog is open.

## void nls_message(int message_number, const char *fallback)

Print message to STDERR.

### Input

{message_number} is an integer detailing which message from the open message catalog to display.  
{fallback} is a char pointer to a string containing a message to be displayed if the specified message cannot be fetched from the message catalog.

### Notes on usage

When calling the function, a message will be printed to STDERR in the format "tool: message\n". "tool" is the utility currently being used and is fetched from the message catalog, entry 1. "message" is the message fetched from the message catalog, entry {message_number}.

### Error information

If information cannot be fetched from the message catalog, then {fallback} is printed to STDERR as is.