#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include "errors.h"
#include "lst.h"
#define MAX_STRING_SIZE 12
#define MAX_BYTE 127
#define MIN_BYTE -128
#define MAX_HALF_WORD 32767
#define MIN_HALF_WORD -32768



typedef enum {
    DIRECTIVE_INVALID = 0,
    DIRECTIVE_DB, /* 8-bit  */
    DIRECTIVE_DH, /* 16-bit */
    DIRECTIVE_DW,  /* 32-bit */
    DIRECTIVE_ASCIZ,
    DIRECTIVE_EXTERN,
    DIRECTIVE_ENTRY
} DirectiveType;

DirectiveType get_directive_type(const char *token);
int process_data_directive(const char *args_str, DirectiveType dir, int *bytes_added, InputError *err, list_ptr data_list);
void asciz_to_hex(char *hex_out, const char *start, const char *end);
void data_directive_to_hex(char *res, long data[], int count, DirectiveType dir);

#endif
