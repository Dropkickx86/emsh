#ifndef EMSH_H
#define EMSH_H

// Flags
struct FlagRegister {
    bool exit;
    bool failure;
};
extern struct FlagRegister flag;

// Options
struct ShellOpts {
    bool vi;
};
extern struct ShellOpts option;

#endif