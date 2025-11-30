#ifndef COMMUNICATION_OPS_COMMIT_TAG_H
#define COMMUNICATION_OPS_COMMIT_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif
#ifndef CRYPTO_H_
#include "commons/crypto.h"
#endif


void set_blocks_in_metadata(t_config* metadata, t_list* blocks) {
    // Construir string tipo "[1,2,3]"
    char* blocks_str = string_new();
    string_append(&blocks_str, "[");

    int size = list_size(blocks);
    for(int i = 0; i < size; i++){
        int numero = (int) list_get(blocks, i);

        char* num_str = string_from_format("%d", numero);
        string_append(&blocks_str, num_str);
        free(num_str);

        if(i < size - 1)
            string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    // Guardar en el metadata
    config_set_value(metadata, "BLOCKS", blocks_str);

    free(blocks_str);
}



void commit_tag_ops(char* file, char* tag, worker* w){

    //Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }
    if(!file_tag_exist_or_not(file, tag, w)){
        return; //Ya se envió el error al worker
    }
    t_config* metadata = get_metadata_from_file_tag(cs, file, tag);
    if(get_state_metadata(metadata) == COMMITED){
        log_info(logger, "El tag %s del archivo %s ya estaba comiteado, no se hace nada (%s:%d)", tag, file, __func__, __LINE__);
        config_destroy(metadata);
        return;
    }

    //Según la condiciión si no está comiteado debo especificarlo como comiteado
    set_state_metadata_from_config(metadata, COMMITED);

    t_config* bhi = get_block_hash_index(cs);
    char* logical_dir = get_logical_blocks_dir(cs, file, tag);

    // Obtener la lista de bloques físicos desde el metadata
    t_list* bloques_fisicos = get_array_blocks_as_list_from_metadata(metadata);
    int cantidad_bloques = list_size(bloques_fisicos);
    // Iterar por cada bloque lógico del File:Tag
    for(int i = 0; i < cantidad_bloques; i++){
        // Obtener el número del bloque físico actual desde el metadata
        int bloque_fisico_actual = (int)list_get(bloques_fisicos, i);

        // Construir el path del bloque lógico
        char* block_logical_name = get_block_name_logical(i);
        char* logical_block_path = string_from_format("%s/%s", logical_dir, block_logical_name);
        free(block_logical_name);
        // Leer el contenido del bloque lógico (que es un hard link al bloque físico)
        FILE* f = fopen(logical_block_path, "rb");
        if(f == NULL){
            log_error(logger, "[COMMIT_TAG] No se pudo abrir el bloque lógico %s", logical_block_path);
            free(logical_block_path);
            continue;
        }

        // Reservar buffer para leer el contenido del bloque
        char* buffer = malloc(g_block_size);
        if(buffer == NULL){
            log_error(logger, "[COMMIT_TAG] No se pudo reservar memoria para el buffer");
            fclose(f);
            free(logical_block_path);
            continue;
        }

        // Leer el contenido del bloque
        size_t bytes_leidos = fread(buffer, 1, g_block_size, f);
        fclose(f);

        if(bytes_leidos != g_block_size){
            log_warning(logger, "[COMMIT_TAG] Se leyeron %zu bytes en lugar de %d del bloque lógico %d", bytes_leidos, g_block_size, i);
        }

        // Calcular el hash MD5 del contenido del bloque
        char* block_hash = crypto_md5(buffer, g_block_size);
        free(buffer);

        // Verificar si ya existe un bloque físico con el mismo hash
        if(exists_hash_in_block_hash(bhi, block_hash)){
            // Obtener el nombre del bloque físico pre-existente (ej: "block0005")
            char* bloque_fisico_existente_str = config_get_string_value(bhi, block_hash);

            // Extraer el número del bloque físico existente (ej: "block0005" -> 5)
            int bloque_fisico_existente = atoi(bloque_fisico_existente_str + 5); // Skip "block" prefix

            // Si el bloque físico actual es diferente al pre-existente, hacer la reasignación
            if(bloque_fisico_actual != bloque_fisico_existente){
                // Construir el path del bloque físico antiguo
                char* block_physical_name = get_block_name_physical(bloque_fisico_actual);
                char* physical_block_dir = get_physical_blocks_dir(cs);
                char* physical_block_old_path = string_from_format("%s/%s", physical_block_dir, block_physical_name);
                free(block_physical_name);
                
                // Eliminar el hard link del bloque lógico actual
                if(unlink(logical_block_path) != 0){
                    log_error(logger, "[COMMIT_TAG] Error al hacer unlink del bloque lógico %s (errno=%d)", logical_block_path, errno);
                    free(physical_block_old_path);
                    free(logical_block_path);
                    free(physical_block_dir);
                    free(block_hash);
                    continue;
                }

                log_info(logger, "## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d", w->id_query, file, tag, i, bloque_fisico_actual);

                // Crear un nuevo hard link al bloque físico pre-existente
                char* new_block_physical_name = get_block_name_physical(bloque_fisico_existente);
                char* physical_block_new_path = string_from_format("%s/%s", physical_block_dir, new_block_physical_name);
                free(new_block_physical_name);

                crear_hard_link(physical_block_new_path, logical_block_path);

                log_info(logger, "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico %d", w->id_query, file, tag, i, bloque_fisico_existente);
                log_info(logger, "## %d - %s:%s Bloque Lógico %d se reasigna de %d a %d", w->id_query, file, tag, i, bloque_fisico_actual, bloque_fisico_existente);

                // Verificar si el bloque físico antiguo tiene más referencias
                struct stat st;
                if(stat(physical_block_old_path, &st) == -1){
                    log_error(logger, "[COMMIT_TAG] Error al hacer stat del bloque físico %s (errno=%d)", physical_block_old_path, errno);
                } else {
                    // Si solo queda el archivo físico sin referencias lógicas, liberarlo
                    if(st.st_nlink == 1){
                        liberar_bloque(g_bitmap, bloque_fisico_actual, g_bitmap_size);
                        log_info(logger, "## %d - Bloque Físico Liberado - Número de Bloque: %d", w->id_query, bloque_fisico_actual);
                    }
                }

                list_replace(bloques_fisicos, i, (void*) bloque_fisico_existente);
                set_blocks_in_metadata(metadata, bloques_fisicos);
                config_save(metadata);

                free(physical_block_old_path);
                free(physical_block_new_path);
            }
        } else {
            // Si no existe el hash, agregarlo al block_hash_index
            char* block_name = get_block_name_by_n(bloque_fisico_actual, NUMBER_OF_DIGITS_BLOCK);
            //char* block_name = string_from_format("block%04d", bloque_fisico_actual);
            insert_hash_block(bhi, block_hash, block_name);
            free(block_name);
        }

        free(block_hash);
        free(logical_block_path);
    }

    list_destroy(bloques_fisicos);
    free(logical_dir);
    config_destroy(metadata);
    config_destroy(bhi);

    // Log de commit exitoso
    log_info(logger, "## %d - Commit de File:Tag %s:%s", w->id_query, file, tag);

    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}


#endif