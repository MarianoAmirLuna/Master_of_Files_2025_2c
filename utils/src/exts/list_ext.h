#ifndef EXTS_LIST_EXT
#define EXTS_LIST_EXT

#include <stdio.h>
#include "commons/collections/list.h"

int list_get_int(t_list* l, int index){
    if(list_size(l) < index)
        log_error(logger, "INDICE INVALIDO EN LA LISTA %s:%d", __func__, __LINE__);
    int res = 0;
    memcpy(&res, list_get(l, index), sizeof(int));
    return res;
}

char* list_get_str(t_list* l, int index){
    return (char*)list_get(l, index);
    //return buffer_to_string(list_get(l, index));
}
t_list* list_filter_by(t_list* l, int(*condition)(void*, void*), void* by)
{
    t_list* res = list_create();
    for(int i=0;i<list_size(l);i++){
        void* elem = list_get(l, i);
        if(condition(elem, by))
            list_add(res, elem);
    }
    return res;
}

t_list* list_filter_by_and_by(t_list* l, int(*condition)(void*, void*, void*), void* by, void* andby)
{
    t_list* res = list_create();
    for(int i=0;i<list_size(l);i++){
        void* elem = list_get(l, i);
        if(condition(elem, by, andby))
            list_add(res, elem);
    }
    return res;
}


/// @brief Busca el primer elemento que conicida con su condición
/// @param l 
/// @param condition 
/// @param by 
/// @return el elemento, si no existe retorna NULL
void* list_find_by(t_list* l, int(*condition)(void*, void*), void* by){
    
    for(int i=0;i<list_size(l);i++){
        void* elem = list_get(l, i);
        if(condition(elem, by))
            return elem;
    }
    return NULL;
}

int list_exists(t_list* l, int(*condition)(void*, void*), void* by)
{
    return list_find_by(l,condition, by) != NULL;
}

/// @brief Suponigendo que todos los elementos son de tipo entero
/// @param l 
/// @return 
int list_sum(t_list* l){
    int res = 0;
    for(int i=0;i<list_size(l);i++){
        int v=0;
        memcpy(&v, list_get(l, i), sizeof(int));
        res+=v;
    }
    return res;
}

double list_avg(t_list* l){
    int sum = list_sum(l);
    int sz = list_size(l);
    if(sz == 0) //NO SE PUEDE DIVIDIR POR CERO
        return 0;
    return (double)sum / (double)sz;
}

void list_fill(t_list* l, void* value)
{
    for(int i=0;i<list_size(l);i++){
        list_replace(l, i, value);
    }
}
void list_fill_from_to(t_list* l, void* value, int from, int to){
    if(list_size(l) >= to || from < 0)
    {
        printf("Invalid %s from %d to %d the list size is %d", __func__, from,to,list_size(l));
        exit(1);
    }
    for(int i=from;i<to;i++)
        list_replace(l,i,value);
}
void list_fill_from_length(t_list* l, void* value, int from, int length){
    list_fill_from_to(l,value, from, from+length);
}
void list_add_range(t_list* l, void* v, int sz){
    for(int i=0;i<sz;i++){
        list_add(l, v);
    }
}

void list_add_range_int(t_list* l, int v, int sz){
    for(int i=0;i<sz;i++){
        void* val = malloc(sizeof(int));
        memcpy(val, &v, sizeof(int));
        list_add(l, val);
    }
}

t_list* list_add_range_fill(t_list* l, void*(*fptr)()){
    for(int i=0;i<list_size(l);i++){
        list_add(l, fptr());
    }
    return l;
}

t_list* list_add_range_fill_length(t_list* l, void*(*fptr)(), int length){
    for(int i=0;i<length;i++){
        list_add(l, fptr());
    }
    return l;
}
#endif