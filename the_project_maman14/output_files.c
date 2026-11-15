#include "output_files.h"

/* Creates and writes LABEL_NAME, ADDRESS into .ent file */
void write_entry_file(hash_map h_map, char *base_filename){
    int i;
    FILE *file; /* File */
    node_ptr curr; /* Pointer to current hash map data block */
    Data_ptr data; /* Current hash map data block */
    char *full_filename; /* String that stores output file name */

    full_filename = (char *)malloc(strlen(base_filename) + 5);
    if (full_filename == NULL){
      printf("Can't allocate memory.\n");
      return;
    }
    
    /* Create output file name */
    strcpy(full_filename, base_filename);
    strcat(full_filename, ".ent");

    file = fopen(full_filename, "w");
    if (file == NULL){
        printf("Can't create entry file.\n");
        free(full_filename);
        return;
    }

    for (i = 0; i < HASH_SIZE; i++){
        for (curr = h_map->ht[i]; curr != NULL; curr = curr->next){
            data = (Data_ptr)curr->data_ptr;
            /* If label exist in symbol table and is entry, print label name and address */
            if (data != NULL && (data->att == CODE_ENTRY || data->att == DATA_ENTRY)) fprintf(file, "%s\t%04d\n", curr->key, data->mem_address);
        }
    }

    fclose(file);
    free(full_filename);
}

/* Creates and writes into .ext file */
void write_extern_file(cmd_label used_labels, hash_map symbol_map, char *base_name){
    char filename[128]; /* String that stores name of the new file created */
    FILE *ext_file = NULL; /* File */
    Node_ptr curr; /* Current block of data from cmd_label */

    if (used_labels == NULL || symbol_map == NULL || base_name == NULL) return;
    
    /* Construct target filename */
    sprintf(filename, "%s.ext", base_name);
    ext_file = fopen(filename, "w");
    if (ext_file == NULL){
      fprintf(stderr, "Unable to create file %s\n", filename);
      return;
    }

    curr = used_labels->head;

    /* Traverse the list to check if any external labels exist */
    while (curr != NULL){
        l_data_ptr used_item = curr->data_ptr;

        if (used_item != NULL && used_item->label_name != NULL){
            Data_ptr symbol_data = (Data_ptr)get_data(symbol_map, used_item->label_name); /* Find the label in symbol table */
            /* Check if the label is EXTERNAL and Write: LABEL_NAME, ADDRESS_OF_INSTRUCTION */
            if (symbol_data != NULL && symbol_data->att == EXTERNAL) fprintf(ext_file, "%s %04d\n", used_item->label_name, used_item->address);
        }
        curr = curr->next; /* Go to next data block */
    }
    
    fclose(ext_file);
}

/* 
Primary driver: Opens target file, writes machine header, iterates memory 
list stream, delegates token parsing, and manages output cleanup.
*/
void write_ob_file(hash_map h_map, list_ptr lst_p, char *base_filename, int IC, int DC) {
    char filename[128]; /* String that stores name of the new file created */
    FILE *fp; /* File */
    char *p, *end_p; /* p is the pointer to current char in the machine code string, end_p points to its end */
    int current_addr = 100, /* Address that will be printed in .ob file, every time we write new line in .ob file, we add 4 to this value */
    bytes_in_line = 0; /* Number of bytes we printed in a line */

    if (!lst_p || !lst_p->lst) return; /* No machine code */

    sprintf(filename, "%s.ob", base_filename); /* Name of the new .ob file */
    fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create .ob file\n");
        return;
    }

    /* Print header: IC - 100 and DC */
    fprintf(fp, "%d %d\n", IC - 100, DC);

    p = lst_p->lst; /* Start of machine code string */
    end_p = lst_p->lst + lst_p->size; /* End of machine code string */

    while (p < end_p && *p != '\0'){
        if (*p != '/') process_raw_hex(fp, &p, &bytes_in_line, &current_addr); /* Process 2 next hex chars */
        else process_label_token(fp, &p, h_map, &bytes_in_line, &current_addr); /* Label was found */
    }

    if (bytes_in_line > 0) fprintf(fp, "\n");
    fclose(fp);
}

/* Writes hex character strings to a file */
void write_hex_bytes(FILE *fp, const char *hex_str, int len, int *bytes_in_line, int *current_addr){
    int i;
    /* Process hex pairs */
    for (i = 0; i < len; i += 2){
        /* Print 4 digit padded address at the start of a new line, else print separator space */
        if (*bytes_in_line == 0) fprintf(fp, "%04d ", *current_addr);
        else fprintf(fp, " ");
        
        /* Print single byte (2 hex characters) */
        fprintf(fp, "%c%c", hex_str[i], hex_str[i + 1]);
        (*bytes_in_line)++;
        
        /* End line after 4 bytes are written, reset byte counter, and advance base address */
        if (*bytes_in_line == 4){
            fprintf(fp, "\n");
            *bytes_in_line = 0;
            *current_addr += 4;
        }
    }
}

/* 
Encodes a calculated integer address into little-endian hex strings. 
I-type: 2 bytes (4 hex chars)
J-type: 3 bytes (6 hex chars)
*/
void encode_label_hex(char *hex_buf, int val, int is_i_type){
    /* 2-byte little endian output */
    if (is_i_type) sprintf(hex_buf, "%02X%02X", val & 0xFF, (val >> 8) & 0xFF);
    
    /* 3-byte little endian output */
    else sprintf(hex_buf, "%02X%02X%02X", val & 0xFF, (val >> 8) & 0xFF, (val >> 16) & 0xFF);
}

/* 
Parses a label marker format (/LABEL-OFFSET/ or /LABEL/), resolves its address
via hash map lookup, and writes the resulting little endian hex bytes.
*/
void process_label_token(FILE *fp, char **p_ptr, hash_map h_map, int *bytes_in_line, int *current_addr){
    char label[31]; /* String where we store our extracted label, max label length is 31 */
    int label_len = 0, /* Length of the extracted label */
    offset = 0, /* Value of extracted offset */
    is_i_type = 0, /* Flag to see if we need to convert I type instructions of J type */
    calculated_val; /* Value that we will convert into machine code */
    char hex_buf[16]; /* String that stores converted hex code */
    char *p = *p_ptr + 1; /* Skip initial '/' */
    Data_ptr data; /* Hash map data block */

    /* Extract label identifier */
    while (*p != '\0' && *p != '-' && *p != '/') label[label_len++] = *p++;
    label[label_len] = '\0';

    /* Parse optional subtraction offset for I type branch instructions */
    if (*p == '-'){
        is_i_type = 1;
        p++;
        offset = atoi(p);
        while (*p != '\0' && *p != '/') p++;
    }
    if (*p == '/') p++; /* Skip '/' */
    *p_ptr = p;

    /* Resolve memory address and construct offset */
    data = (Data_ptr)get_data(h_map, label);
    if (is_i_type){
        calculated_val = data->mem_address - offset + 4; /* Offset calculations */
        encode_label_hex(hex_buf, calculated_val, 1);
        write_hex_bytes(fp, hex_buf, 4, bytes_in_line, current_addr);
    } 
    else{
        calculated_val = data->mem_address; /* Just label address */
        encode_label_hex(hex_buf, calculated_val, 0);
        write_hex_bytes(fp, hex_buf, 6, bytes_in_line, current_addr);
    }
}

/* Processes raw 2 character hex pairs directly embedded in the stream buffer. */
void process_raw_hex(FILE *fp, char **p_ptr, int *bytes_in_line, int *current_addr){
    char hex_pair[3]; /* String that holds next pair of hex chars */
    hex_pair[0] = *(*p_ptr)++; /* First char */
    hex_pair[1] = *(*p_ptr)++; /* Second char */
    hex_pair[2] = '\0'; /* Null terminator */
    write_hex_bytes(fp, hex_pair, 2, bytes_in_line, current_addr); /* Write in file */
}
