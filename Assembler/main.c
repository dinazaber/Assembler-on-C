/* 
Names: Shimon Beglyarov, Vladislav Surzhko

Main Module.
This program simulates assembler.
Here we go over all the input .as files
and for each file we run assembler.
The program begins with macro parsing, if it failed, we go to next file.
Opens generated .am file which contains the code from original file after macro parsing(pre processor) and
goes through every line and checks syntax, prints all the errors found with exact line in the code where it appears
and how many errors found.
If no errors found, output .ob file with machine code represented in hex.
Also optionally, outputs .ent file that contains names and addresses of all entry labels
and .ext file that contains names of external labels and addresses of J type instructions that use them.
Uses hash map for symbol table, dynamic string array for machine code and stack to store names of the labels
that instructions use(including line in code, if it J type or I type instruction and address of the instruction).

Assumptions: 
  * Length of the input file/path is up to 128 characters.
  * Input file has up to 1024 lines.
  * No macro calls inside macros.
  * Macro declaration always ends with 'mcroend'.
  * Length of a single line in input file is up to 128 characters(an error printed if a line
  contains more than 80 characters).
  * All the characters in input file are ASCII characters.
  * Should be executed on Linux.
*/

#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"
#include "parser.h"
#include "mcro.h"
#include "output_files.h"

#define MAX_LINE_LEN 128

/*
* Processes given file, runs assembler on that file.
* Algorithm: Checks macros first, if there is an error, returns.
Processes new generated .am file after pre processing, goes line by line
in this file and checks syntax, fills symbol table and makes machine code for instructions and data and prints errors.
Makes second pass checks(entry labels declared and instructions use declared labels).
Checks if there were errors and if yes, prints how many and returns.
If no errors, unites machine code into one string
and generates .ent(if there are entry labels), .ext(if external labels were used in instructions), .ob files.
*/
void process_file(char *filename);

/*
* Gets file name and returns new string without ending. Returns NULL if original file name is not ending with .as
* Input: filename (name of the file).
* Algorithm: Copy filename to new string, remove the ending after '.' and return the new string.
*/
char* remove_suffix(char *str);

/*
* Frees all the data.
* Input: hm (symbol table), code_list (instruction machine code), data_list (data machine code),
  machine_code (united machine code), used_labels (list of labels that were used in instructions), base_filename (string that stores base file name).
* Algorithm: Frees data by data.
*/
void free_all(hash_map hm, list_ptr code_list, list_ptr data_list, list_ptr machine_code, cmd_label used_labels, char *base_filename);

int main(int argc, char *argv[]){
  int i;
  
  /* No files given */
  if (argc < 2){
    fprintf(stderr, "Usage: %s <file1.as> [file2.as ...]\n", argv[0]);
    return 1;
  }
  
  /* Process every given file */
  for (i = 1; i < argc; i++){
    printf("\n========================================\n");
    printf(" Processing file: %s\n", argv[i]);
    printf("========================================\n");
    process_file(argv[i]);
  }
  
  return 0;
}


void process_file(char *filename){
  int line_count = 0; /* Tracks line number in source input file */
  int IC = 100; /* Instruction Counter starting at address 100 */
  int DC = 0; /* Data Counter tracking data byte offsets */
  char line[MAX_LINE_LEN]; /* Buffer reading line contents from source file */
  InputError err = NO_ERROR; /* Error code */
  int has_entry = 0; /* Flag that represents existence of declared .entry labels */
  int has_extern = 0; /* Flag that represents existence of used .extern labels  in J type instructions */
  int comp_err = 0; /* Number of compilation errors */
  char *base_filename = remove_suffix(filename); /* Use this for filenames later(without .as ending) */
  char am_filename[128]; /* String that stores name of generated .am file */
  FILE *file_ptr; /* File */
  
  hash_map hm = NULL; /* Symbol hash map storing labels and memory addresses */
  list_ptr code_list = NULL; /* Machine code string for instructions */
  list_ptr data_list = NULL; /* Machine code string for data */
  list_ptr machine_code = NULL; /* United machine code, first instructions code then data code */
  cmd_label used_labels = NULL; /* List that stores all the appearances of labels in instructions */
  
  if (base_filename == NULL){
    fprintf(stderr, "Wrong file ending for '%s'. Should end with .as.\n", filename);
    return;
  }
  
  open_mcro(filename, &err, &line_count);
  if (err != NO_ERROR){
    fprintf(stderr, "Macro expansion failed for %s\n", filename);
    free(base_filename);
    return;
  }
  line_count = 0; /* reset line count after macro */
  
  sprintf(am_filename, "%s.am", base_filename);
  file_ptr = fopen(am_filename, "r"); /* File handle pointing to input assembly file */

  if (file_ptr == NULL){
      printf("[ERROR] Can't open generated file '%s'\n", am_filename);
      return;
  }
  
  /* Initialize data */
  hm = create_empty_hash_map();
  code_list = new_list();
  data_list = new_list();
  used_labels = new_cmd_label();
  
  while (fgets(line, sizeof(line), file_ptr) != NULL){
      line_count++;
      process_line(line, line_count, hm, &IC, &DC, &err, code_list, data_list, used_labels); /* Process current line */
      print_error(err, line_count);
      if (err != NO_ERROR)comp_err += 1;
  }
  
  fclose(file_ptr);
  
  /* Check for undefined entry labels and that used labels are declared */
  comp_err += check_entries(hm, &has_entry);
  comp_err += validate_used_labels(used_labels, hm, &has_extern);
  
  /* If there are undefined entry labels, abort output generation */
  if (comp_err){ 
    fprintf(stderr, "Assembly failed for '%s' with %d errors.\n", filename, comp_err);
    free_all(hm, code_list, data_list, machine_code, used_labels, base_filename); /* Free all data */
    return; /* Process next file */
  }
  
  update_data_label_addresses(hm, IC); /* Add ICF to any data label's address */
  
  /* If no errors and entry labels exist, write the .ent file */
  if (has_entry) write_entry_file(hm, base_filename);
  
  /* If no errors and external labels were used, write the .ext file */
  if (has_extern) write_extern_file(used_labels, hm, base_filename);
  
  machine_code = list_merge(code_list, data_list); /* Merge 2 machine code strings into one */
  /* If no errors, write the .ob file */
  write_ob_file(hm, machine_code, base_filename, IC, DC);
  
  free_all(hm, code_list, data_list, machine_code, used_labels, base_filename); /* Free all data at the end */
  
  printf("\nAssembler passed successfully with file '%s'\n", filename);
}

/* Get base file name without ending */
char* remove_suffix(char *str){
    int len = strlen(str);
    const int suffix_len = 3;
    char *res;

    if(len <= suffix_len || strcmp(str + len - suffix_len, ".as") != 0)
        return NULL;

    res = malloc(len * sizeof(char) - suffix_len + 1);

    if(res == NULL){
        printf("can't allocate memory\n");
        exit(1);
    }

    strncpy(res, str, len - suffix_len);
    res[len - suffix_len] = '\0';

    return res;
}

/* Frees all the data */
void free_all(hash_map hm, list_ptr code_list, list_ptr data_list, list_ptr machine_code, cmd_label used_labels, char *base_filename){
  free_hash_map(hm);
  free_cmd_label(used_labels);
  free_list(code_list);
  free_list(data_list);
  free(base_filename);
}
