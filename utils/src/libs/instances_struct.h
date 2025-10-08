#ifndef INSTANCES_STRUCT_H
#define INSTANCES_STRUCT_H

#include "stdlib.h"
#ifndef UTILS_STRUCTS_H
#include "../utils/structs.h"
#endif 


/// @brief General
/// @return 
query* create_query(){
    query* result = malloc(sizeof(query));
    result->id = -1;
    result->fd = -1;
    result->id = -1;
    result->sp = STATE_READY;
    result->pc = 0;
    result->archive_query = NULL;
    result->priority = 0;
    result->start_tick = 0;
    result->temp = NULL;
    result->end_tick = 0;
    result->instructions =NULL;
    return result;
}

/// @brief Este método es útil para que sea invocado desde módulo WORKER
/// @param id 
/// @param archivo_query 
/// @param pc 
/// @return 
query* create_basic_query(int id, char* archivo_query, int pc){
    query* result = create_query();
    result->id = id;
    result->archive_query = malloc(strlen(archivo_query)+1);
    strcpy(result->archive_query, archivo_query);
    result->pc = pc;
    result->instructions = list_create();
    return result;
}

void free_query(query* q){
    if(q != NULL){
        if(q->archive_query != NULL) 
            free(q->archive_query);
        if(q->temp != NULL) 
            free(q->temp);
        if(q->instructions != NULL) 
            list_destroy_and_destroy_elements(q->instructions, free);
        free(q);
    }
}


#endif
