#ifndef EMSH_H
#define EMSH_H

// Flags
struct FlagRegister {
    bool exit;
    bool failure;
    bool sigint;
};
extern struct FlagRegister flag;

// Options
struct ShellOpts {
    bool vi;
    bool interactive;
};
extern struct ShellOpts option;

#endif