/* Symbol Table Module: Manages symbol storage, allocation, and label attribute state updates */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "symbol_table.h"
#include "errors.h"

void update_symbol_to_entry(Data *existing_data, const char *symbol){
    if (existing_data->att == CODE) { /* checking for attribute */
        existing_data->att = CODE_ENTRY;
    } 
    else if (existing_data->att == DATA){
        existing_data->att = DATA_ENTRY;
    }
}

int save_label(char *label, hash_map hm, int address, Attributes att, InputError *err){
    Data *existing_data = (Data *)get_data(hm, label); /* Points to existing entry in hash map if present */

    if (existing_data != NULL){ /* when there is a label with the same name: */
        if (existing_data->att == UNKNOWN){
            existing_data->mem_address = address; /* getting attribute */
            existing_data->att = (att == DATA) ? DATA_ENTRY : CODE_ENTRY;
            free(label);
            return 1;
        }
        *err = DOUBLE_LABEL;
        free(label);
        return 0;
    }

    create_and_insert_data(hm, label, address, att, err); /* saving label into symbol table */
    free(label);
    return 1;
}

char *extract_target_symbol(const char *args_ptr){
    const char *ptr = args_ptr; /* Traverses character string to scan target symbol */
    const char *start; /* Points to start position of symbol identifier */
    size_t len = 0; /* Stores length of target symbol name */
    char *target; /* Holds allocated copy of extracted target symbol */

    while (*ptr != '\0' && isspace((unsigned char)*ptr)) ptr++;
    start = ptr;
    while (*ptr != '\0' && !isspace((unsigned char)*ptr)){
        len++;
        ptr++;
    }

    if (len == 0) return NULL;

    target = (char *)malloc(len + 1);
    if (target == NULL){
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    strncpy(target, start, len);
    target[len] = '\0';
    return target;
}

void handle_extern_directive(char *args_ptr, hash_map hm, InputError *err){
    char *target_symbol = extract_target_symbol(args_ptr); /* Holds extracted target symbol string */
    Data *existing; /* Holds query result for target symbol from hash map */

    if (target_symbol == NULL) return;

    existing = (Data *)get_data(hm, target_symbol); /* finding "duplicates" */
    if (existing != NULL) {
        if (existing->att != EXTERNAL) *err = DOUBLE_LABEL;
        free(target_symbol);
        return;
    }

    create_and_insert_data(hm, target_symbol, 0, EXTERNAL, err); /* saving label into symbol table */
    free(target_symbol);
}

void handle_entry_directive(char *args_ptr, hash_map hm, InputError *err){
    char *target_symbol = extract_target_symbol(args_ptr); /* Holds extracted target symbol string */
    Data *existing; /* Holds query result for target symbol from hash map */

    if (target_symbol == NULL) return;

    existing = (Data *)get_data(hm, target_symbol); /* finding "duplicates" */
    if (existing != NULL){
        if (existing->att == EXTERNAL) *err = DOUBLE_LABEL;
        
        else update_symbol_to_entry(existing, target_symbol);
        
        free(target_symbol);
        return;
    }

    create_and_insert_data(hm, target_symbol, 0, UNKNOWN, err); /* saving label into symbol table */
    free(target_symbol);
}

Data *create_and_insert_data(hash_map hm, const char *symbol, int address, Attributes att, InputError *err){
    Data *d = (Data *)malloc(sizeof(Data));

    if (d == NULL){
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    d->mem_address = address;
    d->att = att;
    insert_key(hm, (char *)symbol, d, err); /* inserting created data */
    return d;
}

int validate_used_labels(cmd_label used_labels, hash_map symbol_map, int *has_extern){
    int error_count = 0;
    Node_ptr curr;

    if (used_labels == NULL || symbol_map == NULL) return 0;

    curr = used_labels->head;

    while (curr != NULL) {
        l_data_ptr used_item = curr->data_ptr;
        
        if (used_item != NULL && used_item->label_name != NULL) {
            /* Retrieve the Data_ptr paired to the label key */
            Data_ptr symbol_data = (Data_ptr)get_data(symbol_map, used_item->label_name);

            /* Error Check: Label not found in hash map */
            if (symbol_data == NULL) {
                print_error(UNDECLARED_LABEL, used_item->line);
                printf("%s\n", used_item->label_name);
                error_count++;
            } 
            else {
                /* Error Check: EXTERNAL label referenced in an I-type instruction */
                if (symbol_data->att == EXTERNAL && used_item->type == I_TYPE){
                    print_error(EXTERN_IN_I_TYPE, used_item->line);
                    printf("%s\n", used_item->label_name);
                    error_count++;
                }
                if (symbol_data->att == EXTERNAL && used_item->type == J_TYPE) *has_extern = 1;
            }
        }

        curr = curr->next;
    }

    return error_count;
}

int check_entries(hash_map h_map, int *has_entry){
    int i;
    int error_count = 0;
    node_ptr curr;
    Data_ptr data;

    *has_entry = 0;
    if (h_map == NULL) return 0;

    for (i = 0; i < HASH_SIZE; i++){ /* going trough all the buckets of the hash map */
        for (curr = h_map->ht[i]; curr != NULL; curr = curr->next){
            data = (Data_ptr)curr->data_ptr;
            if (data == NULL) continue;

            /* Check for undefined entry labels */
            if (data->att == UNKNOWN){
                printf("[ERROR]: Entry label '%s' was declared but never defined.\n", curr->key);
                error_count++;
            }

            /* Flag that we have at least one valid entry label to write */
            if (data->att == CODE_ENTRY || data->att == DATA_ENTRY) *has_entry = 1;
        }
    }

    return error_count; /* Returns total number of UNKNOWN label errors */
}


void update_data_label_addresses(hash_map h_map, int final_IC){ /* correction with final IC */
    int i;
    node_ptr curr;
    Data_ptr data;

    if (h_map == NULL) return;

    for (i = 0; i < HASH_SIZE; i++){ /* going trough all the buckets of the hash map */
        for (curr = h_map->ht[i]; curr != NULL; curr = curr->next){
            data = (Data_ptr)curr->data_ptr;
            if (data != NULL && (data->att == DATA || data->att == DATA_ENTRY)) data->mem_address += final_IC;
        }
    }
}

