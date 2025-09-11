#ifndef FASE_EXECUTE_H
#define FASE_EXECUTE_H

#ifndef MEMORIA_H
#include "memoria.h"
#endif

static char *tag_buscado;

void ejecutar_create(char *file, char *tag)
{
}

void ejecutar_truncate(char *file_y_tag, int tam)
{
}

int realizar_escritura(frame, contenido){
    return 0;
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

}

bool esta_libre(void *elem)
{
    marco *entry = (marco *)elem;
    return entry->libre;
}
bool hay_espacio_memoria(char *contenido)
{
    int length = string_length(contenido);
    int cant_pags = (length + block_size - 1) / block_size;

    sem_wait(&tabla_pag_en_uso);
    t_list *frames_libres = list_filter(lista_frames, esta_libre);
    sem_post(&tabla_pag_en_uso);

    bool aux = list_size(frames_libres) >= cant_pags;
    list_destroy(frames_libres);

    return aux;
}

bool coincide_tag(void *elem)
{
    file_y_tabla_pags *entry = (file_y_tabla_pags *)elem;
    return strcmp(entry->file_y_tag, tag_buscado) == 0;
}

bool existe_tabla_paginas(char *ft)
{
    tag_buscado = ft;

    sem_wait(&tabla_pag_en_uso);
    t_list *filtrada = list_filter(tablas_pags, coincide_tag);
    sem_post(&tabla_pag_en_uso);

    bool r = list_is_empty(filtrada); // Redundante para asegurar mutua exclusion
    list_destroy(filtrada);

    return r;
}

void ejecutar_write(char *file, char *tag, int dir_base, char *contenido)
{
    char *file_tag = strcat(file, tag);
    int direccion_logica = dir_base;

    //TODO: Se debe realizar en el momento de la escritura, no sabes si estas reemplazando la página que vas a escribir
    if (!hay_espacio_memoria(contenido))
    {
        log_info(logger, "Iniciando algoritmo de reemplazo");
    }

    if (!existe_tabla_paginas(file_tag)) // si no existe la tabla de paginas y hay espacio en memoria
    {
        nueva_tabla_pags(tag_buscado); // crea la tabla y la añade a la lista de tablas de paginas usando mutex
    }

    while(direccion_logica>=0){
        int frame = obtener_frame(file_tag, direccion_logica);
        direccion_logica = realizar_escritura(frame, contenido);
    }

    /*
    caso 1: contenido = pagina
    caso 2: contenido < pagina
    caso 3: contenido > pagina
    */
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