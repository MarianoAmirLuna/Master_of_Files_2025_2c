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


#endif