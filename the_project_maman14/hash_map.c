#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_map.h"
#include "cmd.h"

unsigned hash(char *str) {
    unsigned hash_val;
    
    for (hash_val = 0; *str != '\0'; str++) {
        hash_val = *str + 31 * hash_val; /* goes trough each char, and increments the hashing value according the char's value */
    }
    return hash_val % HASH_SIZE;
}

node_ptr find_node(hash_map h_map, char *key, unsigned *hash_val) {
    node_ptr n_ptr;
    node_ptr last_node_ptr;

    if (h_map == NULL || key == NULL) { /* safety check */
        return NULL;
    }

    last_node_ptr = h_map->last_node_found;
    
    if ((last_node_ptr != NULL) && (last_node_ptr->key != NULL) && (strcmp(key, last_node_ptr->key) == 0)) { /* Check last node found cache */
        return last_node_ptr;
    }
        
    *hash_val = hash(key);
    
    for (n_ptr = (h_map->ht)[*hash_val]; n_ptr != NULL; n_ptr = n_ptr->next) { /* Search the bucket chain */
        if (n_ptr->key != NULL && strcmp(key, n_ptr->key) == 0) {
            h_map->last_node_found = n_ptr;
            return n_ptr;
        }
    }
    
    return NULL;
}

void insert_key(hash_map h_map, char *key, void *data_ptr, InputError *error) {
    unsigned hash_val = 0;
    node_ptr n_ptr = find_node(h_map, key, &hash_val);
    
    *error = NO_ERROR;
    
    if (n_ptr == NULL) { /* Key is not in the hash map, create new node */
        n_ptr = (node_ptr)malloc(sizeof(*n_ptr));
        if (n_ptr == NULL) {
            printf("Can't allocate memory\n");
            exit(-1);
        }
        
        /* Allocate memory and copy key for ANSI C safety */
        n_ptr->key = (char *)malloc(strlen(key) + 1);
        if (n_ptr->key == NULL) {
            printf("Can't allocate memory for key\n");
            exit(-1);
        }
        strcpy(n_ptr->key, key);

        n_ptr->data_ptr = data_ptr;
        h_map->last_node_found = n_ptr;
        
        n_ptr->next = (h_map->ht)[hash_val];
        (h_map->ht)[hash_val] = n_ptr;
    
    } else { /* Overwrite existing node */
        free(n_ptr->key);
        if (n_ptr->data_ptr) {
            free(n_ptr->data_ptr);
        }

        n_ptr->key = (char *)malloc(strlen(key) + 1);
        strcpy(n_ptr->key, key);

        n_ptr->data_ptr = data_ptr;
        h_map->last_node_found = n_ptr;
    }
}

void* get_data(hash_map h_map, char *key) {
    unsigned dummy_hash = 0;
    node_ptr n_ptr = find_node(h_map, key, &dummy_hash); /* Passing a valid pointer to hash_val so find_node doesn't write to address 0 */
    
    if (n_ptr == NULL) { /* Safely check if node exists before dereferencing */
        return NULL;
    }
    
    return n_ptr->data_ptr;
}

void free_hash_map(hash_map h_map) { 
    int i;
    node_ptr n_ptr, next_ptr;
    
    if (h_map == NULL) return;

    for (i = 0; i < HASH_SIZE; i++) { /* going trough each bucket */
        n_ptr = (h_map->ht)[i];
        
        while (n_ptr != NULL) { /* freeing each node in the bucket */
            next_ptr = n_ptr->next;
            if (n_ptr->key) {
                free(n_ptr->key);
            }
            if (n_ptr->data_ptr) {
                free(n_ptr->data_ptr);
            }
            free(n_ptr);
            n_ptr = next_ptr;
        }
    }
    
    free(h_map);
}

hash_map create_empty_hash_map() {
    int i;
    hash_map hm = (hash_map)malloc(sizeof(*hm)); /* allocating memory */
    if (hm == NULL) {
        printf("Out of memory\n");
        exit(-1);
    }
    
    hm->last_node_found = NULL;
    for (i = 0; i < HASH_SIZE; i++) { /* initializing each pointer in the bucket array to NULL */
        hm->ht[i] = NULL;
    }

    return hm;
}
