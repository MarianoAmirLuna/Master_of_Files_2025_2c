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
int obtener_frame(char *archivo, int donde_comenzar)
{
    // 1. Encontrar la tabla de paginas del file:tag
    // 2. calcular la pagina en base a la dir_base
    // 3. Si la pagina esta presente en memoria retorno el frame
    return 0;
}

int obtener_offset(char *archivo, int donde_comenzar)
{
    return 0;
}

/// @brief escribe datos a una direccion logica (como maximo el largo necesario para llenar la pagina)
/// @param file_tag 
/// @param dir_logica 
/// @param contenido 
/// @return 0 si entró todo el contenido, 1 si falta contenido por escribir
int realizar_escritura(char *file_tag, int dir_logica, char *contenido)
{
    int frame = obtener_frame(file_tag, dir_logica);
    int offset = obtener_offset(file_tag, dir_logica);
    marco *el_marco = list_get(lista_frames, frame);
    void *base = el_marco->inicio;

    int espacio_restante_en_marco = block_size - offset;

    if (strlen(contenido) > espacio_restante_en_marco) // si no me alcanza con lo que queda de marco, solo copio lo que pueda
    {
        memcpy(base + offset, contenido, espacio_restante_en_marco);
        //realizar_escritura(file_tag, dir_logica + espacio_restante_en_marco, contenido + espacio_restante_en_marco); // feo con ganas eh
    }
    else
    {
        memcpy(base + offset, contenido, strlen(contenido));
    }
}

void* actualizar_pagina(char *file_tag, int pagina){
    t_packet* paq = create_packet();

    //TODO: Storage no los necesita juntos?
    char* file = strtok(file_tag, ":");
    char* tag = strtok(NULL, "");

    add_int_to_packet(paq, GET_BLOCK_DATA);
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    add_int_to_packet(paq, pagina);

    send_and_free_packet(paq, sock_storage);
    
    sem_wait(&sem_bloque_recibido);

    int base = buscar_base_pagina(file_tag, pagina);
    
    memcpy(memory+base, data_bloque, storage_block_size);
}

void *reservar_frame(char *file_tag, int pagina)
{
    marco *frame_libre = buscar_frame_libre();
    
    frame_libre->libre = false;
    frame_buscado->inicio = frame_libre->inicio;

    int indice_frame_table = list_index_of(lista_frames, &frame_buscado, comparar_marcos);
    if(indice_frame_table < 0){
        log_error(logger, "El indice del frame no se encontro");
    }

    entrada_tabla_pags *nueva = nueva_entrada(file_tag, pagina, indice_frame_table);

    //Si el algoritmo es LRU acá se esta añadiendo una nueva entrada con la referencia más reciente
    queue_push(tabla_pags_global, nueva);

    return frame_libre->inicio;
}

bool dl_en_tp(char *file_tag, int pagina)
{
    t_list *tabla_de_paginas = obtener_tabla_paginas(file_tag);

    bool aux = existe_fileTag_y_pag_en_tp(file_tag, pagina, tabla_de_paginas);
    list_destroy(tabla_de_paginas);

    return aux;
}

int calcular_pagina(int dir_base)
{
    return dir_base / block_size; // En teoría como son enteros la division redondea al entero inferior siempre
}


void ejecutar_write(char *file_tag, int dir_base, char *contenido)
{

    int pagina = calcular_pagina(dir_base);
    int offset = obtener_offset(file_tag, dir_base);
    int restante_en_pag = block_size-offset;
    int espacio_ya_escrito=0;

    for(int indice=dir_base; espacio_ya_escrito < strlen(contenido); indice += (espacio_ya_escrito==0?restante_en_pag:block_size)) //llego el fakin operador ternario (si lees esto, perdon)
    {
        espacio_ya_escrito=indice-dir_base;
        pagina = calcular_pagina(indice);
        if (!dl_en_tp(file_tag, pagina))
        {
            if (!hay_espacio_memoria(contenido))
            {
                log_info(logger, "Iniciando algoritmo de reemplazo");
                seleccionar_victima(); //selecciona una victima y la borra. Tenes garantizado un frame libre despues de esto
            }

            // Apartir de acá hay espacio
            reservar_frame(file_tag, pagina);

            // Trae el contenido del bloque de storage
            actualizar_pagina(file_tag, pagina);

            // Apartir de acá puede ser una falopeada, no sé si se va a comportar como espero
            // ejecutar_write(file_tag, dir_base, contenido); @mariano no me mates porfa, pero creo que si dejo esto va a romper todo
            // return;
        }
        // Apartir de acá existe la DL en memoria

        realizar_escritura(file_tag, indice, contenido+espacio_ya_escrito); //espacio_ya_escrito funciona como un offset para el contenido
    }

    // realizar_escritura(file_tag, dir_base, contenido);
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