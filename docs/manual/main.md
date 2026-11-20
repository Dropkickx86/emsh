# Main loop - emsh.c

Init, main running loop, exit handler.

## Synopsis

emsh

## Description

The main function initializes the program, inititalizes the loop, starts the input reader, parses input, executes input actions, does cleanup and restarts the loop.

On exit, the main function handles cleanup and then exits with a return value.

## Options

## Operands

## STDIN

Not used.

## Input files

## Environment variables

## Asynchronous events

## STDOUT

Not used.

## STDERR

The standard error shall be used only for diagnostic messages.

## Output files

## Extended description

## Exit status

The following exit values shall be returned:

0 - Program executed successfully.
0 - Program exited successfully on user exit request.
\>0 - An error occured

## Consequenses of errors

The program will end execution.

## Application usage

## Examples

## Rationale

## Future directions

## See also

## Change history

Changes are not recorded before part reaches its first complete state.