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

/// @brief devuelve la tabla de paginas correspondiente para el par file:tag enviado como argumento
/// @param file_y_tag
/// @return el file_y_tabla_pags* que corresponda
file_y_tabla_pags *buscar_tabla_pags(char *file_y_tag)
{
    for (int i = 0; i < list_size(tablas_pags); i++)
    {
        file_y_tabla_pags *aux = list_get(tablas_pags, i);
        if (!strcmp(aux->file_y_tag, file_y_tag))
        {
            return aux;
        }
    }
    return NULL;
}

/// @brief dado un numero de frame, devuelve la base del mismo en la memoria
/// @param n
/// @return
int buscar_base_marco(int n)
{
    return n * storage_block_size;
}

/// @brief dado un par file:tag y un numero de pagina, devuelve el numero de marco que le corresponde
/// @param file_y_tag
/// @param n_pag
/// @return
int buscar_marco_en_tabla(char *file_y_tag, int n_pag)
{
    file_y_tabla_pags *tabla = buscar_tabla_pags(file_y_tag);
    for (int i = 0; i < list_size(tabla->tabla_pags); i++)
    {
        entrada_tabla_pags *entrada = list_get(tabla->tabla_pags, i);
        if (entrada->pag == n_pag)
        {
            return entrada->marco;
        }
    }
    return -1;
}

int buscar_base_pagina(char *file_y_tag, int pag)
{
    file_y_tabla_pags *tabla = buscar_tabla_pags(file_y_tag);

    return ERROR;
}

/// @brief crea una nueva tabla de paginas y la mete a la lista
/// @param file_y_tag
/// @return el file_y_tabla_pags* creado
file_y_tabla_pags *nueva_tabla_pags(char *file_y_tag)
{
    file_y_tabla_pags *ret = malloc(sizeof(file_y_tabla_pags));
    ret->file_y_tag = malloc(sizeof(file_y_tag));
    strcpy(ret->file_y_tag, file_y_tag); // Identifico la tabla de paginas mediante el file:tag
    ret->tabla_pags = queue_create();    // Creo la tabla de paginas como una cola(para facilitar clok-m)

    sem_wait(&tabla_pag_en_uso);
    list_add(tablas_pags, ret); // Persisto el conjunto
    sem_post(&tabla_pag_en_uso);
    return ret;
}

entrada_tabla_pags *nueva_entrada()
{
    return NULL;
}

#endif