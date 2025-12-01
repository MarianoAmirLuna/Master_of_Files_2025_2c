#ifndef MEMORIA_H
#define MEMORIA_H

void inicializar_memoria()
{
    memory = malloc(cw.tam_memoria);
    if(block_size == 0){
        log_error(logger, "No podés dividir por 0 gil pero lo voy a ignorar seteando como 16");
        block_size = 16;
    }
    int cant_frames = cw.tam_memoria / block_size;
    log_info(logger, "la cantidad de frames es: %d", cant_frames);
    log_info(logger, "cant_frames * tam_block: %d | tamaño de memoria: %d", cant_frames*block_size, cw.tam_memoria);    

    lista_frames = list_create();

    for (int i = 0; i < cant_frames; i++)
    {
        marco *entrada_frame_table = malloc(sizeof(marco));

        entrada_frame_table->libre = true;
        //entrada_frame_table->inicio = (char*)memory + i * block_size; //Se castea a char* para aritmetica de punteros
        entrada_frame_table->inicio = (char*)(memory + i * block_size); //Se castea a char* para aritmetica de punteros
        
        list_add(lista_frames, entrada_frame_table);
    }
    log_trace(logger, "tamaño de la lista: %d", list_size(lista_frames));
}

bool esta_libre(void* element){
    marco * marco_element = (marco *) element;
    return marco_element->libre;
}

bool hay_espacio_memoria(char *contenido)
{
    int length = string_length(contenido);
    int cant_pags = (length + block_size - 1) / block_size;

    sem_wait(&tabla_pag_en_uso);
    t_list *frames_libres = list_filter(lista_frames, esta_libre);
    sem_post(&tabla_pag_en_uso);
    log_trace(logger, "cantidad de frames libres: %d, cantidad de páginas: %d", list_size(frames_libres), cant_pags);
    bool aux = list_size(frames_libres) >= cant_pags;
    list_destroy(frames_libres);

    log_trace(logger, "hay espacio en memoria?????? capaz %d", aux);

    return aux;
}

bool hay_n_bytes_en_memoria(int n)
{
    int cant_pags = (n + block_size - 1) / block_size;

    sem_wait(&tabla_pag_en_uso);
    t_list *frames_libres = list_filter(lista_frames, esta_libre);
    sem_post(&tabla_pag_en_uso);

    bool aux = list_size(frames_libres) >= cant_pags;
    list_destroy(frames_libres);

    return aux;
}

bool entrada_compare_completa(void* a, void* b) {
    entrada_tabla_pags* e1 = (entrada_tabla_pags*)a;
    entrada_tabla_pags* e2 = (entrada_tabla_pags*)b;

    return string_equals_ignore_case(e1->file_tag, e2->file_tag) &&
           e1->marco == e2->marco &&
           e1->pag == e2->pag &&
           e1->modificada == e2->modificada &&
           e1->uso == e2->uso;
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
    log_trace(logger, "filetag: %s, pag: %d", entry->file_tag, entry->pag);
    return strcmp(entry->file_tag, file_tag_buscado) == 0;
}



dto_file_tag_pag dto_buscado;
bool coincide_tag_y_pagina(void* element){
    entrada_tabla_pags* entrada_pag_element = (entrada_tabla_pags*) element;
    return (entrada_pag_element->pag == dto_buscado.pag) && (entrada_pag_element->file_tag == dto_buscado.file_tag);
}

/***
 * @brief Te dice si un archivo:tag esta en la T.P.
 */
bool existe_fileTag_y_pag_en_tp(char* file_tag, int pagina, t_list* tabla_de_paginas){
    //dto_buscado.pag = pagina;
    //dto_buscado.file_tag = file_tag;
    //bool aux = list_any_satisfy (tabla_de_paginas, coincide_tag_y_pagina);

    for (int i=0;i<list_size(tabla_de_paginas); i++)
    {
        entrada_tabla_pags* aux = list_get(tabla_de_paginas, i);
        if(aux == NULL){
            log_error(logger, "Eee que paso aca esto es nulo en (%s:%d)", __func__,__LINE__);
        }
        if(aux->pag==pagina && string_equals_ignore_case(aux->file_tag, file_tag)){
            return true;
        }
    }
    return false;
}

/***
 * @brief retorna el primer marco que este libre o NULL
 */
marco* buscar_frame_libre(){
    return list_find(lista_frames, esta_libre);
}

/// @brief dado un numero de frame, devuelve el indice de la base del mismo en la memoria
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
    log_debug(logger, "FILEYTAG: %s (%s:%d)", file_y_tag, __func__,__LINE__);
    file_tag_buscado=strdup(file_y_tag);
    t_list* ret = list_filter(tabla_pags_global->elements, coincide_tag);
    free(file_tag_buscado);
    log_debug(logger, "LISTA ES NULL?: %d (%s:%d)", ret == NULL, __func__,__LINE__);
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
            // creo que acá iría if(R_LRU == cw.algoritmo_reemplazo){actualizarPrioridadLRU(entrada)}
            return entrada->marco;
        }
    }
    list_destroy(tabla);
    return -1;
}

/// @brief Retorna el desplazamiento en la memoria real necesario para llegar al inicio de una pagina
int buscar_base_pagina(char *file_y_tag, int pag)
{
    int n_marco = buscar_marco_en_tabla(file_y_tag, pag);
    int df = buscar_base_marco(n_marco);
    return df;
}

entrada_tabla_pags* nueva_entrada(char* file_tag, int pagina, int marco)
{
    entrada_tabla_pags* ret = malloc(sizeof(entrada_tabla_pags));
    ret->file_tag=malloc(strlen(file_tag)+1);
    strcpy(ret->file_tag, file_tag);
    ret->marco=marco;
    ret->modificada=false;
    ret->pag=pagina;
    ret->uso=true;
    return ret;
}

void loguear_tabla_paginas_global() {
    log_warning(logger, "");
    log_warning(logger, "=== Tabla de Páginas Global ===");

    if (queue_size(tabla_pags_global) == 0) {
        log_info(logger, "La tabla de páginas global está vacía.");
        return;
    }

    for (int i = 0; i < queue_size(tabla_pags_global); i++) {
        entrada_tabla_pags* entrada = list_get(tabla_pags_global->elements, i);

        if (entrada == NULL) {
            log_error(logger, "Entrada nula en la tabla de páginas global en el índice %d", i);
            continue;
        }

        log_info(logger, "Entrada %d: File:Tag = <%s>, Página = <%d>, Marco = <%d>, Uso = <%d>, Modificada = <%d>",
                 i,
                 entrada->file_tag,
                 entrada->pag,
                 entrada->marco,
                 entrada->uso,
                 entrada->modificada);
    }

    log_warning(logger, "=== Fin de la Tabla de Páginas Global ===");
    log_warning(logger, "");
}

/*
    ALGORITMOS DE REEMPLAZO
*/
bool comparar_entrada_tabla_pags(void *a, void *b)
{
    entrada_tabla_pags *lineaA = (entrada_tabla_pags *)a;
    entrada_tabla_pags *lineaB = (entrada_tabla_pags *)b;
    return (!strcmp(lineaA->file_tag, lineaB->file_tag)) && (lineaA->pag == lineaB->pag);
}

void actualizarPrioridadLRU(entrada_tabla_pags *entrada)
{
    int index = list_index_of(tabla_pags_global->elements, entrada, comparar_entrada_tabla_pags); // Obtengo el índice del que quiero actualizar

    if (index != -1){                                                                         // Verifica que el elemento esté en la lista
        entrada_tabla_pags* entradaRemovida = list_remove(tabla_pags_global->elements, index); // Lo borro
        queue_push(tabla_pags_global, entradaRemovida);                                  // Lo vuelvo a agregar al final
        log_light_green(logger, "Query <%d>: Se actualiza la prioridad LRU de la página <%d> del - File:Tag : <%s>",
            actual_worker->id_query, entrada->pag, entrada->file_tag);
    }
}

void mostrar_contenido_memoria() {
    log_trace(logger, "Mostrando contenido de la memoria:");

    for (int i = 0; i < cw.tam_memoria; i++) {
        unsigned char byte = *((unsigned char *)memory + i);
        log_info(logger, "Byte %d: 0x%02X (%c)", i, byte, (byte >= 32 && byte <= 126) ? byte : '.');
    }

    log_trace(logger, "Fin del contenido de la memoria.");
}

void actualizar_pagina_en_storage(entrada_tabla_pags *elemento, bool reportar_error)
{
    char* contenido = malloc(storage_block_size);
    int base = buscar_base_pagina(elemento->file_tag, elemento->pag);
    log_trace(logger,"base: %d, storage_block_size=%d" ,base, storage_block_size);
    if (base < 0 || base + storage_block_size > cw.tam_memoria) {
        log_error(logger, "Acceso fuera de límites en memoria: base=%d, tamaño=%d", base, storage_block_size);
        free(contenido);
        return;
    }
//_RESIDENT_EVIL:_ // deberia llegar esto (16 caracteres)

//_RESIDENT_EVIL:_io 1 // me llega esto (20 caracteres ??? )
    memcpy(contenido, memory + base, storage_block_size);

    char* contenido2 = string_substring(contenido, 0, storage_block_size);

    log_pink(logger, "contenido: %s, tamaño = %d", contenido2, strlen(contenido2));

    log_trace(logger, "contenido enviado a storage: %s, el bloque lógico es: %d y el archivo es: %s", 
        contenido2, elemento->pag, elemento->file_tag
    );

    t_packet* paq = create_packet();
    add_int_to_packet(paq, reportar_error ? WRITE_BLOCK : WRITE_BLOCK_NOT_ERROR);

    log_trace(logger, "FileTag del elemento: %s", elemento->file_tag);
    char* file = NULL;
    char* tag = NULL;
    char** spl= string_split(elemento->file_tag, ":");
    file = malloc(strlen(spl[0]));
    tag = malloc(strlen(spl[1]));
    strcpy(file, spl[0]);
    strcpy(tag, spl[1]);
    
    add_string_to_packet(paq, file);
    add_string_to_packet(paq, tag);
    add_int_to_packet(paq, elemento->pag); 
    add_string_to_packet(paq, contenido2);
    
    send_and_free_packet(paq, sock_storage);
    log_trace(logger, "FILE: %s, TAG:%s a enviar al storage", file, tag);
    free(file);
    free(tag);
    string_array_destroy(spl);
    free(contenido2);
    free(contenido);
}

void liberar_entrada_TPG(entrada_tabla_pags *elemento)
{
    if (elemento->modificada)
    {
        actualizar_pagina_en_storage(elemento, true);
    }

    marco* el_frame = list_get(lista_frames, elemento->marco);
    el_frame->libre=true;

    char* file = strtok(elemento->file_tag, ":");
    char* tag = strtok(NULL, ":");
    log_info(logger, "Query <%d>: Se libera el Marco: <%d> perteneciente al - File: <%s> - Tag: <%s>", actual_worker->id_query, elemento->marco, file, tag);

    free(elemento->file_tag);
    free(elemento);
    return;
}


entrada_tabla_pags* buscar_victima_lru(){
    //Al actualizar las prioridades de la tabla de paginas global siempre
    //que se acceda a la misma provoca que al hacer pop(tabla_pags_global)
    //se saque el de Least Recently Used
    entrada_tabla_pags* aux = queue_pop(tabla_pags_global);
    log_light_green(logger, "Query <%d>: Se selecciona como víctima la pagina <%d> del - File:Tag : <%s>",
        actual_worker->id_query, aux->pag, aux->file_tag);

    return aux;
}

entrada_tabla_pags* buscar_victima_clock_modificado(){
    if (queue_size(tabla_pags_global) == 0) {
        log_error(logger, "CLOCK-M: tabla_pags_global vacía, no hay víctima para seleccionar");
        return NULL;
    }
    //  Primera pasada: Buscar (0,0) 
    for (int i = 0; i < queue_size(tabla_pags_global); i++) {
        entrada_tabla_pags* elemento = queue_pop(tabla_pags_global);
        if (elemento->uso == false && elemento->modificada == false) {
            //liberar_entrada_TPG(elemento);
            return elemento;
        }
        queue_push(tabla_pags_global, elemento);
    }
    
    // Segunda pasada: Buscar (0,1) y resetear el bit de uso ---
    // Si llegamos a esta parte, no encontramos un (0,0).
    for (int i = 0; i < queue_size(tabla_pags_global); i++) {
        
        entrada_tabla_pags* elemento = queue_pop(tabla_pags_global);
        //DIMA: Recuerden que si la lista del queue tabla_pags_global es vacía posiblemente les retorne NULL y explote
        if (elemento->uso == false && elemento->modificada == true) {
            // Encontramos la víctima (0,1). La liberamos y salimos.
            //liberar_entrada_TPG(elemento);
            return elemento;
        }
        
        // Si no es la víctima, reseteamos su bit de uso.
        elemento->uso = false;
        
        // Lo volvemos a poner al final de la cola para la próxima pasada.
        queue_push(tabla_pags_global, elemento);
    }
    buscar_victima_clock_modificado();
}

entrada_tabla_pags* seleccionar_victima()
{
    entrada_tabla_pags* victima;
    log_warning(logger, "Algoritmo de reemplazo seleccionado: %d",
        cw.algoritmo_reemplazo);

    log_trace(logger, "Estado actual de la tabla de páginas global ANTES de seleccionar víctima:");
    loguear_tabla_paginas_global();

    if (R_LRU == cw.algoritmo_reemplazo)
    {
        log_warning(logger, "Usando LRU para seleccionar víctima");
        victima = buscar_victima_lru();
    }
    else
    {
        log_warning(logger, "Usando CLOCK MODIFICADO para seleccionar víctima");
        victima =buscar_victima_clock_modificado();
    }
    log_trace(logger, "Estado actual de la tabla de páginas global DESPUES de seleccionar víctima:");
    loguear_tabla_paginas_global();

    return victima;
}

bool contiene_string(t_list* lista, char* valor) {
    for (int i = 0; i < list_size(lista); i++) {
        char* elem = list_get(lista, i);
        if (strcmp(elem, valor) == 0) {
            return true;
        }
    }
    return false;
}

t_list* obtener_file_tags_unicos(t_list* entradas) {
    t_list* unicos = list_create();

    for (int i = 0; i < list_size(entradas); i++) {
        entrada_tabla_pags* entry = list_get(entradas, i);

        if (!contiene_string(unicos, entry->file_tag)) {
            list_add(unicos, entry->file_tag);   // agrego el puntero original
        }
    }

    return unicos;
}

void flushear_tabla_paginas(bool reportar_error){
    
    t_list* file_tags = obtener_file_tags_unicos(tabla_pags_global->elements);

    for(int i = 0; i < list_size(file_tags); i++){
        ejecutar_flush(i, reportar_error);
    }
}

#endif