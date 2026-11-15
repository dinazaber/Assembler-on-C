#ifndef PARSER_H
#define PARSER_H

#include "hash_map.h"
#include "errors.h"
#include "cmd.h"
#include "directive.h"
#include "symbol_table.h"

/*
 Validates whether a character sequence adheres to syntax rules for label identifiers.

 Input/Output:
 - Input: start (const char* pointing to label start), len (size_t label length).
 - Output: Returns 1 if valid label syntax, 0 otherwise.

 How it works:
 Verifies that length is between 1 and MAX_LABEL_LEN and that the first character is alphabetic.
 Loops through the remaining characters to ensure they are all alphanumeric.
*/
int is_valid_label_chars(const char *start, size_t len);

/*
 Parses a leading label definition from the start of an assembly line if present.

 Input/Output:
 - Input: line_ptr (pointer to char* line pointer), err (InputError* output parameter).
 - Output: Returns dynamically allocated label string, or NULL if no label or error.
           Advances *line_ptr past the colon on success.

 How it works:
 Looks for a colon in the line buffer. If found, validates characters prior to colon.
 On success, allocates memory, copies label text, updates error code, and advances the
 line pointer past the colon.
*/
char *extract_label(char **line_ptr, InputError *err);

/*
 Processes assembler directives (.data, .string, .entry, .extern) and updates table/data counts.

 Input/Output:
 - Input: curr_ptr (char* line position), token (char* directive name), dir (DirectiveType),
           label (char* optional label), hm (hash_map), DC (int* Data Counter pointer),
           line_count (int line number).
 - Output: Updates DC, modifies hash map, or prints appropriate errors/warnings (void).

 How it works:
 For .entry/.extern directives, warns if a label is present (ignoring/freeing it), parses arguments,
 and invokes handle_extern_directive or handle_entry_directive. For regular data directives,
 saves any associated label with DATA attribute, processes arguments to calculate byte size,
 and increments the DC counter accordingly.
*/
void handle_data_directive(char *curr_ptr, char *token, DirectiveType dir, char *label, hash_map hm, int *DC, InputError *err, list_ptr data_list);

/*
 Parses arguments for a valid code instruction, encodes it into hex, and increments IC.

 Input/Output:
 - Input: cmd_name (Command enum), i (int character offset), curr_ptr (char* line position),
           IC (int* Instruction Counter pointer), line_count (int current line number).
 - Output: Updates IC by 4 and outputs debug information (void).

 How it works:
 Resolves opcode and funct values for the command, then calls scan_command to parse registers and
 arguments. If parsing succeeds, encodes the operation via R_type_to_hex, prints the hex output,
 and increments IC by 4 bytes.
*/
void execute_code_instruction(Command cmd_name, int i, char *curr_ptr, int *IC, InputError *err, list_ptr code_list, cmd_label used_labels, int line);

/*
 Saves optional code labels and triggers parsing/execution for instruction lines.

 Input/Output:
 - Input: curr_ptr (char* line position), label (char* optional extracted label),
           hm (hash_map), IC (int* Instruction Counter pointer), line_count (int line number).
 - Output: Updates symbol table or delegates execution to execute_code_instruction (void).

 How it works:
 If a label is present, attempts to save it in the symbol table with CODE attribute and current IC address.
 Identifies the command name; if invalid, logs an error, otherwise invokes execute_code_instruction
 for valid code instructions.
*/
void handle_code_instruction(char *curr_ptr, char *label, hash_map hm, int *IC, InputError *err, list_ptr code_list, cmd_label used_labels, int line);

/*
 Main Pass 1 parser for a single line of assembly input.

 Input/Output:
 - Input: line (char* string), line_count (int line number), hm (hash_map),
          IC (int* instruction counter), DC (int* data counter).
 - Output: Updates IC, DC, and symbol table in-place (void).

 How it works:
 Skips whitespace/comments, checks for and extracts a label prefix, determines
 whether line is a directive or code instruction, and delegates processing to
 specialized handler routines.
*/
void process_line(char *line, int line_count, hash_map hm, int *IC, int *DC, InputError *err, list_ptr code_list, list_ptr data_list, cmd_label used_labels);

#endif /* PARSER_H */
