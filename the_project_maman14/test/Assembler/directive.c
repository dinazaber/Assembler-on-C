#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "directive.h"

#define MAX_LABEL_LEN 31

static char *strdup_custom(const char *s) { /* returns string's duplicate on the heap */
    char *dup; /* the pointer to the duplicate string */
    size_t len; /* the length of the string */

    if (s == NULL) return NULL;
    len = strlen(s) + 1; 
    dup = (char *)malloc(len);
    if (dup != NULL) {
        strcpy(dup, s);
    }
    return dup;
}

DirectiveType get_directive_type(const char *token){ /* returns an enum based on the token */
    if (token == NULL) return DIRECTIVE_INVALID;
    if (strcmp(token, ".db") == 0) return DIRECTIVE_DB;
    if (strcmp(token, ".dh") == 0) return DIRECTIVE_DH;
    if (strcmp(token, ".dw") == 0) return DIRECTIVE_DW;
    if (strcmp(token, ".asciz") == 0) return DIRECTIVE_ASCIZ;
    if (strcmp(token, ".extern") == 0) return DIRECTIVE_EXTERN;
    if (strcmp(token, ".entry") == 0) return DIRECTIVE_ENTRY;
    return DIRECTIVE_INVALID;
}

static int get_directive_bounds(DirectiveType dir, long *min_val, long *max_val, int *unit_size) {
    switch (dir) { /* selects the boundaries of the arguments, based on the input enum */
        case DIRECTIVE_DB:
            *min_val = MIN_BYTE;
            *max_val = MAX_BYTE;
            *unit_size = 1;
            return 1;
        case DIRECTIVE_DH:
            *min_val = MIN_HALF_WORD;
            *max_val = MAX_HALF_WORD;
            *unit_size = 2;
            return 1;
        case DIRECTIVE_DW:
            *min_val = LONG_MIN;
            *max_val = LONG_MAX;
            *unit_size = 4;
            return 1;
        default:
            return 0;
    }
}

static int has_trailing_garbage(const char *endptr) { 
    while (*endptr != '\0') { /* checks for trailing garbage */
        if (!isspace((unsigned char)*endptr)) return 1;
        endptr++;
    }
    return 0;
}

static InputError validate_number(const char *str, long min_val, long max_val, long *out_val) {
    char *endptr;
    long val;

    while (*str != '\0' && isspace((unsigned char)*str)) str++; /* skippes whitespaces */
    if (*str == '\0') return MISSING_PARAM;

    errno = 0;
    val = strtol(str, &endptr, 10); /* getting the number from the string */

    if (has_trailing_garbage(endptr)) return INVALID_PARAM;
    if (errno == ERANGE || val < min_val || val > max_val) return VALUE_OUT_OF_BOUNDS; /* checking boundaries */
    
    *out_val = val;
    return NO_ERROR;
}

static int is_all_whitespace(const char *start, const char *end){
    while (start < end){ /* goes through the string */
        if (!isspace((unsigned char)*start)) return 0;
        start++;
    }
    return 1;
}

static InputError validate_asciz_bounds(const char *args_str, const char **start, const char **end){ /* checking syntax and boundaries of arg_str */
    *start = strchr(args_str, '"');
    if (*start == NULL) return INVALID_PARAM;

    if (!is_all_whitespace(args_str, *start)) return INVALID_PARAM;

    *end = strrchr(args_str, '"');
    if (*end == NULL || *end == *start) return INVALID_PARAM;

    if (!is_all_whitespace(*end + 1, args_str + strlen(args_str))) return EXTRA_TEXT;
    
    if ((int)(*end - *start - 1) >= MAX_STRING_SIZE) return INVALID_PARAM;

    return NO_ERROR;
}

static int process_asciz_directive(const char *args_str, int *bytes_added, InputError *err, list_ptr data_list){
    const char *start;
    const char *end;
    char hex_str[MAX_STRING_SIZE * 2];

    *bytes_added = 0;
    if (args_str == NULL || *args_str == '\0'){
        *err = MISSING_PARAM;
        return 0;
    }

    *err = validate_asciz_bounds(args_str, &start, &end); /* checking syntax */
    if (*err != NO_ERROR) return 0;

    *bytes_added = (int)(end - start - 1) + 1; /* the amount of bytes added to DC is is the string's length + null terminator */
    
    asciz_to_hex(hex_str, start, end); /* converting to hex */
    list_write(data_list, hex_str, strlen(hex_str)); /* saving data */
    
    
    return 1;
}

void asciz_to_hex(char *hex_out, const char *start, const char *end){
    const char *ptr;
    char *out_ptr = hex_out;

    /* Convert every character inside the quotes */
    for (ptr = start + 1; ptr < end; ptr++){
        sprintf(out_ptr, "%02X", (unsigned char)(*ptr));
        out_ptr += 2;
    }

    /* Append null terminator byte (00 in hex) for .asciz */
    sprintf(out_ptr, "00");
    out_ptr += 2;
    
    *out_ptr = '\0';
}

static int is_valid_label_chars(const char *label, size_t len){ /* checking label syntax */
    size_t k;
    if (len == 0 || len > MAX_LABEL_LEN || !isalpha((unsigned char)label[0])) return 0;
    
    for (k = 1; k < len; k++) {
        if (!isalnum((unsigned char)label[k])) return 0;
    }
    return 1;
}

static int process_extern_entry_directive(const char *args_str, InputError *err){
    const char *ptr = args_str;
    const char *label_start;
    size_t label_len = 0;

    if (args_str == NULL){ /* missing label */
        *err = MISSING_PARAM;
        return 0;
    }

    while (*ptr != '\0' && isspace((unsigned char)*ptr)) ptr++;
    if (*ptr == '\0'){
        *err = MISSING_PARAM;
        return 0;
    }

    label_start = ptr;
    while (*ptr != '\0' && !isspace((unsigned char)*ptr)){ /* skipping whitespaces */
        label_len++;
        ptr++;
    }

    if (!is_valid_label_chars(label_start, label_len)){
        *err = INVALID_LABEL;
        return 0;
    }

    while (*ptr != '\0' && isspace((unsigned char)*ptr)) ptr++; /* checking end */
    if (*ptr != '\0'){
        *err = EXTRA_TEXT;
        return 0;
    }

    *err = NO_ERROR;
    return 1;
}

static InputError check_comma_errors(const char *str){
    int i = 0;
    int len = (int)strlen(str);

    while (str[i] != '\0' && isspace((unsigned char)str[i])) i++; /* skipping whitespaces */
    if (str[i] == ',') return INVALID_PARAM;

    while (len > 0 && isspace((unsigned char)str[len - 1])) len--; /* checking for garbage */
    if (len > 0 && str[len - 1] == ',') return EXTRA_TEXT;

    for (i = 0; str[i] != '\0'; i++){ /* checking for consecutive commas */
        if (str[i] == ','){
            int j = i + 1;
            while (str[j] != '\0' && isspace((unsigned char)str[j])) j++;
            if (str[j] == ',') return CONSECUTIVE_COMMAS;
        }
    }
    return NO_ERROR;
}

static InputError validate_tokens(char *args_copy, long min_val, long max_val, long **data_args, int *capacity, int *count){
    long *temp = NULL;
    char *token = strtok(args_copy, ",");
    *count = 0;

    while (token != NULL){ /* for each token, checks syntax and boundaries */
        long val;
        InputError num_err = validate_number(token, min_val, max_val, &val);
        if (num_err != NO_ERROR) return num_err;
        
        if (*count >= *capacity){ /* if needed, increasing capasity */
          *capacity *= 2;
          temp = realloc(*data_args, (*capacity) * sizeof(long));
          if (temp == NULL){
            fprintf(stderr, "Out of memory\n");
            exit(1);
          }
          *data_args = temp;
        }
        
        (*data_args)[*count] = val;
        (*count)++;
        token = strtok(NULL, ",");
    }

    return (*count == 0) ? MISSING_PARAM : NO_ERROR;
}

int process_data_directive(const char *args_str, DirectiveType dir, int *bytes_added, InputError *err, list_ptr data_list){
    long min_val, max_val;
    int unit_size, count;
    int capacity = 8;
    char *args_copy;
    long *data_args = malloc(capacity * sizeof(long));
    char *hex_out = NULL;

    *bytes_added = 0;
    *err = NO_ERROR;
    
    /* the wrapper function chooses which function to execute */

    if (dir == DIRECTIVE_EXTERN || dir == DIRECTIVE_ENTRY) return process_extern_entry_directive(args_str, err);
    
    if (dir == DIRECTIVE_ASCIZ) return process_asciz_directive(args_str, bytes_added, err, data_list);
    
    /* deciding error code */
    
    if (!get_directive_bounds(dir, &min_val, &max_val, &unit_size)){
        *err = INVALID_PARAM;
        return 0;
    }
    if (args_str == NULL || *args_str == '\0'){
        *err = MISSING_PARAM;
        return 0;
    }

    *err = check_comma_errors(args_str);
    if (*err != NO_ERROR) return 0;

    args_copy = strdup_custom(args_str); /* getting a copy of the arguments */
    if (args_copy == NULL || data_args == NULL){
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    *err = validate_tokens(args_copy, min_val, max_val, &data_args, &capacity, &count);
    free(args_copy);

    if (*err != NO_ERROR){
      return 0;
      free(data_args);
    }
    
    hex_out = malloc((count * unit_size * 2) + 1);
    
    if(hex_out == NULL){
      printf("can't allocate memory");
      exit(1);
    }
    
    data_directive_to_hex(hex_out, data_args, count, dir);
    list_write(data_list, hex_out, strlen(hex_out));
    free(hex_out);
    
    
    *bytes_added = count * unit_size;
    free(data_args);
    return 1;
}

void data_directive_to_hex(char *res, long data[], int count, DirectiveType dir){
    int i;
    char *out_ptr = res;

    for (i = 0; i < count; i++){
        unsigned int val = (unsigned int)data[i];
        switch (dir){
            case DIRECTIVE_DB:
                /* 1 byte: 8 bits */
                val = val & 0xFF;
                sprintf(out_ptr, "%02X", val);
                out_ptr += 2;
                break;

            case DIRECTIVE_DH:
                /* 2 bytes: 16 bits */
                val = ((val & 0x00FFU) << 8) | ((val & 0xFF00U) >> 8);
                sprintf(out_ptr, "%04X", val);
                out_ptr += 4;
                break;

            case DIRECTIVE_DW:
                /* 4 bytes: 32 bits */
                val = ((val & 0x000000FFU) << 24) | ((val & 0x0000FF00U) << 8)  | ((val & 0x00FF0000U) >> 8)  | ((val & 0xFF000000U) >> 24); 
                sprintf(out_ptr, "%08X", val);
                out_ptr += 8;
                break;
            default: break;
        }
    }

    *out_ptr = '\0'; /* Null-terminate final output string */
}
