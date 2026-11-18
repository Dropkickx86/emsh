#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

// Environment variables
struct ShVar {
    char *name;
    char *value;
    bool ro;
    bool export;
};
extern struct ShVar *shell_vars;

// Exported environment variables
extern char **environ;

struct ShVar *sh_var_get(const char *name);
int sh_var_set(const char *name, const char *value);
int sh_vars_populate();
void sh_vars_init();

#endif