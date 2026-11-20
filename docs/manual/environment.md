# Environment handling and initialization - environment.c

Main handler for shell variables, and by extension environment variables.

## Initialization

On calling sh_vars_populate(), *environ will be read and all valid pre-existing environment variables will be added as shell variables which are marked for export. If the variable is invalid, it is exported without being added as a shell variable.

On calling sh_vars_init(), the variables PPID, PS1, PS2, PS4 and PWD will be created as shell variables if they do not exist, and their value will be set.

PPID will be set to the parent process PID.

PS1 will be set to "$ " for regular users or "# " for root. PS2 will be set to "> " and PS4 will be set to "+ ".

PWD will be validated if the variable contains a value when calling the function. If the value is invalid or missing, it will be set to the current working directory. If validation of PWD did not complete successfully, or the value of PWD could not be updated if invalid or missing, a message will be printed to STDERR and execution will continue.

## Shell variable setting and getting

Calling sh_var_get(NAME) will return a struct ShVar pointer to the specified variable if found, or a nullpointer if not found.

Calling sh_var_set(NAME, VALUE) will update the value of the specified variable if found, or create a new variable if not found, and the return 0. If the variable exists but is read-only, a message will be written to STDERR and the function returns -1.