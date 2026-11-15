#include "cmd.h"

static Command match_lookup(const char *cmd_start, int len){
    int idx;
    for (idx = 0; idx < lookup_size; idx++) { /* goes trough the lookup table */
        if (len == (int)strlen(lookup[idx].name) && strncmp(cmd_start, lookup[idx].name, len) == 0) return lookup[idx].cmd_enum;
    }
    return UNDEFINED_COMMAND;
}

Command identify_command(int *command_pointer, char command[]){
    int start_idx, len;

    while (isspace((unsigned char)command[*command_pointer])) (*command_pointer)++; /* skipping whitespaces */
    start_idx = *command_pointer;

    if (command[start_idx] == '\0' || command[start_idx] == '\n' || command[start_idx] == EOF) return EMPTY_LINE;

    while (command[*command_pointer] != '\0' && !isspace((unsigned char)command[*command_pointer])) (*command_pointer)++; /* finding potential's command end */

      len = *command_pointer - start_idx;
      return match_lookup(command + start_idx, len);
}

InputError go_to_reg(int *i, char command[]){ 
    while (isspace((unsigned char)command[*i])) (*i)++; /* skipping whitespaces */
    
    if (command[*i] != '$') return MISSING_REG;
    
    (*i)++;
    if (isdigit((unsigned char)command[*i])) return NO_ERROR; /* checking for correct syntax */
    return UNDEFINED_REG;
}

InputError go_to_num(int *i, char command[]){
    while (isspace((unsigned char)command[*i])) (*i)++; /* skipping whitespaces */
    
    if (command[*i] == ',') return CONSECUTIVE_COMMAS;
    
    if (command[*i] == '-' || isdigit((unsigned char)command[*i])) return NO_ERROR;
    
    return MISSING_PARAM;
}

InputError go_to_label(int *i, char command[]){
    while (isspace((unsigned char)command[*i])) (*i)++; /* skipping whitespaces */
    
    if (command[*i] == ',') return CONSECUTIVE_COMMAS;
    
    if (isalpha((unsigned char)command[*i])) return NO_ERROR;
    
    if (command[*i] == '\0') return MISSING_LABEL;
    
    return INVALID_LABEL;
}

InputError go_to_comma(int *i, char command[]){
    while (isspace((unsigned char)command[*i])) (*i)++; /* skipping whitespaces */
    if (command[*i] == ',') {
        (*i)++;
        return NO_ERROR;
    }
    if (command[*i] == '\n' || command[*i] == EOF || command[*i] == '\0') return MISSING_PARAM;
    
    return MISSING_COMMA;
}

long get_num(int *i, char command[]){
    char *end_ptr;
    long val = strtol(command + *i, &end_ptr, 10);
    *i = (int)(end_ptr - command);
    return val;
}

char *get_label(int *i, char command[]){
    char *start_ptr = command + *i;
    size_t len = strcspn(start_ptr, " \t\n\r,");
    char *label_buf;

    if (len == 0) return NULL;
    label_buf = (char *)malloc(len + 1);
    if (label_buf == NULL){
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    strncpy(label_buf, start_ptr, len); /* copying the label into a buffer */
    label_buf[len] = '\0';
    *i += (int)len;
    return label_buf;
}

InputError check_end(int *i, char command[]){
    while (isspace((unsigned char)command[*i])) (*i)++; /* skipping whitespaces */
    if (command[*i] == '\n' || command[*i] == EOF || command[*i] == '\0') return NO_ERROR;
    return EXTRA_TEXT;
}

void scan_comma(int *i, char command[], InputError *error){
    *error = go_to_comma(i, command);
}

void scan_num(int *i, char command[], long *imm, InputError *error){
    *error = go_to_num(i, command);
    *imm = get_num(i, command);
}

static int is_valid_label_chars(const char *label){
    int j = 0;
    while (label[j] != '\0'){ /* checking for numbers */
        if (!isalnum((unsigned char)label[j])) return 0;
        j++;
    }
    return 1;
}

char *scan_label(int *i, char command[], InputError *error){
    char *label = NULL;
    *error = go_to_label(i, command);
    if (*error != NO_ERROR) return NULL;

    label = get_label(i, command); /* get label's name */
    if (label == NULL || !is_valid_label_chars(label)){
        if (label != NULL) free(label);
        *error = INVALID_LABEL;
        return NULL;
    }
    return label;
}

void scan_reg(int *i, char command[], int *reg, InputError *error){
    *error = go_to_reg(i, command);
    if (*error != NO_ERROR) return;
    *reg = (int)get_num(i, command);
    if (*reg > 31 || *reg < 0) *error = OUT_OF_BOUNDS_REG; /* checking the register's syntax */
}

void scan_line_R_type(int *i, char command[], int opcode, int funct, int reg_amount, InputError *error, list_ptr code_list){
    int regs[MAX_REG] = {0};
    char cmd_hex[HEX_STR];
    int index;
    for (index = 0; index < reg_amount; index++){ /* finding all the registers */
        scan_reg(i, command, &(regs[index]), error);
        if (*error != NO_ERROR) return;

        if (index == reg_amount - 1) check_end(i, command);
        else scan_comma(i, command, error);

        if (*error != NO_ERROR) return;
    }
    if (reg_amount == 2){ /* Used for R type instructions with 2 registers */
      int temp = regs[1];
      regs[1] = 0;
      regs[2] = temp;
    }
    
    *error = check_end(i, command);
    
    R_type_to_hex(cmd_hex, regs, opcode, funct); /* converting the registers to hex */
  
    list_write(code_list, cmd_hex, strlen(cmd_hex)); /* writing to the result to the hex list */
}

void scan_line_I_type_imm(int *i, char command[], int opcode, InputError *error, list_ptr code_list){
    int regs[2];
    long imm;
    char cmd_hex[HEX_STR];
    scan_reg(i, command, &(regs[0]), error); /* finding first register */
    if (*error != NO_ERROR) return;
    scan_comma(i, command, error);
    if (*error != NO_ERROR) return;

    scan_num(i, command, &imm, error); /* finding the imm value */
    if (*error != NO_ERROR) return;
    scan_comma(i, command, error);
    if (*error != NO_ERROR) return;

    scan_reg(i, command, &(regs[1]), error); /* finding second register */
    if (*error != NO_ERROR) return;
    *error = check_end(i, command);
    
    if (*error == NO_ERROR){
      I_type_no_label_to_hex(cmd_hex, regs, imm, opcode); /* converting result to hex */
      list_write(code_list, cmd_hex, strlen(cmd_hex)); /* writing result to hex list */
    }
}

void scan_line_I_type_label(int *i, char command[], int opcode, InputError *error, int IC, list_ptr code_list, cmd_label used_labels, int line){
    int regs[2];
    char cmd_hex[50]; /* size of label(31) + 2 * '/' + '-' + 8 * number(decimal for address) */
    char *label = NULL;
    scan_reg(i, command, &(regs[0]), error); /* finding first register */
    if (*error != NO_ERROR) return;
    scan_comma(i, command, error);
    if (*error != NO_ERROR) return;

    scan_reg(i, command, &(regs[1]), error); /* finding second register */
    if (*error != NO_ERROR) return;
    scan_comma(i, command, error);
    if (*error != NO_ERROR) return;

    label = scan_label(i, command, error); /* finding label name */
    if (*error != NO_ERROR) return;
    *error = check_end(i, command);
    
    if (*error == NO_ERROR){ /* saving results */
      insert_label(used_labels, label, IC, I_TYPE, line);
      I_type_with_label_to_hex(cmd_hex, regs, opcode, label, IC+4);
      list_write(code_list, cmd_hex, strlen(cmd_hex));
    }
}

void scan_line_J_type(int *i, char command[], int opcode, InputError *error, Command isJMP, list_ptr code_list, cmd_label used_labels, int IC, int line){
    int tempI = *i;
    int reg[1] = {0};
    char cmd_hex[40]; /* size of label(31) + 2 hex chars + 2 * '/' + '\0' */
    char *label = NULL;
    
    if (isJMP == HLT){ /* edge case for halt command */
      J_type_to_hex(cmd_hex, opcode, 0, 0, NULL);
      list_write(code_list, cmd_hex, strlen(cmd_hex));
      return;
    }

    label = scan_label(i, command, error);
    
    if (*error != NO_ERROR && isJMP == JMP){ /* edge case for jump command */
        scan_reg(&tempI, command, &(reg[0]), error);
        if (*error != NO_ERROR) return;
        *i = tempI;
    }
    if (*error == NO_ERROR) *error = check_end(i, command);
    
    if (*error == NO_ERROR){ /* other jump commands */
      if (reg[0]) J_type_to_hex(cmd_hex, opcode, 1, reg[0], NULL);
      else J_type_to_hex(cmd_hex, opcode, 0, 0, label);
      
      insert_label(used_labels, label, IC, J_TYPE, line);
      
      list_write(code_list, cmd_hex, strlen(cmd_hex));
    }
}

void scan_command(Command cmd, int *i, char command[], InputError *error, int IC, list_ptr code_list, cmd_label used_labels, int line){
    int op = opcode(cmd);
    int fn = funct(cmd);
    
    /* choosing the correct wrapper function */
    
    if (ADD <= cmd && cmd <= NOR) scan_line_R_type(i, command, op, fn, 3, error, code_list);
    
    else if (MOVE <= cmd && cmd <= MVLO) scan_line_R_type(i, command, op, fn, 2, error, code_list);
    
    else if ((ADDI <= cmd && cmd <= NORI) || (LB <= cmd && cmd <= SH)) scan_line_I_type_imm(i, command, op, error, code_list);
    
    else if (BNE <= cmd && cmd <= BGT) scan_line_I_type_label(i, command, op, error, IC, code_list, used_labels, line);
    
    else if (JMP <= cmd && cmd <= HLT) scan_line_J_type(i, command, op, error, cmd, code_list, used_labels, IC, line);
    
    else if (cmd == UNDEFINED_COMMAND) printf("Undefined command found!\n");
}

int opcode(Command cmd){ /* calculates the opcode based on the order of the commands */
    if (ADD <= cmd && cmd <= NOR) return 0;
    if (MOVE <= cmd && cmd <= MVLO) return 1;
    if (ADDI <= cmd && cmd <= SH) return cmd + (10 - ADDI);
    if (JMP <= cmd && cmd <= CALL) return cmd + (30 - JMP);
    if (cmd == HLT) return 63;
    return -1;
}

int funct(Command cmd){ /* calculates the opcode based on the order of the commands */
    if (ADD <= cmd && cmd <= NOR) return cmd + (1 - ADD);
    if (MOVE <= cmd && cmd <= MVLO) return cmd + (1 - MOVE);
    return -1;
}

void int_to_hex(int num, char res[], int hex_len){ 
    int i;
    for (i = 0; i < hex_len; i++){
        int shift = 4 * (hex_len - i - 1);
        int val = (num >> shift) & 0xF; /* getting the the value of each hex digit */
        res[i] = (val <= 9) ? ('0' + val) : ('A' + val - 10); /* calculating the ASCII code of the value */
    }
    res[hex_len] = '\0';
}

void R_type_to_hex(char res[HEX_STR], int regs[MAX_REG], int opcode, int funct){
    int bin_str = (opcode << 26) | (regs[0] << 21) | (regs[1] << 16) | (regs[2] << 11) | (funct << 6); /* shifting the values to the correct position in the binary string */
    bin_str = little_endian(bin_str);
    int_to_hex(bin_str, res, 8);
}


void I_type_no_label_to_hex(char res[HEX_STR], int regs[MAX_REG-1], long imm, int opcode){
    int bin_str = (opcode << 26) | (regs[0] << 21) | (regs[1] << 16) | (imm & 0xFFFF); /* shifting the values to the correct position in the binary string */
    bin_str = little_endian(bin_str);
    int_to_hex(bin_str, res, 8);
}

void I_type_with_label_to_hex(char res[50], int regs[MAX_REG-1], int opcode, char *label, int cmd_add /* IC + 4 */){
      int upper_bits = ((opcode & 0x3F) << 10) | ((regs[0] & 0x1F) << 5) | (regs[1] & 0x1F); /* shifting the values to the correct position in the binary string */
      upper_bits = ((upper_bits & 0x00FFU) << 8) | ((upper_bits & 0xFF00U) >> 8);
      sprintf(res, "/%s-%d/%04X", label, cmd_add, upper_bits);
}

void J_type_to_hex(char res[40], int opcode, int is_reg, int reg, char *label){
    if (label != NULL){ /* there is a label name, not a register */
      int bin_str = ((opcode & 0x3F) << 2) | ((is_reg & 1) << 1); /* shifting the values to the correct position in the binary string */
      sprintf(res, "/%s/%02X", label, bin_str);
    }
    else{
      int bin_str = (opcode << 26) | ((is_reg & 1) << 25) | (reg & 0x1FFFFFF); /* shifting the values to the correct position in the binary string */
      bin_str = little_endian(bin_str);
      int_to_hex(bin_str, res, 8);
    }
}

int little_endian(int val){ /* shifting to little endian configuration */
    return ((val & 0x000000FFU) << 24) |
           ((val & 0x0000FF00U) << 8)  |
           ((val & 0x00FF0000U) >> 8)  |
           ((val & 0xFF000000U) >> 24);
}

