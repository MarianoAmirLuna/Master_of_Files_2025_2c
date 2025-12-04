#ifndef COMMUNICATION_OPS_TAG_FILE_H
#define COMMUNICATION_OPS_TAG_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef CONTROL_ACCESOS_H
#include "../control_accesos.h"
#endif

void tag_file_ops(char* file, char* tag_origen, char* file_destino, char* tag_destino, worker* w){

    //Control de que no se reciban cosas nulas
    if(file == NULL || tag_origen == NULL || file_destino == NULL || tag_destino == NULL){
        log_error(logger, "FILE, TAG_ORIGEN, file_destino o TAG_DESTINO son nulos");
        return;
    }

    if(!file_tag_exist_or_not(file, tag_origen, w)){
        return; //Ya se envió el error al worker
    }
    log_orange(logger, "[TAG_FILE] Iniciando operacion TAG para %s:%s -> %s:%s", file, tag_origen, file, tag_destino);

    char* path_file_destino =  get_file_path(cs, file_destino);

    if (!control_existencia(path_file_destino)){
        create_nested_directories(path_file_destino);
    }

    char* path_tag_destino = get_filetag_path(cs, file, tag_destino);
    char* path_tag_origen = get_filetag_path(cs, file, tag_origen);

    if (control_existencia(path_tag_destino)){
        log_error(logger, "[TAG_FILE] El tag destino %s ya existe", tag_destino);
        free(path_tag_destino);
        return;
    }

    //Leer metadata del tag de origen
    char* path_metadata_origen =get_metadata_fullpath(cs, file, tag_origen);
    t_config* metadata_origen = get_metadata_from_file_tag(cs, file, tag_origen);
    if (metadata_origen == NULL){
        log_error(logger, "[TAG_FILE] No se pudo abrir metadata del tag origen: %s", path_metadata_origen);
        free(path_tag_origen);
        free(path_tag_destino);
        free(path_metadata_origen);
        return;
    }

    int tamanio_origen =get_size_from_metadata(metadata_origen);
    t_list* bloques_origen = get_array_blocks_as_list_from_metadata(metadata_origen);

    int cantidad_bloques = list_size(bloques_origen);
    log_orange(logger, "[TAG_FILE] Tag origen tiene %d bloques", cantidad_bloques);

    // Validar que hay suficientes bloques físicos libres antes de empezar a clonar
    int total_bloques = g_fs_size / g_block_size;
    int bloques_libres = 0;
    for(int i = 0; i < total_bloques; i++){
        if(!bloque_ocupado(g_bitmap, i)){
            bloques_libres++;
        }
    }

    log_orange(logger, "[TAG_FILE] Bloques necesarios: %d, Bloques libres disponibles: %d",
               cantidad_bloques, bloques_libres);

    // Verificar que hay suficientes bloques libres
    if(bloques_libres < cantidad_bloques){
        send_basic_packet(w->fd, INSUFFICIENT_SPACE);
        log_error(logger, "[TAG_FILE] Espacio insuficiente para clonar %s:%s -> %s:%s - Se necesitan %d bloques pero solo hay %d disponibles",
                  file, tag_origen, file, tag_destino, cantidad_bloques, bloques_libres);
        list_destroy(bloques_origen);
        config_destroy(metadata_origen);
        free(path_tag_origen);
        free(path_tag_destino);
        free(path_metadata_origen);
        return;
    }

    char* logical_blocks_dir = get_logical_blocks_dir(cs, file,tag_destino);
    //char* logical_blocks_dir = string_from_format("%s/logical_blocks", path_tag_destino);
    create_nested_directories(logical_blocks_dir);


    t_list* bloques_destino = list_create();
    for (int i = 0; i < cantidad_bloques; i++){
        int bloque_fisico_origen = (int)list_get(bloques_origen, i);
        //int bloque_fisico_origen = bloque_origen_ptr;

        // Obtener lock del bloque físico origen
        pthread_mutex_lock(&block_locks[bloque_fisico_origen]);

        // Clonar el bloque físico (incluye verificación de hash MD5 y hasta 3 reintentos)
        int bloque_fisico_destino = clonar_bloque_fisico(
            bloque_fisico_origen,
            g_bitmap,
            g_bitmap_size,
            g_block_size,
            g_fs_size,
            cs.punto_montaje,
            &bitmap_lock
        );

        pthread_mutex_unlock(&block_locks[bloque_fisico_origen]);

        // Verificar que se clonó correctamente (después de 3 intentos con verificación de hash)
        if (bloque_fisico_destino == -1){
            log_error(logger, "[TAG_FILE] Error al clonar bloque físico %d después de 3 intentos", bloque_fisico_origen);

            // Cleanup: liberar bloques ya clonados (con protección de mutex)
            pthread_mutex_lock(&bitmap_lock);
            for (int j = 0; j < list_size(bloques_destino); j++){
                liberar_bloque(g_bitmap, (int)list_get(bloques_destino, j), g_bitmap_size);
            }
            pthread_mutex_unlock(&bitmap_lock);

            // Cleanup: eliminar directorio creado
            // TODO: implementar función para eliminar directorio recursivamente
            
            list_destroy(bloques_origen);
            list_destroy(bloques_destino);
            config_destroy(metadata_origen);
            free(logical_blocks_dir);
            free(path_tag_origen);
            free(path_tag_destino);
            free(path_metadata_origen);
            return;
        }

        log_orange(logger, "[TAG_FILE] Bloque físico %d clonado a bloque %d", bloque_fisico_origen, bloque_fisico_destino);

        // Agregar el nuevo bloque a la lista de bloques destino
        list_add(bloques_destino, bloque_fisico_destino);

        // Crear hard link del bloque lógico al bloque físico clonado
        char* block_physical_dir = get_physical_blocks_dir(cs);
        char* block_physical_name = get_block_name_physical(bloque_fisico_destino);
        char* path_bloque_fisico = string_from_format("%s/%s", block_physical_dir, block_physical_name);
        
        //char* block_logical_dir = get_logical_blocks_dir(cs, file,tag_destino);
        char* block_logical_name = get_block_name_logical(i);
        char* path_bloque_logico = string_from_format("%s/%s",logical_blocks_dir, block_logical_name);
       
        
        crear_hard_link(path_bloque_fisico, path_bloque_logico);

        log_info(logger, "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico %d", w->id_query, file, tag_destino, i, bloque_fisico_destino);
         
        free(block_physical_dir);
        free(block_physical_name);
        //free(block_logical_dir);
        free(block_logical_name);
        free(path_bloque_fisico);
        free(path_bloque_logico);
    }

    //Crear metadata del tag destino
    char* path_metadata_destino = string_from_format("%s/metadata.config", path_tag_destino);

    // Log de verificación: mostrar los bloques físicos que se van a guardar
    log_orange(logger, "[TAG_FILE] Guardando metadata con %d bloques físicos:", list_size(bloques_destino));
    for (int i = 0; i < list_size(bloques_destino); i++) {
        log_orange(logger, "[TAG_FILE]   - Bloque lógico %d -> Bloque físico %d", i, (int)list_get(bloques_destino, i));
    }

    t_config* metadata_destino= crear_metadata_config(path_metadata_destino, tamanio_origen, bloques_destino, WORK_IN_PROGRESS);

    log_orange(logger, "[TAG_FILE] Tag creado exitosamente: %s:%s", file, tag_destino);
    log_info(logger, "## %d - Tag creado %s:%s", w->id_query, file, tag_destino);

    // Libero recursos
    //Al ser int sin puntero la lista, no debe ser liberado con destroy_elements, pero sí con list_destroy
    /*list_destroy_and_destroy_elements(bloques_origen, free);
    list_destroy_and_destroy_elements(bloques_destino, free);*/ 
    list_destroy(bloques_origen);
    list_destroy(bloques_destino);

    config_destroy(metadata_origen);
    config_destroy(metadata_destino);
    free(path_tag_origen);
    free(path_tag_destino);
    free(path_metadata_origen);
    free(path_metadata_destino);
    free(logical_blocks_dir);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
    t_packet* response = create_packet();
    add_int_to_packet(response, SUCCESS);
    send_and_free_packet(response, w->fd);
}



#endif