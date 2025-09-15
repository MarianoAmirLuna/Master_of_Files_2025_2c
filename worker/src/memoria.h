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

bool comparar_marcos(void* a, void* b) {
    marco* m1 = (marco*)a;
    marco* m2 = (marco*)b;
    return m1->inicio == m2->inicio;  // criterio: mismo inicio
}

int list_index_of(t_list *self, void *data, bool (*comp)(void *, void *))
{
    int index = 0;
    t_link_element *current = self->head;
    while (current != NULL)
    {
        if (comp(current->data, data))
        {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1; // Elemento no encontrado
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

dto_file_tag_pag dto_buscado;
bool coincide_tag_y_pagina(void* element){
    entrada_tabla_pags * entrada_pag_element = (entrada_tabla_pags *) element;
    return (entrada_pag_element->pag == dto_buscado.pag) && (entrada_pag_element->file_tag == dto_buscado.file_tag);
}

/***
 * @brief Te dice si un archivo:tag esta en la T.P.
 */
bool existe_fileTag_y_pag_en_tp(char* file_tag, int pagina, t_list* tabla_de_paginas){
    dto_buscado.pag = pagina;
    dto_buscado.file_tag = file_tag;
    bool aux = list_any_satisfy (tabla_de_paginas, coincide_tag_y_pagina);

    return aux;
}

/***
 * @brief retorna el primer marco que este libre o NULL
 */
marco* buscar_frame_libre(){
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

entrada_tabla_pags *nueva_entrada(char* file_tag, int pagina, int marco)
{
    entrada_tabla_pags* ret = malloc(sizeof(entrada_tabla_pags));
    ret->file_tag=malloc(strlen(file_tag)+1);
    strcpy(ret->file_tag, file_tag);
    ret->marco=marco;
    ret->modificada=false;
    ret->pag=pagina;
    ret->uso=false;
    return ret;
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