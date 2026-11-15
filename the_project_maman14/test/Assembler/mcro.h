#ifndef MCRO_H
#define MCRO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmd.h"
#include "hash_map.h"
#include "errors.h"

#define MAX_FILE_NAME 128
#define MAX_LINE_LEN_PRE 128
#define MAX_LINE_COUNT 1024
#define MAX_MCRO_NAME 64

typedef char str_list[MAX_LINE_COUNT][MAX_LINE_LEN_PRE];
typedef str_list *str_list_ptr;
typedef struct {

	str_list lines;
	int mcro_line_count;

} mcro, *mcro_ptr; /* macro container */

void open_mcro(char *file_name, InputError *error, int *line_count); /* creates the macro-parsed file */
void read_mcro(FILE* og_file_ptr, char *mcro_name, hash_map mcro_table, int *line_count, InputError *error); /* reads the macro's lines */
void write_mcro(FILE* p_mcro_ptr, mcro_ptr m_ptr); /* writes the macro's lines to the new file */
void get_macro_name(int *index, const char *line, char *macro_name_out); /* gets the macro's name to the third argument */

#endif
