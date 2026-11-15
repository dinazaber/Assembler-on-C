#ifndef ERRORS_H
#define ERRORS_H
#include <stdio.h>

#define MAX_LINE_LENGTH 80

typedef enum {
    NO_ERROR,
    INVALID_COMMAND,
    UNDEFINED_REG,
    EXTRA_TEXT,
    CONSECUTIVE_COMMAS,
    MISSING_PARAM,
    INVALID_PARAM,
    MISSING_COMMA,
    MISSING_REG,
    OUT_OF_BOUNDS_REG,
    DOUBLE_LABEL,
    INVALID_LABEL,
    MISSING_LABEL,
    VALUE_OUT_OF_BOUNDS,
    MULTIPLE_DIRECTIVES,
    INVALID_MCRO_NAME,
    DOUBLE_MCRO_DEC,
    UNDECLARED_LABEL,
    EXTERN_IN_I_TYPE,
    LINE_OVERFLOW
} InputError; /* all error types */

/*
 Function: print_error
 ---------------------
 What it does:
 Prints a formatted error message to standard output based on an error code.

 Input/Output:
 - Input: err (InputError code), line_count (int line number).
 - Output: Outputs text to standard output (void).

 How it works:
 Translates the InputError enum value into a descriptive error string using a
 switch statement and prints it with line context.
*/
void print_error(InputError err, int line_count);

#endif
