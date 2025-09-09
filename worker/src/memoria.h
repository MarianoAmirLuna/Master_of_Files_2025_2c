#ifndef MEMORIA_H
#define MEMORIA_H

#include "base.h"

void inicializar_memoria(){
    int cant_frames = sizeof(memory)%block_size;
    lista_frames = list_create();

    for(int i = 0; i< cant_frames; i++){
        //inicializar la lista de frames
    }
    
}

int buscar_base_frame(int n)
{
    return n * storage_block_size;
}

file_y_tabla_pags* nueva_tabla_pags(char* file_y_tag)
{
    file_y_tabla_pags* ret = malloc(sizeof(file_y_tabla_pags));
    ret->file_y_tag = malloc(sizeof(file_y_tag));
    strcpy(ret->file_y_tag, file_y_tag);
    ret->tabla_pags=list_create();
}



#endif