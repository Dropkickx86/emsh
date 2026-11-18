#ifndef EMSH_H
#define EMSH_H

// Flags
typedef struct {
    bool exit;
    bool failure;
} FlagRegister;
extern FlagRegister flag;

// Options
typedef struct {
    bool vi;
} ShellOpts;
extern ShellOpts option;

// Shell variables  
// PS1-PS4 ASCII only!
typedef struct {
    char *ENV;
    char *HOME;
    char *IFS;
    char *LANG;
    char *LC_ALL;
    char *LC_COLLATE;
    char *LC_CTYPE;
    char *LC_MESSAGES;
    char *LINENO;
    char *NLSPATH;
    char *PATH;
    char *PPID;
    char *PS1;
    char *PS2;
    char *PS4;
    char *PWD;
} ShVarsShortcut;
extern ShVarsShortcut shell_var_short;

#endif