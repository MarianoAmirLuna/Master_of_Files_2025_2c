#ifndef MEMORIA_H
#define MEMORIA_H

void inicializar_memoria()
{
    int cant_frames = cw.tam_memoria / block_size;
    lista_frames = list_create();

    for (int i = 0; i < cant_frames; i++)
    {
        marco *entrada_frame_table = malloc(sizeof(marco));

        entrada_frame_table->libre = true;
        entrada_frame_table->inicio = (char*)memory + i * block_size; //Se castea a char* para aritmetica de punteros

        sem_wait(&tabla_frame_en_uso);
        list_add(lista_frames, entrada_frame_table);
        sem_post(&tabla_frame_en_uso);
    }
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
    entrada_tabla_pags *entry = (entrada_tabla_pags *)elem;
    return strcmp(entry->file_tag, file_tag_buscado) == 0;
}

bool esta_libre(void* element){
    marco * marco_element = (marco *) element;
    return marco_element->libre;
}


/***
 * @brief retorna el primer marco que este libre o NULL
 */
int buscar_frame_libre(){
    return list_find (lista_frames, esta_libre);
}

/// @brief dado un numero de frame, devuelve la base del mismo en la memoria
/// @param n
/// @return
int buscar_base_marco(int n)
{
    return n * storage_block_size;
}

/// @brief obtiene la T.P. en base al file:tag
/// @param file_y_tag
/// @return el t_list* como filtro de la cola global
t_list* obtener_tabla_paginas(char *file_y_tag)
{
    file_tag_buscado=file_y_tag;
    t_list* ret = list_filter(tabla_pags_global->elements, coincide_tag);

    return ret;
}

/// @brief dado un par file:tag y un numero de pagina, devuelve el numero de marco que le corresponde
/// @param file_y_tag
/// @param n_pag
/// @return
int buscar_marco_en_tabla(char *file_y_tag, int n_pag)
{
    t_list *tabla = obtener_tabla_paginas(file_y_tag);
    for (int i = 0; i < list_size(tabla); i++)
    {
        entrada_tabla_pags *entrada = list_get(tabla, i);
        if (entrada->pag == n_pag)
        {
            list_destroy(tabla);
            return entrada->marco;
        }
    }
    list_destroy(tabla);
    return -1;
}


int buscar_base_pagina(char *file_y_tag, int pag)
{
    t_list *tabla = obtener_tabla_paginas(file_y_tag);

    int n_marco = buscar_marco_en_tabla(file_y_tag, pag);

    int df = buscar_base_marco(n_marco);

    return df;
}

entrada_tabla_pags *nueva_entrada()
{
    return NULL;
}


/*
==============================
*/
void buscar_victima_lru(){

}

void buscar_victima_clock_modificado(){

}

void buscar_victima()
{
    if (R_LRU == cw.algoritmo_reemplazo)
    {
        buscar_victima_lru();
    }
    else
    {
        buscar_victima_clock_modificado();
    }
}

#endif