#include "lst.h"

#define INITIAL_CAPACITY 16

list_ptr new_list(){
    list_ptr res = malloc(sizeof(list));
    if (res == NULL){
        printf("can't allocate memory\n");
        exit(1);
    }
    
    res -> capacity = INITIAL_CAPACITY; /* setting initial capacity */
    res -> lst = malloc(res -> capacity); /* creating a list with initial capacity */
    if (res -> lst == NULL){
        printf("can't allocate memory\n");
        exit(1);
    }
    res -> size = 0; /* initial size 0 */
    
    return res;
}

void free_list(list_ptr l_ptr){
    free(l_ptr -> lst);
    free(l_ptr);
}

void increase_capacity(list_ptr l_ptr){
    int new_cap = 2 * (l_ptr -> capacity); /* each time increasing capacity by 2 to minimize realloc calls */
    char *temp = realloc(l_ptr -> lst, new_cap);
    
    if(temp == NULL){
        printf("can't allocate memory\n");
        free_list(l_ptr);
        exit(1);
    }
    
    l_ptr -> lst = temp;
    l_ptr -> capacity = new_cap;
}

void list_write(list_ptr l_ptr, char *str, int size /* if writing label include size with '\0' */){
    int new_size = (l_ptr -> size) + size;
    
    while(new_size > l_ptr -> capacity) /* increasing capacity until satisfactory */
        increase_capacity(l_ptr);
    memcpy((l_ptr -> lst) + (l_ptr -> size), str, size); /* copying the string to the end of the list */
    l_ptr -> size = new_size; /* updating size */
}

list_ptr list_merge(list_ptr l1, list_ptr l2){
    list_ptr res = new_list();
    
    /* writing the memory of the lists to the new list */ 
    
    if(l1 != NULL)
        list_write(res, l1 -> lst, l1 -> size);
    if(l2 != NULL)
        list_write(res, l2 -> lst, l2 -> size);

    return res;
}

