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
    t_packet *paq = create_packet();
    add_int_to_packet(paq, CREATE_FILE);
    add_file_tag_to_packet(paq, parametro);
    send_and_free_packet(paq, sock_storage);
}

void ejecutar_truncate(char *file_y_tag, int tam)
{
    t_packet *paq = create_packet();
    add_int_to_packet(paq, TRUNCATE_FILE);
    add_file_tag_to_packet(paq, file_y_tag);
    add_int_to_packet(paq, tam);
    send_and_free_packet(paq, sock_storage);
}

int calcular_pagina(int dir_base)
{
    return dir_base / block_size; // En teoría como son enteros la division redondea al entero inferior siempre
}

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
    entrada_tabla_pags *ret = NULL;
    if (tabla == NULL)
    {
        log_warning(logger, "La tabla de paginas para el file:tag %s es NULL", archivo);
        return NULL;
    }

    for (int i = 0; i < list_size(tabla); i++)
    {
        entrada_tabla_pags *aux = list_get(tabla, i);
        if (aux->pag == pag)
        {
            // Si se supone que las páginas son únicas a esta instrucción luego se debería invocar un break.
            // Sino el big O es de N
            ret = aux;
            break;
        }
    }

    list_destroy(tabla);

    // Yo comprobaría si el ret es NULL, porque si es NULL la cosa se puso fea y nunca encontró la página.
    if (ret == NULL)
    {
        log_warning(logger, "No se encontro en la tabla de paginas la dirección lógica: %d para el file<tag>: %s|| Linea:  (%s:%d)", donde_comenzar, archivo, __func__, __LINE__);
    }
    return ret;
}

int obtener_offset(int donde_comenzar)
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
    if (entrada_con_frame == NULL)
    {
        log_error(logger, "No se encontro la entrada de la tabla de paginas para el file_tag=%s y dir_logica=%d", file_tag, dir_logica);
        return -1;
    }

    entrada_con_frame->modificada = true;
    entrada_con_frame->uso = true;

    log_trace(logger, "realizar_escritura: dir_logica=%d contenido=%s", dir_logica, contenido);
    log_trace(logger, "Entrada de la TPG: file_tag=%s, pag=%d, marco=%d, modificada=%d, uso=%d", entrada_con_frame->file_tag, entrada_con_frame->pag, entrada_con_frame->marco, entrada_con_frame->modificada, entrada_con_frame->uso);

    actualizarPrioridadLRU(entrada_con_frame);

    int frame = entrada_con_frame->marco;
    // Usar este método con primera argumento file_tag es un desperdicio porque no usa esa variable en la función obtener_offset
    int offset = obtener_offset(dir_logica);

    marco *el_marco = list_get(lista_frames, frame);
    void *base = el_marco->inicio;

    int espacio_restante_en_marco = block_size - offset;

    if (strlen(contenido) > espacio_restante_en_marco) // si no me alcanza con lo que queda de marco, solo copio lo que pueda
    {
        memcpy(base + offset, contenido, espacio_restante_en_marco);
        log_trace(logger, "Se realizó una escritura %d", espacio_restante_en_marco);
        return espacio_restante_en_marco;
        // realizar_escritura(file_tag, dir_logica + espacio_restante_en_marco, contenido + espacio_restante_en_marco); // feo con ganas eh
    }
    else
    {
        memcpy(base + offset, contenido, strlen(contenido));
        log_trace(logger, "Se realizó una escritura %d", (int)strlen(contenido));
        return (int)strlen(contenido);
    }

    loguear_tabla_paginas_global();
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

    // Obtener la entrada de la tabla de páginas para la dirección lógica
    entrada_tabla_pags *entrada_con_frame = obtener_frame(file_tag, dir_logica);
    if (entrada_con_frame == NULL)
    {
        log_warning(logger, "No se encontró la entrada para la dirección lógica: %d en el file<tag>: %s", dir_logica, file_tag);
        return -1; // Retornar error si no se encuentra la entrada
    }

    // Actualizar el bit de uso y la prioridad LRU
    entrada_con_frame->uso = true;
    actualizarPrioridadLRU(entrada_con_frame);

    // Calcular el marco y el offset dentro del marco
    int frame = entrada_con_frame->marco;
    int offset = obtener_offset(dir_logica);
    marco *el_marco = list_get(lista_frames, frame);
    void *base = el_marco->inicio + offset;
    
    // Copiar los datos desde la memoria al destino
    memcpy(dest, base, tam);

    log_trace(logger, "Lectura realizada: %d bytes desde el marco %d, offset %d", tam, frame, offset);
    return tam;
}

void mandarLecturaAMaster(char* lectura, char* file_tag) {
    t_packet *paq = create_packet();
    add_int_to_packet(paq, GET_DATA);
    add_int_to_packet(paq, actual_worker->id_query);
    add_string_to_packet(paq, lectura);
    char **copia_ft = string_split(file_tag, ":");
    add_string_to_packet(paq, copia_ft[0]); // file
    add_string_to_packet(paq, copia_ft[1]); // tag
    send_and_free_packet(paq, sock_master);
    string_array_destroy(copia_ft);
}

void *actualizar_pagina(char *file_tag, int pagina)
{
    log_trace(logger, "ENTRE A ACTUALIZAR PAGINA el CW:RetardoMemoria es: %d", cw.retardo_memoria);
    msleep(cw.retardo_memoria);

    char **spl = string_split(file_tag, ":");
    char *file = spl[0];
    char *tag = spl[1];

    t_packet *paq = create_packet();
    add_int_to_packet(paq, READ_BLOCK);
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    add_int_to_packet(paq, pagina);
    send_and_free_packet(paq, sock_storage);

    log_trace(logger, "Esperando respuesta de Storage para GET_DATA...");
    sem_wait(&sem_get_data);
    int base = buscar_base_pagina(file_tag, pagina, -1);
    log_trace(logger, "StorageBlockSize: %d base=%d", storage_block_size, base);

    memcpy(memory + base, data_bloque, storage_block_size);

    int marco = base / storage_block_size;

    log_info(logger, "Query <%d>: - Memoria Add - File: <%s> - Tag: <%s> - Pagina: <%d> - Marco: <%d>", actual_worker->id_query, file, tag, pagina, marco);

    string_array_destroy(spl);
}

entrada_tabla_pags *reservar_frame(char *file_tag, int pagina)
{
    log_trace(logger, "ENTRE A RESERVAR_FRAME");
    marco *frame_libre = buscar_frame_libre();

    frame_libre->libre = false;

    int indice_frame_table = list_index_of(lista_frames, frame_libre, comparar_marcos);
    if (indice_frame_table < 0)
    {
        log_error(logger, "El indice del frame no se encontro");
    }

    entrada_tabla_pags *nueva = nueva_entrada(file_tag, pagina, indice_frame_table);

    char **copia_ft = string_split(file_tag, ":");
    char *file = copia_ft[0];
    char *tag = copia_ft[1];
    log_info(logger, "Query <%d>: Se asigna el Marco: <%d> a la Página: <%d> perteneciente al - File: <%s> - Tag: <%s>", actual_worker->id_query, indice_frame_table, pagina, file, tag);
    string_array_destroy(copia_ft);

    return nueva;
}

int manejar_miss_memoria(char *file_tag, int pagina)
{
    int marco_a_loguear;
    if (!hay_n_bytes_en_memoria(block_size))
    {
        entrada_tabla_pags *victima = seleccionar_victima();
        log_info(logger, "## Query <%d>: Se reemplaza la página <%s>/<%d> por la <%s>/<%d>",
                 actual_worker->id_query, victima->file_tag, victima->pag, file_tag, pagina);
        liberar_entrada_TPG(victima);
    }

    entrada_tabla_pags *nueva_entrada_TPG = reservar_frame(file_tag, pagina);
    marco_a_loguear = nueva_entrada_TPG->marco;
    queue_push(tabla_pags_global, nueva_entrada_TPG);
    actualizar_pagina(file_tag, pagina);
    return marco_a_loguear;
}

bool dl_en_tp(char *file_tag, int pagina)
{
    log_debug(logger, "entre a dl_en_tp()");
    t_list *tabla_de_paginas = obtener_tabla_paginas(file_tag);

    bool aux = existe_fileTag_y_pag_en_tp(file_tag, pagina, tabla_de_paginas);
    list_destroy(tabla_de_paginas);
    return aux;
}

void ejecutar_write(char *file_tag, int dir_base, char *contenido)
{
    log_trace(logger, "FILETAG ES NULL? %d, DIR=%d, CONTENIDO ES NULL?: %d", file_tag == NULL, dir_base, contenido == NULL);
    int pagina = calcular_pagina(dir_base);
    int offset = obtener_offset(dir_base);
    //int restante_en_pag = block_size - offset;
    int espacio_ya_escrito = 0;
    int n_frame;

    // for (int indice = dir_base; espacio_ya_escrito < strlen(contenido);)
    //{
    log_trace(logger, "escribi: %d, tengo que escribir: %d", espacio_ya_escrito, strlen(contenido));
    // pagina = calcular_pagina(indice);
    if (!dl_en_tp(file_tag, pagina))
    {
        char **spl = string_split(file_tag, ":");

        char *file = spl[0];
        char *tag = spl[1];

        log_debug(logger, "ACTUAL WORKER ES NULL? %d", actual_worker == NULL);
        log_debug(logger, "FILE_TAG ES NULL? %d", file_tag == NULL);
        log_debug(logger, "FILE ES NULL? %d, TAG ES NULL? %d", file == NULL, tag == NULL);
        log_info(logger, "Query <%d>: - Memoria Miss - File: <%s> - Tag: <%s> - Pagina: <%d>", actual_worker->id_query, file, tag, pagina);

        n_frame = manejar_miss_memoria(file_tag, pagina);

        string_array_destroy(spl);
    }
    else
    {
        entrada_tabla_pags *aux = obtener_frame(file_tag, dir_base);
        n_frame = aux->marco;
    }
    // Apartir de acá existe la DL en memoria
    //int bytes_escritos = realizar_escritura(file_tag, dir_base, contenido + espacio_ya_escrito);
    realizar_escritura(file_tag, dir_base, contenido + espacio_ya_escrito); // espacio_ya_escrito funciona como un offset para el contenido

    // indice += bytes_escritos;
    // espacio_ya_escrito += bytes_escritos;
    //}

    log_trace(logger, "frame usado para la escritura: %d, tamaño de bloque: %d, offset: %d", n_frame, storage_block_size, offset);

    log_info(logger, "Query <%d>: Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", actual_worker->id_query, n_frame * storage_block_size + offset, contenido);
}

char* ejecutar_read(char *file_tag, int dir_base, int tam)
{
    void *leido = malloc(tam + 1);
    int pagina = calcular_pagina(dir_base);
    //int espacio_ya_leido = 0;
    int offset = obtener_offset(dir_base);
    int n_frame;

    // for (int indice = dir_base; espacio_ya_leido < tam;)
    //{
    //pagina = calcular_pagina(indice);
    if (!dl_en_tp(file_tag, pagina))
    {
        char **spl = string_split(file_tag, ":");
        char *file = spl[0];
        char *tag = spl[1];
        log_info(logger, "Query <%d>: - Memoria Miss - File: <%s> - Tag: <%s> - Pagina: <%d>", actual_worker->id_query, file, tag, pagina);

        n_frame = manejar_miss_memoria(file_tag, pagina);

        string_array_destroy(spl);
    }
    else
    {
        entrada_tabla_pags *aux = obtener_frame(file_tag, dir_base);
        n_frame = aux->marco;
    }
    // Apartir de acá existe la DL en memoria
    realizar_lectura(leido, file_tag, dir_base, tam);
    //int bytes_leidos = realizar_lectura(leido, file_tag, dir_base, tam);

    // indice += bytes_leidos;
    // espacio_ya_leido += bytes_leidos;
    // }

    ((char *)leido)[tam] = '\0';
    log_trace(logger, "frame usado para la LEER: %d, tamaño de bloque: %d, offset: %d", n_frame, storage_block_size, offset);
    log_info(logger, "Query <%d>: Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", actual_worker->id_query, n_frame * storage_block_size + offset, (char *)leido);

    return leido;
}

void ejecutar_commit(char *file, char *tag)
{
    t_packet *paq = create_packet();
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

    loguear_tabla_de_paginas(tabla);

    for (int i = 0; i < list_size(tabla); i++)
    {
        entrada_tabla_pags *entrada = list_get(tabla, i);
        if (entrada->modificada)
        {
            actualizar_pagina_en_storage(entrada, reportar_error);
            sem_wait(&sem_respuesta_storage);
            if (hubo_error)
            {
                break;
            }
            int indice = list_index_of(tabla_pags_global->elements, entrada, entrada_compare_completa);
            log_trace(logger, "indice a actualizar en tabla global: %d", indice);
            entrada->modificada = false;
            if (indice >= 0)
            {
                list_replace(tabla_pags_global->elements, indice, entrada);
            }
            else
            {
                log_error(logger, "No se encontró la entrada en la tabla global para actualizar.");
            }
        }
    }
    if (hubo_error)
    {
        log_orange(logger, "se abortó el flush");
    }
    loguear_tabla_paginas_global();
    list_destroy(tabla);
    sem_post(&fin_de_flush);
}

void ejecutar_delete(char *file, char *tag)
{
    t_packet *paq = create_packet();
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