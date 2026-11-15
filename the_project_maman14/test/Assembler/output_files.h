/*
Module that handles writing in output files (.ob / .ext / .ent)
Every writing function here creates relevant file by itself.
*/

#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include "hash_map.h"
#include "lst.h"
#include "used_labels_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*
* Creates .ent file and writes entry symbols to a .ent file.
* Input: h_map (symbol hash map), base_filename (output base name string without ending).
* Algorithm: Allocates filename string with ".ent", traverses hash map, filters nodes marked CODE_ENTRY or DATA_ENTRY, and writes "LABEL ADDRESS\n".
*/
void write_entry_file(hash_map h_map, char *base_filename);

/*
* Creates .ext file and writes used external label references to a .ext file.
* Input: used_labels (list of label usages), symbol_map (symbol hash map), base_name (output base name string).
* Algorithm: Formats filename with ".ext", loops through used_labels list, 
  verifies label is marked EXTERNAL in symbol_map, and writes "LABEL ADDRESS\n".
*/
void write_extern_file(cmd_label used_labels, hash_map symbol_map, char *base_name);

/*
* Drives creation of the final binary/object .ob file.
* Input: h_map (symbol hash map), lst_p (machine code buffer), base_filename (base string), IC (instruction count), DC (data count).
* Algorithm: Creates ".ob" file, outputs header "IC-100 DC", reads lst_p byte-by-byte,
  calls process_raw_hex for normal code or process_label_token for '/' label markers.
*/
void write_ob_file(hash_map h_map, list_ptr lst_p, char *base_filename, int IC, int DC);

/*
* Writes formatted hex pairs to file with line address headers.
* Input: fp (file pointer), hex_str (hex string), len (string length), bytes_in_line (line byte counter ptr), current_addr (memory addr ptr).
* Algorithm: Iterates hex string 2 chars at a time. If start of line (bytes_in_line == 0),
  prints 4-digit address. Writes hex pair. 
  At 4 bytes, prints newline, resets count, adds +4 to address.
*/
void write_hex_bytes(FILE *fp, const char *hex_str, int len, int *bytes_in_line, int *current_addr);

/*
* Converts an integer memory address into a little-endian hex string.
* Input: hex_buf (output buffer), val (address integer), is_i_type (flag: 1 for 2-byte I-type, 0 for 3-byte J-type).
* Algorithm: Uses bitwise masks (& 0xFF) and shifts (>> 8, >> 16) with sprintf to write 
  bytes in least significant to most significant order into hex_buf.
*/
void encode_label_hex(char *hex_buf, int val, int is_i_type);

/*
* Parses a label token in code stream, resolves its address, and writes its hex.
* Input: fp (file ptr), p_ptr (double ptr to code stream), h_map (hash map), bytes_in_line (ptr), current_addr (ptr).
* Algorithm: Advances past '/', reads label string until '-' or '/'. If '-' exists, parses
  offset and marks I-type. Lookups label address in h_map, calculates final value,
  calls encode_label_hex, and outputs via write_hex_bytes.
*/
void process_label_token(FILE *fp, char **p_ptr, hash_map h_map, int *bytes_in_line, int *current_addr);

/*
* Reads 2 hex characters from stream and outputs them.
* Input: fp (file ptr), p_ptr (double ptr to code stream), bytes_in_line (ptr), current_addr (ptr).
* Algorithm: Copies next 2 chars from *p_ptr into a buffer, null-terminates, advances 
  *p_ptr by 2, and passes buffer to write_hex_bytes.
*/
void process_raw_hex(FILE *fp, char **p_ptr, int *bytes_in_line, int *current_addr);

#endif /* OUTPUT_FILES_H */
