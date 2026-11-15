#include "used_labels_list.h"

cmd_label new_cmd_label(){
    cmd_label cmd = calloc(1, sizeof(cmd_label_struct));
    
    if(cmd == NULL){
        printf("can't allocate memory");
        exit(1);
    }
    
    return cmd;
}

void insert_label(cmd_label cmd, char *label_name, int address, l_type type, int line){
    Node_ptr n_ptr;
    l_data_ptr data_ptr = malloc(sizeof(l_data));
    
    if(data_ptr == NULL){
        printf("can't allocate memory");
        exit(1);
    }
    
    data_ptr -> label_name = label_name;
    data_ptr -> address = address;
    data_ptr -> type = type;
    data_ptr -> line = line;
    
    n_ptr = malloc(sizeof(*n_ptr));
    
    n_ptr -> data_ptr = data_ptr;
    n_ptr -> next = (cmd -> head);
    
    (cmd -> head) = n_ptr;
    
    
}

void free_cmd_label(cmd_label cmd){
    Node_ptr ptr = cmd -> head;
    Node_ptr temp;
    
    while(ptr != NULL){
        l_data_ptr data = ptr -> data_ptr;
        free(data -> label_name);
        free(data);
        
        temp = ptr -> next;
        free(ptr);
        ptr = temp;
    }
    
    free(cmd);
}
