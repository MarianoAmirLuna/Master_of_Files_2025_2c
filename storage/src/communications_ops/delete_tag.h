#ifndef COMMUNICATION_OPS_DELETE_TAG_H
#define COMMUNICATION_OPS_DELETE_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif
#ifndef BITMAP_EXT_H
#include "exts/bitmap_ext.h"
#endif

void delete_tag_ops(char* file, char* tag, worker* w){
    /*Esta operación eliminará el directorio correspondiente al File:Tag indicado.
    Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado
    no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.*/
    // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).
    /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
        File_inexistente
        Tag_inexistente
    */

    // Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }

    // Aplicar retardo de operación
    //Esto no es necesario porque ya se aplica en la función tratar_mensaje en comunicacion_worker.h
    //usleep(cs.retardo_operacion * 1000);

    // Verificar que File:Tag existe
    if(!file_tag_exist_or_not(file, tag, w)){
        return; // Ya se envió el error al worker
    }

    // Obtener metadata del tag ANTES de eliminar el directorio
    t_config* metadata = get_metadata_from_file_tag(cs, file, tag);
    if(metadata == NULL){
        log_error(logger, "[DELETE_TAG] No se pudo obtener metadata de %s:%s", file, tag);
        return;
    }

    // Obtener lista de bloques físicos del metadata
    t_list* bloques_fisicos = get_array_blocks_as_list_from_metadata(metadata);
    int cantidad_bloques = list_size(bloques_fisicos);

    // Construir el path del directorio del File:Tag
    char* fullpath = get_filetag_path(cs, file, tag);
    char* physical_blocks_dir= get_physical_blocks_dir(cs);
    char* logical_blocks_dir = get_logical_blocks_dir(cs, file, tag);
    // Iterar por cada bloque lógico del File:Tag
    for(int i = 0; i < cantidad_bloques; i++){
        // Aplicar retardo de acceso a bloque
        msleep(cs.retardo_acceso_bloque);

        // Obtener el número del bloque físico desde el metadata
        int bloque_fisico = (int)list_get(bloques_fisicos, i);

        // Construir el path del bloque lógico
        char* logical_name_block = get_block_name_logical(i);
        char* logical_block_path = string_from_format("%s/%s", logical_blocks_dir, logical_name_block);
        free(logical_name_block);

        // Construir el path del bloque físico
        char* physical_name_block = get_block_name_physical(bloque_fisico);
        char* physical_block_path = string_from_format("%s/%s", physical_blocks_dir, physical_name_block);
        free(physical_name_block);

        // Eliminar el hard link del bloque lógico
        if(unlink(logical_block_path) == 0){
            log_info(logger, "## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
                     w->id_query, file, tag, i, bloque_fisico);
        } else {
            log_warning(logger, "[DELETE_TAG] No se pudo eliminar el hard link %s (errno=%d)",
                        logical_block_path, errno);
        }

        // Usar stat() para verificar cuántas referencias tiene el bloque físico
        struct stat st;
        if(stat(physical_block_path, &st) == 0){
            // Si st_nlink == 1, solo queda el archivo físico en physical_blocks
            // (nadie más lo está usando) -> LIBERAR del bitmap
            if(st.st_nlink == 1){
                liberar_bloque(g_bitmap, bloque_fisico, g_bitmap_size);
                log_info(logger, "## %d - Bloque Físico Liberado - Número de Bloque: %d",
                         w->id_query, bloque_fisico);
            }
        } else {
            log_error(logger, "[DELETE_TAG] Error al hacer stat del bloque físico %s (errno=%d)",
                      physical_block_path, errno);
        }

        free(logical_block_path);
        free(physical_block_path);
    }

    // Liberar memoria de la lista de bloques
    list_destroy(bloques_fisicos);
    config_destroy(metadata);

    // Ahora sí eliminar el directorio completo del File:Tag
    delete_directory(fullpath);

    // Log final de eliminación exitosa
    log_info(logger, "## %d - Tag Eliminado %s:%s", w->id_query, file, tag);

    free(fullpath);
    free(physical_blocks_dir);
    free(logical_blocks_dir);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif