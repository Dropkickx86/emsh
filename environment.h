#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

struct ShVar {
    char *name;
    char *value;
    bool ro;
    bool export;
};

// Exported environment variables
extern char **environ;

struct ShVar *sh_var_get(const char *name);
int sh_var_set(const char *name, const char *value);
int sh_vars_populate();
void sh_vars_init();
void signal_ignore(int signal);
void sigint_handling();
void signals_restore();

#endif