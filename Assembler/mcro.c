#include "mcro.h"


void open_mcro(char *file_name, InputError *error, int *line_count){

	FILE *og_file_ptr = fopen(file_name, "r"), *p_mcro_ptr; /* opening original file */
	char post_f_name[MAX_FILE_NAME + 16], line[MAX_LINE_LEN_PRE];
	char *ans = malloc(MAX_FILE_NAME + 16); /* post macro file name */
	int base_len;
	
	hash_map mcro_table = create_empty_hash_map();
	base_len = (int)strlen(file_name) - 3;
	
	sprintf(post_f_name, "%.*s.am", base_len, file_name); /* formatting the file name  */
	p_mcro_ptr = fopen(post_f_name, "w");
	
	if(og_file_ptr == NULL){
		
		printf("couldn't open file: %s\n", file_name);
		exit(-1);
		
	}
	
	if(p_mcro_ptr == NULL){
	
		printf("couldn't open file: %s\n", post_f_name);
		exit(-1);
		
	}
	
	if(ans == NULL){
	
		printf("can't allocate memory");
		exit(-1);
	
	}
	
	
	while(fgets(line, sizeof(line), og_file_ptr) != NULL){ /* goes trough each line */
	
		int line_ptr = 0;
		char mcro_name[MAX_MCRO_NAME];
		Command cmd;
		mcro_ptr m_ptr;
		(*line_count)++;
		
		cmd = identify_command(&line_ptr, line);
		
		if(cmd == MCRO){ /* we found a macro declaration */
		
			int name_ptr = 0;
			Command check_name;
			get_macro_name(&line_ptr, line, mcro_name);
			check_name = identify_command(&name_ptr, mcro_name);
			
			/* printing errors */
			
			if(ADD <= check_name && check_name <= MCROEND){ 
			
				*error = INVALID_MCRO_NAME;
				print_error(*error, *line_count);
				break;
			
			}
			
			*error = check_end(&line_ptr, line);
			
			if(*error != NO_ERROR) {
				print_error(*error, *line_count);
				break;
			}
			
			if(get_data(mcro_table, mcro_name) != NULL){
				*error = DOUBLE_MCRO_DEC;
				print_error(*error, *line_count);
				break;
			}
				
			if(*error != NO_ERROR) {
				print_error(*error, *line_count);
				break;
			}
			
			read_mcro(og_file_ptr, mcro_name, mcro_table, line_count, error); /* reading a macro into the macro table */
			continue;
		
		}
		
		line_ptr = 0;
		get_macro_name(&line_ptr, line, mcro_name);
		
		
		if(mcro_name[0] != '\0'){ /* doesn't copy empty lines */
		
			m_ptr = get_data(mcro_table, mcro_name);
			if(m_ptr != NULL){ /* we found a macro to open */
		
			        write_mcro(p_mcro_ptr, m_ptr);
			        continue;
		
		        }
		
		}
		
		
		
		fprintf(p_mcro_ptr, "%s", line); /* printing into the file a "non macro" line */
		
	}
	
	free_hash_map(mcro_table);
	fclose(p_mcro_ptr);
	sprintf(ans, "%s", post_f_name);
	
	free(ans);
	


}

void read_mcro(FILE* og_file_ptr, char *mcro_name, hash_map mcro_table, int *line_count, InputError *error){

	 mcro_ptr res = malloc(sizeof(mcro));
	 int mcro_line_count = 0;
	 
	 char line[MAX_LINE_LEN_PRE];
	 
	 while(fgets(line, sizeof(line), og_file_ptr) != NULL){ /* reading each line of the macro */
	
		int line_ptr = 0;
		Command cmd;
		(*line_count)++;
		
		cmd = identify_command(&line_ptr, line);
		
		if(cmd == MCROEND){ /* found the end of the macro */
		        
		        /*printing errors */
		        
			*error = check_end(&line_ptr, line);
			
			if(*error != NO_ERROR){
				print_error(*error, *line_count);
				return;
			}
			
			break;
		
		}
		
		strcpy((res -> lines)[mcro_line_count], line); /* copying the line to the macro */
		mcro_line_count++;
		
			
	
	}
	
	 res -> mcro_line_count = mcro_line_count;
	 insert_key(mcro_table, mcro_name, res, error);
	 
	 

}

void write_mcro(FILE* p_mcro_ptr, mcro_ptr m_ptr){

	int i;
	for(i = 0; i < (m_ptr -> mcro_line_count); i++) /* writes the macro line by line */
		fprintf(p_mcro_ptr, "%s", (m_ptr -> lines)[i]);
    
}

/* Extracting the next word from 'line' starting at *index */
void get_macro_name(int *index, const char *line, char *macro_name_out){
    int i = 0;

    /* Skipping leading whitespace */
    while (line[*index] != '\0' && isspace((unsigned char)line[*index])) {
        (*index)++;
    }

    /*Copy the macro name until whitespace, newline, or end of string */
    while (line[*index] != '\0' && !isspace(line[*index]) && i < MAX_MCRO_NAME - 1) {
        macro_name_out[i++] = line[*index];
        (*index)++;
    }

    macro_name_out[i] = '\0';
}

























