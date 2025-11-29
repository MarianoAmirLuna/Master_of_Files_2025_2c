#ifndef FASE_EXECUTE_H
#define FASE_EXECUTE_H



#ifndef MEMORIA_H
#include "memoria.h"
#endif

#ifndef WORKER_BASE_H
#include "base.h"
#endif

void ejecutar_create(char *parametro)
{
    t_packet* paq = create_packet();
    add_int_to_packet(paq, CREATE_FILE);
    add_file_tag_to_packet(paq, parametro);
    send_and_free_packet(paq, sock_storage);
}

void ejecutar_truncate(char *file_y_tag, int tam)
{
    t_packet* paq = create_packet();
    add_int_to_packet(paq, TRUNCATE_FILE);
//    add_string_to_packet(paq, file_y_tag);
    char* file = NULL;
    char* tag = NULL;
    char** spl= string_split(file_y_tag, ":");
    file = malloc(strlen(spl[0])+1);
    tag = malloc(strlen(spl[1])+1);
    strcpy(file, spl[0]);
    strcpy(tag, spl[1]);
    //get_tag_file(file_y_tag, file, tag);
    
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);

    add_int_to_packet(paq, tam);
    send_and_free_packet(paq, sock_storage);
    free(file);
    free(tag);
    string_array_destroy(spl);
}

/*
    caso 1: tabla de paginas tiene la pagina
    caso 2: tabla de paginas tiene la pagina en memoria virtual
    caso 3: tabla de paginas no tiene la pagina
*/
/// @brief dado un archivo y una posicion en el mismo, busca la pagina correspondiente (si está cargada) y devuelve el n° de marco
/// @param archivo
/// @param donde_comenzar
/// @return el n° de frame o -1 si la pag no esta cargada
entrada_tabla_pags *obtener_frame(char *archivo, int donde_comenzar)
{
    // 1. Encontrar la tabla de paginas del file:tag
    log_debug(logger, "entre a obtener_frame()");
    t_list *tabla = obtener_tabla_paginas(archivo);

    // 2. calcular la pagina en base a la dir_base
    int pag = calcular_pagina(donde_comenzar);

    // 3. Si la pagina esta presente en memoria retorno el frame
    entrada_tabla_pags* ret = NULL;
    if (tabla == NULL){
        return NULL;
    }
    for (int i = 0; i < list_size(tabla); i++)
    {
        entrada_tabla_pags *aux = list_get(tabla, i);
        if (aux->pag == pag)
        {
            //Si se supone que las páginas son únicas a esta instrucción luego se debería invocar un break.
            //Sino el big O es de N
            ret = aux;
            break;
        }
    }
    list_destroy(tabla);
    return ret;
}

int obtener_offset(char *archivo, int donde_comenzar)
{
    return donde_comenzar % block_size;
}

/// @brief escribe datos a una direccion logica (como maximo el largo necesario para llenar la pagina)
/// @param file_tag
/// @param dir_logica
/// @param contenido
/// @return cantidad de bytes escritos
int realizar_escritura(char *file_tag, int dir_logica, char *contenido)
{
    msleep(cw.retardo_memoria);
    entrada_tabla_pags *entrada_con_frame = obtener_frame(file_tag, dir_logica);
    actualizarPrioridadLRU(entrada_con_frame);

    int frame = entrada_con_frame->marco;
    int offset = obtener_offset(file_tag, dir_logica);

    marco *el_marco = list_get(lista_frames, frame);
    void *base = el_marco->inicio;

    int espacio_restante_en_marco = block_size - offset;

    if (strlen(contenido) > espacio_restante_en_marco) // si no me alcanza con lo que queda de marco, solo copio lo que pueda
    {
        memcpy(base + offset, contenido, espacio_restante_en_marco);
        log_pink(logger, "escritura devolvio %d", espacio_restante_en_marco);
        return espacio_restante_en_marco;
        // realizar_escritura(file_tag, dir_logica + espacio_restante_en_marco, contenido + espacio_restante_en_marco); // feo con ganas eh
    }
    else
    {
        memcpy(base + offset, contenido, strlen(contenido));
        log_pink(logger, "escritura devolvio %d", strlen(contenido));
        return strlen(contenido);
    }
}

/// @brief guarda en "dest" una cantidad de bytes a partir de la dir. logica dada
/// @param dest
/// @param file_tag
/// @param dir_logica
/// @param tam
/// @return cantidad de bytes leidos de la pagina
int realizar_lectura(void *dest, char *file_tag, int dir_logica, int tam)
{
    msleep(cw.retardo_memoria);
    entrada_tabla_pags *entrada_con_frame = obtener_frame(file_tag, dir_logica);
    actualizarPrioridadLRU(entrada_con_frame);

    int frame = entrada_con_frame->marco;
    int offset = obtener_offset(file_tag, dir_logica);
    marco *el_marco = list_get(lista_frames, frame);
    void *base = el_marco->inicio + offset;
    int ret;

    int bytes_a_leer;
    if (block_size - offset >= tam) // si queda mas tamaño en pagina del que necesito, puedo hacer una leida sola
    {
        bytes_a_leer = tam;
        ret = 0;
    }
    else // si el tamaño es mas grande, necesito leer varias paginas
    {
        bytes_a_leer = block_size - offset;
        ret = -1;
    }

    memcpy(dest, base, bytes_a_leer);
    log_pink(logger, "lectura devolvio %d", bytes_a_leer);
    return bytes_a_leer;
}

void *actualizar_pagina(char *file_tag, int pagina)
{
    msleep(cw.retardo_memoria);
    t_packet *paq = create_packet();
    char *file = strtok(file_tag, ":");
    char *tag = strtok(NULL, "");

    add_int_to_packet(paq, GET_BLOCK_DATA);
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    add_int_to_packet(paq, pagina);

    send_and_free_packet(paq, sock_storage);

    //Podés no usar semáforo de la siguiente forma
    t_list* recv_pack = recv_operation_packet(sock_storage); 
    if(list_get_int(recv_pack, 0) != RETURN_BLOCK_DATA)
    {
        log_error(logger, "Ehh que pasó acá esto no es RETURN_BLOCK_DATA");
    }
    else{
        char* data = list_get_int(recv_pack, 1);
        memcpy(data_bloque, data, storage_block_size);
    }

    int base = buscar_base_pagina(file_tag, pagina);

    memcpy(memory + base, data_bloque, storage_block_size);

    int marco = base / storage_block_size;

    log_info(logger, "Query <%d>: - Memoria Add - File: <%s> - Tag: <%s> - Pagina: <%d> - Marco: <%d>", actual_worker->id_query, file, tag, pagina, marco);
}

void *reservar_frame(char *file_tag, int pagina)
{
    log_light_blue(logger, "ENTRE A RESERVAR_FRAME");
    marco *frame_libre = buscar_frame_libre();
    
    frame_libre->libre = false;

    int indice_frame_table = list_index_of(lista_frames, frame_libre, comparar_marcos);
    if (indice_frame_table < 0)
    {
        log_error(logger, "El indice del frame no se encontro");
    }

    entrada_tabla_pags *nueva = nueva_entrada(file_tag, pagina, indice_frame_table);

    // Si el algoritmo es LRU acá se esta añadiendo una nueva entrada con la referencia más reciente
    queue_push(tabla_pags_global, nueva);

    char *copia_ft = strdup(file_tag);
    char *file = strtok(copia_ft, ":");
    char *tag = strtok(NULL, ":");
    log_info(logger, "Query <%d>: Se asigna el Marco: <%d> a la Página: <%d> perteneciente al - File: <%s> - Tag: <%s>", actual_worker->id_query, indice_frame_table, pagina, file, tag);
    free(copia_ft);

    return frame_libre->inicio;
}

bool dl_en_tp(char *file_tag, int pagina)
{
    log_debug(logger, "entre a dl_en_tp()");
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
    log_orange(logger, "FILETAG ES NULL? %d, DIR=%d, CONTENIDO ES NULL?: %d", file_tag == NULL, dir_base, contenido == NULL);
    int pagina = calcular_pagina(dir_base);
    int offset = obtener_offset(file_tag, dir_base);
    int restante_en_pag = block_size - offset;
    int espacio_ya_escrito = 0;
    int n_frame;

    for (int indice = dir_base; espacio_ya_escrito < strlen(contenido);) //(si lees esto, perdon)
    {
        log_light_blue(logger, "escribi: %d, tengo que escribir: %d", espacio_ya_escrito, strlen(contenido));
        pagina = calcular_pagina(indice);
        if (!dl_en_tp(file_tag, pagina))
        {
            char *copia = strdup(file_tag);
            char *file = strtok(copia, ":");
            char *tag = strtok(NULL, ":");

            log_debug(logger, "PAGINA= %d", pagina);
            log_debug(logger, "ACTUAL WORKER ES NULL? %d", actual_worker == NULL);
            log_debug(logger, "FILE_TAG ES NULL? %d", file_tag == NULL);
            log_debug(logger, "FILE_TAG=%s", file_tag);
            log_debug(logger, "FILE ES NULL? %d, TAG ES NULL? %d", file == NULL, tag == NULL);
            log_debug(logger, "FILE=%s", file);
            log_debug(logger, "TAG=%s", tag);
            log_debug(logger, "IDQUERY=%d", actual_worker->id_query);
            log_info(logger, "Query <%d>: - Memoria Miss - File: <%s> - Tag: <%s> - Pagina: <%d>", actual_worker->id_query, file, tag, pagina);

            if (!hay_espacio_memoria(contenido))
            {
                entrada_tabla_pags *victima = seleccionar_victima(); // selecciona una victima
                log_info(logger, "## Query <%d>: Se reemplaza la página <%s>/<%d> por la <%s>/<%d>", actual_worker->id_query, victima->file_tag, victima->pag, file_tag, pagina);
                liberar_entrada_TPG(victima);
            }

            // Apartir de acá hay espacio
            reservar_frame(file_tag, pagina);

            if(espacio_ya_escrito == 0) //pura y exclusivamente para el log hago esto
            {
                entrada_tabla_pags *entrada_con_frame = obtener_frame(file_tag, dir_base);
                if (entrada_con_frame == NULL)
                {
                    log_error(logger, "ENTRADA CON FRAME ES NULL");
                }
                n_frame = entrada_con_frame->marco;
            }

            //string_array_destroy(spl); // libera el array de punteros
            free(copia);

            // Trae el contenido del bloque de storage
            //TODO: descomentar para que funcione storage
            //actualizar_pagina(file_tag, pagina);

        }
        // Apartir de acá existe la DL en memoria

        int bytes_escritos = realizar_escritura(file_tag, indice, contenido + espacio_ya_escrito); // espacio_ya_escrito funciona como un offset para el contenido

        indice += bytes_escritos;
        espacio_ya_escrito += bytes_escritos;
    }
    
    log_info(logger, "Query <%d>: Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", actual_worker->id_query, n_frame * storage_block_size + offset, contenido);
}

void ejecutar_read(char *file_tag, int dir_base, int tam)
{
    void *leido = malloc(tam + 1);
    int pagina = calcular_pagina(dir_base);
    int espacio_ya_leido = 0;
    int offset = obtener_offset(file_tag, dir_base);
    int n_frame;

    for (int indice = dir_base; espacio_ya_leido < tam;)
    {
        pagina = calcular_pagina(indice);
        if (!dl_en_tp(file_tag, pagina))
        {
            if (!hay_n_bytes_en_memoria(tam))
            {
                entrada_tabla_pags *victima = seleccionar_victima(); // selecciona una victima
                log_info(logger, "## Query <%d>: Se reemplaza la página <%s>/<%d> por la <%s>/<%d>", actual_worker->id_query, victima->file_tag, victima->pag, file_tag, pagina);
                liberar_entrada_TPG(victima);
            }

            // Apartir de acá hay espacio
            reservar_frame(file_tag, pagina);

            if(espacio_ya_leido == 0) //pura y exclusivamente para el log hago esto
            {
                entrada_tabla_pags *entrada_con_frame = obtener_frame(file_tag, dir_base);
                if (entrada_con_frame == NULL)
                {
                    log_error(logger, "ENTRADA CON FRAME ES NULL");
                }
                n_frame = entrada_con_frame->marco;
            }

            // Trae el contenido del bloque de storage
            //TODO: descomentar para que funcione storage
            //actualizar_pagina(file_tag, pagina);
        }
        // Apartir de acá existe la DL en memoria
        int bytes_leidos = realizar_lectura(leido + espacio_ya_leido, file_tag, indice, tam - espacio_ya_leido);

        indice += bytes_leidos;
        espacio_ya_leido += bytes_leidos;
    }
    ((char *)leido)[tam] = '\0';
    log_info(logger, "Query <%d>: Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", actual_worker->id_query, n_frame * storage_block_size + offset, leido);
}

void ejecutar_commit(char *file, char *tag)
{
    t_packet* paq = create_packet();
    add_int_to_packet(paq, COMMIT_TAG);
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    send_and_free_packet(paq, sock_storage);
}

void ejecutar_noop()
{
    return;
}

void ejecutar_flush(char *file_tag, bool reportar_error)
{
    log_debug(logger, "entre a ejecutar_flush()");
    t_list *tabla = obtener_tabla_paginas(file_tag);
    for (int i = 0; i < list_size(tabla); i++)
    {
        entrada_tabla_pags *entrada = list_get(tabla, i);
        actualizar_pagina_en_storage(entrada, reportar_error);
    }
}

void ejecutar_delete(char *file, char *tag)
{
    t_packet* paq = create_packet();
    add_int_to_packet(paq, DELETE);
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    send_and_free_packet(paq, sock_storage);
}
void ejecutar_end()
{
    t_packet *paq = create_packet();
    add_int_to_packet(paq, QUERY_END);
    add_int_to_packet(paq, actual_query->id);
    add_int_to_packet(paq, actual_query->pc);
    send_and_free_packet(paq, sock_master);
}

#endif