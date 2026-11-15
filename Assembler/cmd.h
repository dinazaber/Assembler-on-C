/* This file contains methods to check the syntax of each line, saving labels to the symbol table, and saving partial assembly results. */
#ifndef CMD_H
#define CMD_H

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "lst.h"
#include "used_labels_list.h"
#define MAX_REG 3
#define HEX_STR 9

typedef enum {

    /* R Type */
    ADD, SUB, AND, OR, NOR, MOVE, MVHI, MVLO,

    /* I type */
    ADDI, SUBI, ANDI, ORI, NORI, BNE, BEQ, BLT, BGT, LB, SB, LW, SW, LH, SH,

    /* J type */
    JMP, LA, CALL, HLT,
    
    /* mcro */
    MCRO, MCROEND,

    /* Other */
    UNDEFINED_COMMAND, EMPTY_LINE
    
} Command; /* enumerates the commands */





typedef struct {

    char *name;
    Command cmd_enum;
    
} CommandMap;

typedef enum{
		TYPE_LABEL, TYPE_INT
	} type_input; /* used in the label container to determine its type */

typedef struct {

	union{
	
		char label_name[64];
		int imm_value;
	
	} value;
	
	type_input type;
	
	
} immediate_param; /* label container */

static const CommandMap lookup[] = { /* static table which maps the command's name to its corresponding enum */

    /* R type */
    {"add", ADD}, {"sub", SUB}, {"and", AND}, {"or", OR},
    {"nor", NOR}, {"move", MOVE}, {"mvhi", MVHI}, {"mvlo", MVLO},

    /* I type */
    {"addi", ADDI}, {"subi", SUBI}, {"andi", ANDI}, {"ori", ORI},
    {"nori", NORI}, {"bne", BNE}, {"beq", BEQ}, {"blt", BLT},
    {"bgt", BGT}, {"lb", LB}, {"sb", SB}, {"lw", LW},
    {"sw", SW}, {"lh", LH}, {"sh", SH},

    /* J type */
    {"jmp", JMP}, {"la", LA}, {"call", CALL}, {"hlt", HLT},
    
    /* mcro */
    {"mcro", MCRO}, {"mcroend", MCROEND}
    
};

static const size_t lookup_size = sizeof(lookup) / sizeof(lookup[0]);

Command identify_command(int *i, char command[]); /* identifies the command, checking the validity of it's name */
void scan_command(Command cmd, int *i, char command[], InputError *error, int IC, list_ptr code_list, cmd_label used_labels, int line); /* wrapper method that scans the line */
InputError go_to_reg(int *i, char command[]); /* moves the index to the next register */
InputError go_to_comma(int *i, char command[]); /* moves the index to the next comma */
InputError go_to_num(int *i, char command[]); /* moves the index to the next number */
InputError go_to_label(int *i, char command[]); /* moves the index to the next label name */
long get_num(int *i, char command[]); /* gets the number from the line */
char *get_label(int *i, char command[]); /* gets the label from the line */
InputError check_end(int *i, char command[]); /* checks the end of the line for garbage characters */

void scan_comma(int *i, char command[], InputError *error); /* wrapper method for going to a comma and receiving syntax errors */
void scan_reg(int *i, char command[], int *reg, InputError *error); /* wrapper method for going to a register and receiving syntax errors */
void scan_num(int *i, char command[], long *imm, InputError *error); /* wrapper method for going to a number and receiving syntax errors */
char *scan_label(int *i, char command[], InputError *error); /* wrapper method for going to a label and receiving syntax errors */

int opcode(Command cmd); /* calculates the opcode value of a command */
int funct(Command cmd); /* calculates the funct value of a command */

void scan_line_R_type(int *i, char command[], int opcode, int funct, int reg_amount, InputError *error, list_ptr code_list); /* wrapper method that scans an R type line */
void scan_line_I_type_label(int *i, char command[], int opcode, InputError *error, int IC, list_ptr code_list, cmd_label used_labels, int line); /* wrapper method that scans an I type line (with label) */
void scan_line_I_type_imm(int *i, char command[], int opcode, InputError *error, list_ptr code_list); /* wrapper method that scans an I type line (without label) */
void scan_line_J_type(int *i, char command[], int opcode, InputError *error, Command isJMP, list_ptr code_list, cmd_label used_labels, int IC, int line); /* wrapper method that scans a J type line */

void int_to_hex(int num, char res[], int hex_len); /* turns a binary string (num) to an ASCII hex string */

void R_type_to_hex(char res[HEX_STR], int regs[MAX_REG], int opcode, int funct); /* gets R type's parameters and turns them into an ASCII hex string */
void I_type_no_label_to_hex(char res[HEX_STR], int regs[MAX_REG-1], long imm, int opcode); /* gets I type's parameters and turns them into an ASCII hex string */
void I_type_with_label_to_hex(char res[50], int regs[MAX_REG-1], int opcode, char *label, int cmd_add); /* gets J type's parameters (with label name) and turns them into an ASCII hex string */
void J_type_to_hex(char res[40], int opcode, int is_reg, int reg, char *label); /* gets J type's parameters (general) and turns them into an ASCII hex string */

int little_endian(int val); /* correction for little endian */


#endif /* CMD_H */



