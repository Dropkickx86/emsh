# cd - cd.c

The implementation of the cd intrinsic utility works as specified in [POSIX.1-2024](https://pubs.opengroup.org/onlinepubs/9799919799/) unless otherwise specified in this document.

## Description

> 1. If no directory operand is given and the HOME environment variable is empty or undefined, **the default behavior is implementation-defined** and no further steps shall be taken.

If no directory operand is given and the HOME environment variable is empty or undefined, it will be treates as if an empty string was provided as the directory operand.

> 8. The curpath value shall then be converted to canonical form as follows, considering each component from beginning to end, in sequence:
>
>> c. **An implementation may further simplify curpath by removing any trailing \<slash> characters that are not also leading \<slash> characters, replacing multiple non-leading consecutive \<slash> characters with a single \<slash>, and replacing three or more leading \<slash> characters with a single \<slash>.** If, as a result of this canonicalization, the curpath variable is null, no further steps shall be taken.

Simplifying of curpath is done as described in POSIX.1-2024.

> 9. If curpath is longer than {PATH_MAX} bytes (including the terminating null) and the directory operand was not longer than {PATH_MAX} bytes (including the terminating null), then curpath shall be converted from an absolute pathname to an equivalent relative pathname if possible. This conversion shall always be considered possible if the value of PWD , with a trailing \<slash> added if it does not already have one, is an initial substring of curpath. **Whether or not it is considered possible under other circumstances is unspecified. Implementations may also apply this conversion if curpath is not longer than {PATH_MAX} bytes or the directory operand was longer than {PATH_MAX} bytes.**

Converting curpath to a relative pathname is also considered possible if curpath, with a trailing \<slash> added if it does not already have one, is an initial substring of the value of PWD. If neither pathname are an initial substring of the other, the lowest common directory is found and a relative path is calculated with this common directory as the root of the relative pathname.

Convertion to a relative pathname is not considered if curpath is not longer than {PATH_MAX}. The length of the directory operand is ignored in all cases.

> If the -P option is in effect, the PWD environment variable shall be set to the string that would be output by pwd -P. If there is insufficient permission on the new directory, or on any parent of that directory, to determine the current working directory, **the value of the PWD environment variable is unspecified.**

If the current working directory can not be determined, the value of PWD is set to an empty string.

> **It is unspecified whether, when setting OLDPWD , the shell also causes it to be exported if it was not already.**

OLDPWD will not be exported unless it is marked for export.

## Operands

> If directory consists of a single '-' (\<hyphen-minus>) character, the cd utility shall behave as if directory contained the value of the OLDPWD environment variable, except that after it sets the value of PWD it shall write the new value to standard output. **The behavior is unspecified if OLDPWD does not start with a \<slash> character.**

If the directory operand consists of a single '-' (\<hyphen-minus>) character, the cd utility will behave as if the value of OLDPWD was provided instead of '-'. The value of OLDPWD is not checked before being used for the directory operand.

## Environment variables

Usage of localization environment variables is not implemented and will be revisited later.

> OLDPWD
>
>> **If an application sets or unsets the value of OLDPWD , the behavior of cd with a '-' operand is unspecified.**

If OLDPWD is unset, an error message is printed to STDERR and the cd utility exits with an error code. If OLDPWD is set the cd utility will behave as if the value of OLDPWD was provided instead of '-'. The value of OLDPWD is not checked before being used for the directory operand.

> PWD
>
>> **If an application sets or unsets the value of PWD , the behavior of cd is unspecified.**

The value of PWD is validated as the first step when the cd utility is invoked. If PWD is unset, or if the value of PWD is invalid, an attemt is made to set the value of PWD to the current working directory. If this attemt fails, the cd utility will exit with an error code. An informational message will be printed to STDERR whether setting the value of PWD succeeds or not.

## STDOUT

> If an absolute pathname of the new current working directory cannot be determined, **it is unspecified whether nothing is written to the standard output or the value of curpath used in step 10, followed by a <newline>, is written to the standard output.**

If an absolute pathname of the new current working directory cannot be determined, nothing will be written to STDOUT.

## Exit status

> \>0
> Either the -e option or the -P option is not in effect, and an error occurred.
> 1
> The current working directory was successfully changed, both the -e and the -P options are in effect, and the correct value of the PWD environment variable could not be determined.

With the exception of the situation specified, all errors will cause the exit value 2 to be returned.

## Future directions

> If this utility is directed to display a pathname that contains any bytes that have the encoded value of a \<newline> character when \<newline> is a terminator or separator in the output format being used, implementations are encouraged to treat this as an error.

If this utility is directed to display a pathname that contains any bytes that have the encoded value of a \<newline> character the pathname will not be displayed. This will not affect the exit value of the utility.

## Change history

First added before initial release.