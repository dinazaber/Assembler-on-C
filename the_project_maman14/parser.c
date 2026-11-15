/* 
Parser Module. 
Here we check label declaration and decide what type of command we have(data or code).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "symbol_table.h"
#include "cmd.h"
#include "directive.h"

#define MAX_LABEL_LEN 31

/* Check if given label name is valid */
int is_valid_label_chars(const char *start, size_t len){
  size_t k; /* Loop counter for checking label character validity */
  if (len == 0 || len > MAX_LABEL_LEN || !isalpha((unsigned char)start[0])) return 0;
  for (k = 1; k < len; k++){
      if (!isalnum((unsigned char)start[k])) return 0;
  }
  return 1;
}

/* Returns name of the label used(the one that will be added in symbol table) */
char *extract_label(char **line_ptr, InputError *err) {
  char *start = *line_ptr; /* Points to starting character of potential label */
  char *colon_pos; /* Pointer location of colon character(':') in line */
  char *label_str; /* Allocated buffer string for extracted label name */
  size_t label_len; /* Length of extracted label in characters */

  while (isspace((unsigned char)*start)) start++; /* Skip white spaces */
  /* Check if there is a label(should end with ':') */
  colon_pos = strchr(start, ':');
  if (colon_pos == NULL){
      *err = NO_ERROR;
      return NULL;
  }

  label_len = (size_t)(colon_pos - start);
  /* If the label is invalid, print error */
  if (!is_valid_label_chars(start, label_len)){
      *err = INVALID_LABEL;
      return NULL;
  }

  label_str = (char *)malloc(label_len + 1); /* Allocate memory */
  if (label_str == NULL){
      fprintf(stderr, "Fatal: Out of memory\n");
      exit(1);
  }

  strncpy(label_str, start, label_len); /* copy label name into label_str */
  label_str[label_len] = '\0'; /* Add null terminator */
  *line_ptr = colon_pos + 1; /* Advance line pointer right after ':' */
  *err = NO_ERROR;
  return label_str;
}

/*
A transition point, if data, check data and if entry/extern check them.
DC is updated here.
*/
void handle_data_directive(char *curr_ptr, char *token, DirectiveType dir, char *label, hash_map hm, int *DC, InputError *err, list_ptr data_list){
  int bytes_added = 0; /* Tracks memory bytes allocated by directive */
  char *args_ptr = strstr(curr_ptr, token) + strlen(token); /* Points to arguments following directive token */

  if (dir == DIRECTIVE_EXTERN || dir == DIRECTIVE_ENTRY){
      /* Label declaration in the same line as .entry / .extern, prints warning for meaningless label that is not stored in symbol table */
      if (label != NULL){
          printf("[WARNING]: Meaningless label \"%s\" preceding directive %s. Ignoring label.\n", label, token);
          free(label);
          label = NULL;
      }
      
      
      /* If .extern or .entry, check the syntax add them to symbol table if no errors. */
      if (process_data_directive(args_ptr, dir, &bytes_added, err, data_list)){
          if (dir == DIRECTIVE_EXTERN) handle_extern_directive(args_ptr, hm, err);
          else handle_entry_directive(args_ptr, hm,  err);
      } 
      return;
  }
  
  /* If the label was declared before, return error */
  if (label != NULL && !save_label(label, hm, *DC, DATA, err)) return;
  
  /* If there were no errors during data checks, increment DC by the amount of bytes used. */
  if (process_data_directive(args_ptr, dir, &bytes_added, err, data_list)) *DC += bytes_added;
}

/* Executes checks for code commands */
void execute_code_instruction(Command cmd_name, int i, char *curr_ptr, int *IC, InputError *err, list_ptr code_list, cmd_label used_labels, int line){
  /* Check command syntax */
  scan_command(cmd_name, &i, curr_ptr, err, *IC, code_list, used_labels, line);
  
  /* If there were no errors, increment IC */
  if (*err == NO_ERROR)
      *IC += 4;
}

/* Decides if we run syntax check on commands */
void handle_code_instruction(char *curr_ptr, char *label, hash_map hm, int *IC, InputError *err, list_ptr code_list, cmd_label used_labels, int line){
  Command cmd_name; /* Identified command type enum value */
  int i = 0; /* Parsing character offset counter */
  
  /* If the label exist in symbol table, return with error */
  if (label != NULL && !save_label(label, hm, *IC, CODE, err)) return;

  cmd_name = identify_command(&i, curr_ptr); /* Identify command */
  if (cmd_name == UNDEFINED_COMMAND) *err = INVALID_COMMAND; /* If the command name is invalid, change error code */
  /* If command name is valid, check its syntax */
  else if (cmd_name != EMPTY_LINE) execute_code_instruction(cmd_name, i, curr_ptr, IC, err, code_list, used_labels, line);
}

/* function called in main, here we extract the declared label(if exists) and decide if the command is data or code */
void process_line(char *line, int line_count, hash_map hm, int *IC, int *DC, InputError *err, list_ptr code_list, list_ptr data_list, cmd_label used_labels){
  DirectiveType dir; /* Matched directive enum type */
  char token[64]; /* Buffer holding extracted first command/directive token */
  char *curr_ptr = line; /* Traversal pointer for scanning line buffer */
  char *label; /* Pointer to extracted line label string */
  int line_len;

  *err = NO_ERROR; /* Reset or initialize error state for the current line */
  line_len = strlen(line);
  
  while (isspace((unsigned char)*curr_ptr)) curr_ptr++; /* Skip white spaces */
  if (*curr_ptr == '\0' || *curr_ptr == ';' || *curr_ptr == '\n') return; /* If the line is a comment or empty, skip it */
  
  printf("\n-------------------------------------------------------\nProcessing line %d: %s", line_count, line); /* Print the line that is processed */
  
  /* If command line contains more than 80 characters, return with error */
  if (line_len > MAX_LINE_LENGTH){
    *err = LINE_OVERFLOW;
    return;
  }

  label = extract_label(&curr_ptr, err); /* Extract label */
  if (*err != NO_ERROR) return; /* If there are errors with the label, stop processing the line */

  while (isspace((unsigned char)*curr_ptr)) curr_ptr++; /* Skip white spaces */
  
  if (sscanf(curr_ptr, "%63s", token) == 1 && (dir = get_directive_type(token)) != DIRECTIVE_INVALID) handle_data_directive(curr_ptr, token, dir, label, hm, DC, err, data_list);
  
  else handle_code_instruction(curr_ptr, label, hm, IC, err, code_list, used_labels, line_count);
}
