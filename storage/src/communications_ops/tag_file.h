#ifndef COMMUNICATION_OPS_TAG_FILE_H
#define COMMUNICATION_OPS_TAG_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef CONTROL_ACCESOS_H
#include "../control_accesos.h"
#endif

void tag_file_ops(char* file, char* tag_origen, char* tag_destino, worker* w){

    //Control de que no se reciban cosas nulas
    if(file == NULL || tag_origen == NULL || tag_destino == NULL){
        log_error(logger, "FILE, TAG_ORIGEN o TAG_DESTINO son nulos");
        return;
    }
        
    if(!file_tag_exist_or_not(file, tag_origen, w)){
        return; //Ya se envió el error al worker
    }
    log_orange(logger, "[TAG_FILE] Iniciando operacion TAG para %s:%s -> %s:%s", file, tag_origen, file, tag_destino);

    // 1. Verificar que el file de origen existe
    char* path_file = string_from_format("%s/files/%s", cs.punto_montaje, file);
    if (!control_existencia(path_file)){
        log_error(logger, "[TAG_FILE] El archivo %s no existe", file);
        free(path_file);
        return;
    }

    // 2. Verificar que el tag de origen existe
    char* path_tag_origen = string_from_format("%s/%s", path_file, tag_origen);
    if (!control_existencia(path_tag_origen)){
        log_error(logger, "[TAG_FILE] El tag de origen %s no existe", tag_origen);
        free(path_file);
        free(path_tag_origen);
        return;
    }

    // 3. Verificar que el tag destino NO existe
    char* path_tag_destino = string_from_format("%s/%s", path_file, tag_destino);
    if (control_existencia(path_tag_destino)){
        log_error(logger, "[TAG_FILE] El tag destino %s ya existe", tag_destino);
        free(path_file);
        free(path_tag_origen);
        free(path_tag_destino);
        return;
    }

    // 4. Leer metadata del tag de origen
    char* path_metadata_origen = string_from_format("%s/metadata.config", path_tag_origen);
    t_config* metadata_origen = config_create(path_metadata_origen);
    if (metadata_origen == NULL){
        log_error(logger, "[TAG_FILE] No se pudo abrir metadata del tag origen: %s", path_metadata_origen);
        free(path_file);
        free(path_tag_origen);
        free(path_tag_destino);
        free(path_metadata_origen);
        return;
    }

    // Leer tamaño y bloques del tag origen
    int tamanio_origen = config_get_int_value(metadata_origen, "TAMAÑO");
    char** bloques_array = config_get_array_value(metadata_origen, "BLOCKS");

    // Convertir array de bloques a lista
    t_list* bloques_origen = list_create();
    if (bloques_array != NULL){
        for (int i = 0; bloques_array[i] != NULL; i++){
            int* bloque_fisico = malloc(sizeof(int));
            *bloque_fisico = atoi(bloques_array[i]);
            list_add(bloques_origen, bloque_fisico);
            free(bloques_array[i]);
        }
        free(bloques_array);
    }

    int cantidad_bloques = list_size(bloques_origen);
    log_orange(logger, "[TAG_FILE] Tag origen tiene %d bloques", cantidad_bloques);

    // 5. Crear directorio del tag destino
    crear_directorio(tag_destino, path_file);

    // 6. Crear directorio logical_blocks dentro del tag destino
    char* path_logical_blocks_destino = string_from_format("%s/logical_blocks", path_tag_destino);
    crear_directorio("logical_blocks", path_tag_destino);

    // 7. Clonar bloques físicos y crear hard links
    t_list* bloques_destino = list_create();

    for (int i = 0; i < cantidad_bloques; i++){
        int* bloque_origen_ptr = (int*)list_get(bloques_origen, i);
        int bloque_fisico_origen = *bloque_origen_ptr;

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
                int* bloque_liberado = (int*)list_get(bloques_destino, j);
                liberar_bloque(g_bitmap, *bloque_liberado, g_bitmap_size);
            }
            pthread_mutex_unlock(&bitmap_lock);

            // Cleanup: eliminar directorio creado
            // TODO: implementar función para eliminar directorio recursivamente

            list_destroy_and_destroy_elements(bloques_origen, free);
            list_destroy_and_destroy_elements(bloques_destino, free);
            config_destroy(metadata_origen);
            free(path_file);
            free(path_tag_origen);
            free(path_tag_destino);
            free(path_metadata_origen);
            free(path_logical_blocks_destino);
            return;
        }

        log_orange(logger, "[TAG_FILE] Bloque físico %d clonado a bloque %d", bloque_fisico_origen, bloque_fisico_destino);

        // Agregar el nuevo bloque a la lista de bloques destino
        int* bloque_destino_ptr = malloc(sizeof(int));
        *bloque_destino_ptr = bloque_fisico_destino;
        list_add(bloques_destino, bloque_destino_ptr);

        // Crear hard link del bloque lógico al bloque físico clonado
        char* path_bloque_fisico = string_from_format("%s/physical_blocks/block%04d.dat",
                                                       cs.punto_montaje, bloque_fisico_destino);
        char* path_bloque_logico = string_from_format("%s/%06d.dat",
                                                       path_logical_blocks_destino, i);

        crear_hard_link(path_bloque_fisico, path_bloque_logico);

        log_info(logger, "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico %d",
                 w->id_query, file, tag_destino, i, bloque_fisico_destino);

        free(path_bloque_fisico);
        free(path_bloque_logico);
    }

    // 8. Crear metadata del tag destino
    char* path_metadata_destino = string_from_format("%s/metadata.config", path_tag_destino);

    // Log de verificación: mostrar los bloques físicos que se van a guardar
    log_orange(logger, "[TAG_FILE] Guardando metadata con %d bloques físicos:", list_size(bloques_destino));
    for (int i = 0; i < list_size(bloques_destino); i++) {
        int* bloque_ptr = (int*)list_get(bloques_destino, i);
        log_orange(logger, "[TAG_FILE]   - Bloque lógico %d -> Bloque físico %d", i, *bloque_ptr);
    }

    crear_metadata_config(path_metadata_destino, tamanio_origen, bloques_destino, WORK_IN_PROGRESS);

    log_orange(logger, "[TAG_FILE] Tag creado exitosamente: %s:%s", file, tag_destino);
    log_info(logger, "## %d - Tag creado %s:%s", w->id_query, file, tag_destino);

    // Libero recursos
    list_destroy_and_destroy_elements(bloques_origen, free);
    list_destroy_and_destroy_elements(bloques_destino, free);
    config_destroy(metadata_origen);
    free(path_file);
    free(path_tag_origen);
    free(path_tag_destino);
    free(path_metadata_origen);
    free(path_metadata_destino);
    free(path_logical_blocks_destino);

    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}



#endif