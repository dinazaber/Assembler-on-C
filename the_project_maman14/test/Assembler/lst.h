#ifndef LST_H
#define LST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char *lst;
    int size;
    int capacity;
} list, *list_ptr; /* the list data structure */

list_ptr new_list(); /* creates a new list */
void free_list(list_ptr l_ptr); /* frees the list */
void increase_capacity(list_ptr l_ptr); /* increases the capacity by 2 */
void list_write(list_ptr l_ptr, char *str, int size /* if writing label include size with '\0' */); /* writes to the list */
list_ptr list_merge(list_ptr l1, list_ptr l2); /* merges 2 lists to a new singular list */

#endif /* LST_H */
