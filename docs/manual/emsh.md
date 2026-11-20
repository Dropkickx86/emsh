# EMSH Developers manual

## Premise

EMSH is fully compliant with [POSIX.1-2024](https://pubs.opengroup.org/onlinepubs/9799919799/). Any part being in conflict with POSIX.1-2024 is unintentional and should be reported to maintainers.

Parts not described in POSIX.1-2024 are added as standard modules, except when needed to support the integration of modules. No standard modules are in conflict with POSIX.1-2024, and none are vital for operation.

All parts of EMSH are implemented as described in POSIX.1-2024 unless otherwise described. All parts declared undefined in POSIX.1-2024 are described, even when behaviour is undefined in EMSH.

## Rules of development

All base code is compliant with C23 extended by POSIX.1-2024. Pure C23 is preferred over POSIX-extensions when reasonably possible. All modules are written in an interpreted language supported by EMSH.

All utilities are defined in their own sourcefile, and exposes one function via a shared header for calling the function. The function and the sourcefile bears the same name, and are named after the utility. Local functions in the sourcefile are allowed.

Supporting functions that are used in several parts of EMSH are added as helper funcions in helpers.c. If the helper function is a replacement for a standard function, the helper function will always be preferred over the standard function.

All C code uses EMGC for memory management. EMSH will attemt to exit gracefully on any memory allocation failure.