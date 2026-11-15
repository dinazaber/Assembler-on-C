#ifndef USED_LABELS_LIST_H
#define USED_LABELS_LIST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {I_TYPE, J_TYPE} l_type;

typedef struct {
    char *label_name;
    int address;
    l_type type;
    int line;
    
}l_data, *l_data_ptr;

typedef struct Node{
    l_data_ptr data_ptr;
    struct Node *next;
} *Node_ptr;

typedef struct{
    Node_ptr head;
} cmd_label_struct;

typedef cmd_label_struct *cmd_label;

cmd_label new_cmd_label();
void insert_label(cmd_label cmd, char *label_name, int address, l_type type, int line);
void free_cmd_label(cmd_label cmd);

#endif /* USED_LABELS_LIST_H */
