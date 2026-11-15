#include "errors.h"

void print_error(InputError err, int line_count) { /* for each error prints error message */
    if (err == NO_ERROR) return;
    printf("[ERROR Line %d]: ", line_count);
    switch (err) {
        case UNDEFINED_REG: printf("Undefined register name.\n"); break;
        case INVALID_COMMAND: printf("Invalid command name\n"); break;
        case EXTRA_TEXT: printf("Extraneous text or trailing comma found.\n"); break;
        case CONSECUTIVE_COMMAS: printf("Consecutive commas found without an intervening parameter.\n"); break;
        case MISSING_PARAM: printf("Missing parameter for command or directive.\n"); break;
        case INVALID_PARAM: printf("Invalid parameter format or leading comma.\n"); break;
        case MISSING_COMMA: printf("Missing expected comma separator.\n"); break;
        case MISSING_REG: printf("Missing required register parameter.\n"); break;
        case OUT_OF_BOUNDS_REG: printf("Register index is out of bounds.\n"); break;
        case DOUBLE_LABEL: printf("Duplicate label definition.\n"); break;
        case INVALID_LABEL: printf("Invalid label name or syntax.\n"); break;
        case MISSING_LABEL: printf("Missing expected label parameter.\n"); break;
        case VALUE_OUT_OF_BOUNDS: printf("Numerical parameter exceeds allowable directive bounds.\n"); break;
        case MULTIPLE_DIRECTIVES: printf("Multiple directives found on the same line.\n"); break;
        case INVALID_MCRO_NAME: printf("A macro can't have a command name\n"); break;
        case DOUBLE_MCRO_DEC: printf("A macro with the same name was declared before\n"); break;
        case UNDECLARED_LABEL: printf("No declaration in file of label "); break;
        case EXTERN_IN_I_TYPE: printf("Using in I type instruction .extern label "); break;
        case LINE_OVERFLOW: printf("Line contains more than 80 characters.\n"); break;
        default: printf("Unknown assembly error occurred.\n"); break;
    }
}
