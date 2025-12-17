#ifndef COMMUNICATION_OPS_TRUNCATE_FILE_H
#define COMMUNICATION_OPS_TRUNCATE_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef CONTROL_ACCESOS_H
#include "../control_accesos.h"
#endif

#ifndef BITMAP_EXT_H
#include "exts/bitmap_ext.h"
#endif

void truncate_file_ops(char* file, char* tag, int nuevo_tam, worker* w){
    /*Esta operación se encargará de modificar el tamaño del File:Tag especificados agrandando o achicando el tamaño del mismo
    para reflejar el nuevo tamaño deseado (actualizando la metadata necesaria).
    Al incrementar el tamaño del File, se le asignarán tantos bloques lógicos (hard links) como sea necesario.
    Inicialmente, todos ellos deberán apuntar el bloque físico nro 0.
    Al reducir el tamaño del File, se deberán desasignar tantos bloques lógicos como sea necesario (empezando por el final del archivo).
    Si el bloque físico al que apunta el bloque lógico eliminado no es referenciado por ningún otro File:Tag,
    deberá ser marcado como libre en el bitmap.*/

    // Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }

    // Verificar que File:Tag existe
    if(!file_tag_exist_or_not(file, tag, w)){
        return; // Ya se envió el error al worker
    }

    // Obtener metadata del tag
    log_pink(logger, "FILE: %s, TAG: %s", file, tag);
    t_config* metadata = get_metadata_from_file_tag(cs, file, tag);
    if(metadata == NULL){
        log_error(logger, "[TRUNCATE] No se pudo obtener metadata de %s:%s", file, tag);
        return;
    }

    // Verificar que el tag NO esté en estado COMMITED
    if(get_state_metadata(metadata) == COMMITED){
        send_basic_packet(w->fd, WRITE_NO_PERMISSION);
        log_error(logger, "[TRUNCATE] Tag %s:%s está COMMITED, no se puede truncar", file, tag);
        config_destroy(metadata);
        return;
    }

    // Obtener tamaño actual y lista de bloques
    int tamanio_actual = get_size_from_metadata(metadata);
    t_list* bloques_fisicos = get_array_blocks_as_list_from_metadata(metadata);
    int bloques_actuales = list_size(bloques_fisicos);

    // Calcular bloques necesarios para el nuevo tamaño
    int bloques_necesarios = (nuevo_tam == 0) ? 0 : (int)ceil((double)nuevo_tam / (double)g_block_size);

    log_debug(logger, "[TRUNCATE] %s:%s - Tamaño actual: %d, Nuevo tamaño: %d", file, tag, tamanio_actual, nuevo_tam);
    log_debug(logger, "[TRUNCATE] Bloques actuales: %d, Bloques necesarios: %d", bloques_actuales, bloques_necesarios);

    // Obtener paths necesarios
    char* logical_blocks_dir = get_logical_blocks_dir(cs, file, tag);
    char* physical_blocks_dir = get_physical_blocks_dir(cs);

    // CASO 1: Incrementar tamaño - agregar bloques lógicos apuntando al bloque físico 0
    if(bloques_necesarios > bloques_actuales){
        // Calcular cuántos bloques nuevos se necesitan agregar
        int bloques_a_agregar = bloques_necesarios - bloques_actuales;

        // Contar bloques físicos libres disponibles en el bitmap
        int total_bloques = g_fs_size / g_block_size;
        int bloques_libres = 0;
        for(int i = 0; i < total_bloques; i++){
            if(!bloque_ocupado(g_bitmap, i)){
                bloques_libres++;
            }
        }

        log_debug(logger, "[TRUNCATE] Bloques a agregar: %d, Bloques libres disponibles: %d",
                  bloques_a_agregar, bloques_libres);

        // Verificar que hay suficientes bloques libres
        // Nota: Aunque inicialmente apuntan al bloque 0, cuando se escriban necesitarán bloques físicos propios
        if(bloques_libres < bloques_a_agregar){
            send_basic_packet(w->fd, INSUFFICIENT_SPACE);
            log_error(logger, "[TRUNCATE] Espacio insuficiente para %s:%s - Se necesitan %d bloques pero solo hay %d disponibles",
                      file, tag, bloques_a_agregar, bloques_libres);
            list_destroy(bloques_fisicos);
            config_destroy(metadata);
            free(logical_blocks_dir);
            free(physical_blocks_dir);
            return;
        }

        // Construir path del bloque físico 0
        char* physical_block_0_name = get_block_name_physical(0);
        char* physical_block_0_path = string_from_format("%s/%s", physical_blocks_dir, physical_block_0_name);
        free(physical_block_0_name);

        for(int i = bloques_actuales; i < bloques_necesarios; i++){
            // Aplicar retardo de acceso a bloque
            msleep(cs.retardo_acceso_bloque);

            // Construir path del nuevo bloque lógico
            char* logical_block_name = get_block_name_logical(i);
            char* logical_block_path = string_from_format("%s/%s", logical_blocks_dir, logical_block_name);
            free(logical_block_name);

            // Crear hard link al bloque físico 0
            crear_hard_link(physical_block_0_path, logical_block_path);

            log_info(logger, "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico 0",
                     w->id_query, file, tag, i);

            // Agregar bloque 0 a la lista de bloques
            list_add(bloques_fisicos, (void*)0);

            free(logical_block_path);
        }

        free(physical_block_0_path);
    }
    // CASO 2: Reducir tamaño - eliminar bloques lógicos desde el final
    else if(bloques_necesarios < bloques_actuales){
        for(int i = bloques_actuales - 1; i >= bloques_necesarios; i--){
            // Aplicar retardo de acceso a bloque
            msleep(cs.retardo_acceso_bloque);

            // Obtener el número del bloque físico desde la lista
            int bloque_fisico = (int)list_get(bloques_fisicos, i);

            // Construir paths
            char* logical_block_name = get_block_name_logical(i);
            char* logical_block_path = string_from_format("%s/%s", logical_blocks_dir, logical_block_name);
            free(logical_block_name);

            char* physical_block_name = get_block_name_physical(bloque_fisico);
            char* physical_block_path = string_from_format("%s/%s", physical_blocks_dir, physical_block_name);
            free(physical_block_name);

            // Eliminar el hard link del bloque lógico
            if(unlink(logical_block_path) == 0){
                log_info(logger, "## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
                         w->id_query, file, tag, i, bloque_fisico);
            } else {
                log_warning(logger, "[TRUNCATE] No se pudo eliminar el hard link %s (errno=%d)",
                            logical_block_path, errno);
            }

            // Usar stat() para verificar cuántas referencias tiene el bloque físico
            struct stat st;
            if(stat(physical_block_path, &st) == 0){
                // Si st_nlink == 1, solo queda el archivo físico en physical_blocks
                // (nadie más lo está usando) -> LIBERAR del bitmap
                if(st.st_nlink == 1){
                    pthread_mutex_lock(&bitmap_lock);
                    liberar_bloque(g_bitmap, bloque_fisico, g_bitmap_size);
                    pthread_mutex_unlock(&bitmap_lock);
                    log_info(logger, "## %d - Bloque Físico Liberado - Número de Bloque: %d",
                             w->id_query, bloque_fisico);
                }
            } else {
                log_error(logger, "[TRUNCATE] Error al hacer stat del bloque físico %s (errno=%d)",
                          physical_block_path, errno);
            }

            // Remover el último elemento de la lista de bloques
            list_remove(bloques_fisicos, i);

            free(logical_block_path);
            free(physical_block_path);
        }
    }

    // Actualizar metadata con el nuevo tamaño y lista de bloques
    char* metadata_path = get_metadata_fullpath(cs, file, tag);
    t_config* updated_metadata = crear_metadata_config(metadata_path, nuevo_tam, bloques_fisicos, WORK_IN_PROGRESS);
    config_destroy(updated_metadata);
    free(metadata_path);

    // Liberar recursos
    list_destroy(bloques_fisicos);
    config_destroy(metadata);
    free(logical_blocks_dir);
    free(physical_blocks_dir);

    // Log de truncado exitoso
    log_info(logger, "## %d - File Truncado %s:%s - Tamaño: %d", w->id_query, file, tag, nuevo_tam);

    // Si necesitan decirle algo al worker desde este método se crea el packet y se envía en w->fd
    // Ejemplo: send_and_free_packet(p, w->fd);
    t_packet* response = create_packet();
    add_int_to_packet(response, SUCCESS);
    send_and_free_packet(response, w->fd);
}

#endif