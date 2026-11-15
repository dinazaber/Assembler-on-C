#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "errors.h"

#define HASH_SIZE 101

typedef enum{

	CODE, CODE_ENTRY, DATA_ENTRY, DATA, EXTERNAL, UNKNOWN, ERROR_VAL

} Attributes;

typedef struct{

	int mem_address;
	Attributes att;

} Data, *Data_ptr;

typedef struct node{

	struct node *next;
	char *key;
	void *data_ptr;
	

}node, *node_ptr;

typedef node_ptr hash_table[HASH_SIZE];

typedef struct {

	hash_table ht;
	node_ptr last_node_found;

} *hash_map;

unsigned hash(char *str); /* returns the hash value of a string */
node_ptr find_node(hash_map h_map, char *key, unsigned *hash_val); /* returns a node pointer coresponding to the key, if exists */
void insert_key(hash_map h_map, char *key, void *data_ptr, InputError *error); /* inserts the key into the hash map, if it already exists, acts according to the attributes */

void* get_data(hash_map h_map, char *key); /* returns the data paired to the key */

void free_hash_map(hash_map h_map); /* frees the hash map */
hash_map create_empty_hash_map();

#endif /* HASH_MAP_H */
