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

#endif