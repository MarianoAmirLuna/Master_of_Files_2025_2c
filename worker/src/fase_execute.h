#ifndef FASE_EXECUTE_H
#define FASE_EXECUTE_H

#ifndef MEMORIA_H
#include "memoria.h"
#endif


void ejecutar_create(char *file, char *tag)
{
}

void ejecutar_truncate(char *file_y_tag, int tam)
{
}


/*
    caso 1: tabla de paginas tiene la pagina
    caso 2: tabla de paginas tiene la pagina en memoria virtual
    caso 3: tabla de paginas no tiene la pagina
*/
int obtener_frame(char* archivo,int donde_comenzar)
{
    // 1. Encontrar la tabla de paginas del file:tag
    // 2. calcular la pagina en base a la dir_base
    // 3. Si la pagina esta presente en memoria retorno el frame
    return 0;
}

int obtener_offset(char* archivo,int donde_comenzar)
{
    return 0;
}

bool existe_tabla_paginas(char *ft)
{
    file_tag_buscado = ft;

    sem_wait(&tabla_pag_en_uso);
    t_list *filtrada = list_filter(tabla_pags_global, coincide_tag);
    sem_post(&tabla_pag_en_uso);

    bool r = list_is_empty(filtrada); // Redundante para asegurar mutua exclusion
    list_destroy(filtrada);

    return r;
}

void realizar_escritura(char* file_tag, int dir_logica, char* contenido)
{
    int frame = obtener_frame(file_tag, dir_logica);
    int offset = obtener_offset(file_tag, dir_logica);
    marco* el_marco = list_get(lista_frames, frame);
    void* base = el_marco->inicio;

    int espacio_restante_en_marco = block_size-offset;

    if(strlen(contenido) > espacio_restante_en_marco) //si no me alcanza con lo que queda de marco
    {
        memcpy(base+offset, contenido, espacio_restante_en_marco);
        realizar_escritura(file_tag, dir_logica+espacio_restante_en_marco, contenido+espacio_restante_en_marco); //feo con ganas eh
    }   
    else
    {
        memcpy(base+offset, contenido, strlen(contenido));
    }
}

int calcular_pagina(int dir_base){
    return dir_base/block_size; //En teoría como son enteros la division redondea al entero inferior siempre
}

bool file_tag_en_tp(char* file_tag){
    t_list* l = obtener_tabla_paginas(file_tag);
    bool cond = (NULL != l);
    free(l);
    return cond;
}

int reservar_frame(char *file_tag){
    return 0;
}

bool dl_en_tp(char* file_tag, int pagina){
    t_list* tabla_de_paginas = obtener_tabla_paginas(file_tag);

    

    return true;
}
void ejecutar_write(char *file_tag, int dir_base, char *contenido)
{
    t_list* tabla_paginas;

    int pagina = calcular_pagina(dir_base);

    if(!dl_en_tp(file_tag, pagina)){
        if(!hay_espacio_memoria(contenido)){
            log_info(logger, "Iniciando algoritmo de reemplazo");
        }
        
        reservar_frame(file_tag);
        
        //Apartir de acá puede ser una falopeada, no sé si se va a comportar como espero
        ejecutar_write(file_tag, dir_base, contenido); 
        
        return;
    }

    realizar_escritura(file_tag, dir_base, contenido);
}

void ejecutar_read(char *file, char *tag, int dir_base, int tam)
{
}

void ejecutar_tag(char *file_old, char *tag_old, char *file_new, char *tag_new)
{
}

void ejecutar_commit(char *file, char *tag)
{
}

void ejecutar_flush(char *file, char *tag)
{
}

void ejecutar_delete(char *file, char *tag)
{
}

void ejecutar_end()
{
}

#endif