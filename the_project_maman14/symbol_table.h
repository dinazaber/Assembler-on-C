#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "hash_map.h"
#include "parser.h"

/*
 Promotes an existing local symbol's attribute state to an entry state.

 Input/Output:
 - Input: existing_data (Data* node pointer), symbol (const char* identifier).
 - Output: Modifies the att field of existing_data in-place (void).

 How it works:
 Checks if the current attribute is CODE or DATA, and updates it to CODE_ENTRY
 or DATA_ENTRY respectively, logging a debug message upon completion.
*/
void update_symbol_to_entry(Data *existing_data, const char *symbol);

/*
 Saves a new label definition into the symbol table or resolves a pending entry.

 Input/Output:
 - Input: label (char* string), hm (hash_map), address (int memory offset),
          att (Attributes type), line_count (int current line number).
 - Output: Returns 1 on successful save or update, 0 on failure/duplicate error.
           Frees the label memory on all exit paths.

 How it works:
 Queries the hash map for an existing symbol. If found with UNKNOWN attribute,
 updates its memory address and promotes it to CODE_ENTRY/DATA_ENTRY. If found with
 another attribute, reports a DOUBLE_LABEL error. If not found, allocates and
 inserts a new symbol entry.
*/
int save_label(char *label, hash_map hm, int address, Attributes att, InputError *err);

/*
 Scans an input string, skips leading whitespace, and allocates a heap copy 
 of the first whitespace-delimited word encountered.

 Input/Output:
 - Input: args_ptr (const char* pointing to raw argument text).
 - Output: Dynamically allocated string (char*) containing the extracted symbol,
           or NULL if no symbol is found. Terminates the program on allocation failure.

 How it works:
 Advances the input pointer past any leading whitespace characters. Measures the
 length of the non-whitespace identifier token that follows. Returns NULL if the
 length is zero. Otherwise, allocates memory for the token plus a null-terminator,
 copies the string slice, and returns the result.
*/
char *extract_target_symbol(const char *args_ptr);

/*
 Processes an .extern directive argument and registers the external symbol.

 Input/Output:
 - Input: args_ptr (char* string following directive), hm (hash_map),
          line_count (int line number).
 - Output: Updates the hash map with the external symbol (void).

 How it works:
 Extracts the target symbol identifier from arguments. If the symbol already
 exists with a non-EXTERNAL attribute, reports a DOUBLE_LABEL error. Otherwise,
 creates a new symbol entry with address 0 and EXTERNAL attribute.
*/
void handle_extern_directive(char *args_ptr, hash_map hm, InputError *err);

/*
 Processes an .entry directive argument and updates or adds pending entry symbols.

 Input/Output:
 - Input: args_ptr (char* string following directive), hm (hash_map),
          line_count (int line number).
 - Output: Updates or creates a symbol table record in the hash map (void).

 How it works:
 Extracts the target symbol identifier from arguments. If the symbol already exists,
 verifies it is not EXTERNAL and updates its status via update_symbol_to_entry. If it
 does not exist yet, creates a pending entry record with address 0 and UNKNOWN attribute.
*/
void handle_entry_directive(char *args_ptr, hash_map hm, InputError *err);

/*
 Dynamically allocates a Data struct and inserts it into the symbol hash map.

 Input/Output:
 - Input: hm (hash_map), symbol (const char* key), address (int memory address),
          att (Attributes type).
 - Output: Returns pointer to the newly allocated Data struct.

 How it works:
 Allocates memory for a Data struct, sets its address and attribute fields, and
 calls insert_key to store the entry in the hash map.
*/
Data *create_and_insert_data(hash_map hm, const char *symbol, int address, Attributes att, InputError *err);

/*
* Checks symbol table for entry label errors and detects if valid entries exist.
* Input: h_map (symbol hash map), has_entry (out-flag ptr for valid entry labels).
* Algorithm: Initializes *has_entry to 0. Iterates all hash map nodes:
  If label attribute is UNKNOWN (declared .entry but never defined), logs an error and increments count.
  If attribute is CODE_ENTRY or DATA_ENTRY, sets *has_entry to 1.
* Returns: Total number of undefined entry label errors.
*/
int check_entries(hash_map h_map, int *has_entry);

/*
* Relocates data segment label addresses by adding the final Instruction Counter offset.
* Input: h_map (symbol hash map), final_IC (final instruction count offset).
* Algorithm: Scans all buckets in the symbol hash map. For each symbol with an attribute of
  DATA or DATA_ENTRY, adds final_IC to its mem_address field.
 */
void update_data_label_addresses(hash_map h_map, int final_IC);

/*
* Validates label references used in assembly instructions and flags external usage.
* Input: used_labels (list of label usages), symbol_map (symbol hash map), has_extern (out-flag ptr for external labels).
* Algorithm: Loops through used_labels list, looks up each label in symbol_map.
  Triggers errors if label is undeclared or if an EXTERNAL label is used in an I-TYPE instruction.
  Sets *has_extern to 1 if no errors are encountered.
* Returns: Total number of errors found (0 if valid).
*/
int validate_used_labels(cmd_label used_labels, hash_map symbol_map, int *has_extern);

#endif /* SYMBOL_TABLE_H */
