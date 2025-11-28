#ifndef COMMUNICATION_OPS_WRITE_BLOCK_H
#define COMMUNICATION_OPS_WRITE_BLOCK_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef CONTROL_ACCESOS_H
#include "../control_accesos.h"
#endif

void write_block_ops(char* file, char* tag, int bloque_logico, char* contenido, worker* w, bool reportar_error){
    /*Esta operación recibirá el contenido de un bloque lógico de un File:Tag y guardará los cambios en el 
    bloque físico correspondiente, siempre y cuando el File:Tag no se encuentre en estado COMMITED y 
    el bloque lógico se encuentre asignado.
    Si el bloque lógico a escribir fuera el único referenciando a su bloque físico asignado, 
    se escribirá dicho bloque físico directamente. En caso contrario, se deberá buscar un nuevo bloque físico, 
    escribir en el mismo y asignarlo al bloque lógico en cuestión.*/

    // EXCEPCIONES:
    //  - File_inexistente
    //  - Tag_inexistente
    //  - Espacio_Insuficiente
    //  - Escritura_no_permitida
    //  - Lectura_o_escritura_fuera_de_limite

    if(file == NULL || tag == NULL || contenido == NULL){
        log_error(logger, "[WRITE_BLOCK] FILE, TAG o CONTENIDO son nulos");
        return;
    }

    // 1) Verificar que exista el File:Tag
    if(!file_tag_exist_or_not(file, tag, w)){
        return; // Ya se envió el error al worker
    }

    // 2) Obtener metadata
    t_config* metadata = get_metadata_from_file_tag(cs, file, tag);
    if(metadata == NULL){
        log_error(logger, "[WRITE_BLOCK] No se pudo obtener la metadata del File:Tag %s:%s", file, tag);
        return;
    }

    // Guardamos tamaño actual (no cambia por WRITE_BLOCK)
    int tamanio_actual = get_size_from_metadata(metadata);

    // 3) Verificar estado COMMITED
    if(reportar_error && get_state_metadata(metadata) == COMMITED){
        send_basic_packet(w->fd, WRITE_NO_PERMISSION);   // Escritura_no_permitida
        log_error(logger, "[WRITE_BLOCK] El tag %s:%s está COMMITED, no se puede escribir", file, tag);
        config_destroy(metadata);
        return;
    }

    // 4) Obtener lista de bloques físicos desde el metadata
    t_list* bloques_fisicos = get_array_blocks_as_list_from_metadata(metadata);
    int cantidad_bloques = list_size(bloques_fisicos);

    // 5) Validar que el bloque lógico exista -- Escritura fuera de limite
    if(reportar_error && (bloque_logico < 0 || bloque_logico >= cantidad_bloques)){
        // Acá deberías mandar el error de "Lectura_o_escritura_fuera_de_limite"
        // send_basic_packet(w->fd, READ_WRITE_OUT_OF_LIMIT);
        log_error(logger, "[WRITE_BLOCK] Bloque lógico fuera de límite: %d (máximo %d) para %s:%s",
                  bloque_logico, cantidad_bloques - 1, file, tag);
        send_basic_packet(w->fd, READ_WRITE_OVERFLOW); // lectura fuera de limites

        list_destroy(bloques_fisicos);
        config_destroy(metadata);
        return;
    }

    // 6) Obtener el bloque físico asociado a este bloque lógico
    int bloque_fisico_actual = (int) list_get(bloques_fisicos, bloque_logico);
    log_debug(logger, "[WRITE_BLOCK] %s:%s - Bloque lógico %d -> Bloque físico %d",
              file, tag, bloque_logico, bloque_fisico_actual);

    // 7) Lock lógico del File:Tag (a partir de acá nadie más debería tocar este Tag)
    pthread_mutex_t* tag_lock = get_file_tag_lock(file, tag);
    pthread_mutex_lock(tag_lock);

    // =============================
    // PASO 3: VER CUÁNTOS LINKS TIENE EL BLOQUE FÍSICO
    // =============================

    char* physical_dir  = get_physical_blocks_dir(cs);
    char* physical_name = get_block_name_physical(bloque_fisico_actual);
    char* physical_path = string_from_format("%s/%s", physical_dir, physical_name);

    struct stat st;
    if(stat(physical_path, &st) != 0){
        log_error(logger, "[WRITE_BLOCK] stat() falló sobre bloque físico %d (%s)", bloque_fisico_actual, physical_path);
        free(physical_dir);
        free(physical_name);
        free(physical_path);
        pthread_mutex_unlock(tag_lock);
        list_destroy(bloques_fisicos);
        config_destroy(metadata);
        return;
    }

    int cantidad_links = st.st_nlink;
    log_debug(logger, "[WRITE_BLOCK] Bloque físico %d tiene %d hard-links", bloque_fisico_actual, cantidad_links);

    // Vamos a trackear cuál es el bloque físico "final" sobre el que quedan los datos nuevos
    int bloque_fisico_final = bloque_fisico_actual;

    // =============================
    // CASO A: BLOQUE EXCLUSIVO -> escribir directamente
    // =============================
    if(cantidad_links == 2){
        log_debug(logger, "[WRITE_BLOCK] Bloque físico %d es exclusivo. Se escribirá directamente.", bloque_fisico_actual);

        // Lock del bloque físico actual
        pthread_mutex_lock(&block_locks[bloque_fisico_actual]);

        escribir_bloque_fisico(bloque_fisico_actual, contenido);

        pthread_mutex_unlock(&block_locks[bloque_fisico_actual]);
        // No cambiamos BLOCKS ni metadata en este caso
    }

    // =============================
    // CASO B: BLOQUE COMPARTIDO -> COPY-ON-WRITE
    // =============================
    else if(cantidad_links > 2){
        log_debug(logger, "[WRITE_BLOCK] Bloque físico %d se comparte (%d links). Se hace COPY-ON-WRITE.",
                  bloque_fisico_actual, cantidad_links);

        // 1) Clonar el bloque físico actual usando la misma función que en TAG_FILE
        pthread_mutex_lock(&block_locks[bloque_fisico_actual]);
        int bloque_nuevo = clonar_bloque_fisico(
            bloque_fisico_actual,
            g_bitmap,
            g_bitmap_size,
            g_block_size,
            g_fs_size,
            cs.punto_montaje,
            &bitmap_lock
        );
        pthread_mutex_unlock(&block_locks[bloque_fisico_actual]);

        if (bloque_nuevo == -1){
            log_error(logger, "[WRITE_BLOCK] Error al clonar bloque físico %d para COW", bloque_fisico_actual);

            free(physical_dir);
            free(physical_name);
            free(physical_path);
            pthread_mutex_unlock(tag_lock);
            list_destroy(bloques_fisicos);
            config_destroy(metadata);
            return;
        }

        bloque_fisico_final = bloque_nuevo;
        log_debug(logger, "[WRITE_BLOCK] COPY-ON-WRITE asignó nuevo bloque físico %d", bloque_fisico_final);

        // 2) Escribir el contenido nuevo en el bloque físico nuevo
        pthread_mutex_lock(&block_locks[bloque_fisico_final]);
        escribir_bloque_fisico(bloque_fisico_final, contenido);
        pthread_mutex_unlock(&block_locks[bloque_fisico_final]);

        // 3) Reasignar el bloque lógico: unlink viejo -> hard link nuevo
        char* new_phys_name = get_block_name_physical(bloque_fisico_final);
        char* new_phys_path = string_from_format("%s/%s", physical_dir, new_phys_name);

        char* logical_dir  = get_logical_blocks_dir(cs, file, tag);
        char* logical_name = get_block_name_logical(bloque_logico);
        char* logical_path = string_from_format("%s/%s", logical_dir, logical_name);

        // Eliminar la referencia lógica vieja
        if(unlink(logical_path) != 0){
            log_warning(logger, "[WRITE_BLOCK] No se pudo hacer unlink de %s (errno=%d)", logical_path, errno);
        }

        // Crear nuevo hard link
        crear_hard_link(new_phys_path, logical_path);

        log_info(logger, "## %d - %s:%s Bloque lógico %d se reasigna: %d -> %d",
                 w->id_query, file, tag, bloque_logico,
                 bloque_fisico_actual, bloque_fisico_final);

        // 4) Actualizar la lista BLOCKS en memoria
        list_replace(bloques_fisicos, bloque_logico, (void*)(long)bloque_fisico_final);

        free(new_phys_name);
        free(new_phys_path);
        free(logical_dir);
        free(logical_name);
        free(logical_path);
    }

    else {
        log_error(logger, "[WRITE_BLOCK] ERROR: bloque físico %d tiene links inesperados (%d)", bloque_fisico_actual, cantidad_links);
        free(physical_dir);
        free(physical_name);
        free(physical_path);
        pthread_mutex_unlock(tag_lock);
        list_destroy(bloques_fisicos);
        config_destroy(metadata);
        return;
    }

    free(physical_dir);
    free(physical_name);
    free(physical_path);

    // =============================
    // ACTUALIZAR METADATA SI HIZO FALTA
    // =============================
    // Si hicimos COW, bloques_fisicos ya está actualizado. Si no, queda igual.
    char* metadata_path = get_metadata_fullpath(cs, file, tag);
    t_config* nueva_metadata = crear_metadata_config(
        metadata_path,
        tamanio_actual,      // tamaño no cambia con WRITE_BLOCK
        bloques_fisicos,
        WORK_IN_PROGRESS     // sigue en WIP hasta que hagan COMMIT_TAG
    );
    config_destroy(nueva_metadata);
    free(metadata_path);

    pthread_mutex_unlock(tag_lock);
    list_destroy(bloques_fisicos);
    config_destroy(metadata);

    log_info(logger, "Ejecutando la operacion WRITE_BLOCK");
    log_info(logger, "## %d - Bloque Lógico Escrito %s:%s Número de Bloque %d", w->id_query, file, tag, bloque_logico);
}

#endif
