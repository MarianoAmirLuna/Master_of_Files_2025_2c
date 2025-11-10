#ifndef COMMUNICATION_OPS_TRUNCATE_FILE_H
#define COMMUNICATION_OPS_TRUNCATE_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef CONTROL_ACCESOS_H
#include "../control_accesos.h"
#endif

void truncate_file_ops(char* file, char* tag, int nuevo_tam, worker* w){
    //Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }
        
    if(!file_tag_exist_or_not(file, tag, w)){
        return; //Ya se envió el error al worker
    }

}

#endif
  /*Esta operación se encargará de modificar el tamaño del File:Tag especificados agrandando o achicando el tamaño del mismo 
    para reflejar el nuevo tamaño deseado (actualizando la metadata necesaria).
    Al incrementar el tamaño del File, se le asignarán tantos bloques lógicos (hard links) como sea necesario. 
    Inicialmente, todos ellos deberán apuntar el bloque físico nro 0.
    Al reducir el tamaño del File, se deberán desasignar tantos bloques lógicos como sea necesario (empezando por el final del archivo). 
    Si el bloque físico al que apunta el bloque lógico eliminado no es referenciado por ningún otro File:Tag, 
    deberá ser marcado como libre en el bitmap.*/
    // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

    /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
        File_inexistente
        Tag_inexistente
        Espacio_Insuficiente

    */

    /* Tema importante:
        - Cuando hay que reducir la cantidad de bloques fisicos (porque hay que achicar el archivo), 
            hay que fijarse si el bloque fisico al que apunta el bloque logico eliminado no es referenciado por ningun otro File:Tag,
            y en ese caso, liberarlo en el bitmap.
    */


    // parseo args
    /*char* file = args[0];
    char* tag  = args[1];
    off_t nuevo_tam = atoll(args[2]); //LONG INT??? ENFERMO*/

    // paths


    //OLD CODE - Se comenta el codigo por errores en la forma que se gestionan los bloques fisicos y logicos.

    /*
        char* file_path     = string_from_format("%s/files/%s", cs.punto_montaje, file);
    char* tag_path      = string_from_format("%s/%s", file_path, tag);
    char* metadata_path = NULL;

    t_config* metadata = NULL;
    t_list*   bloques  = NULL;
    pthread_mutex_t* tag_lock = NULL;
    bool tag_locked = false;

    // valido existencia
    if (!control_existencia_file(file_path)) {
        log_error(logger, "[TRUNCATE] File inexistente: %s", file);
    } else if (!control_existencia_file(tag_path)) {
        log_error(logger, "[TRUNCATE] Tag inexistente: %s:%s", file, tag);
    } else {
        // lock del tag
        tag_lock = get_file_tag_lock(file, tag);
        pthread_mutex_lock(tag_lock);
        tag_locked = true;

        log_info(logger, "[TRUNCATE] %s:%s -> nuevo tamaño %lld", file, tag, (long long)nuevo_tam);

        // abro metadata
        metadata_path = string_from_format("%s/metadata.config", tag_path);
        metadata = config_create(metadata_path);
        
        if (!metadata) {
            log_error(logger, "[TRUNCATE] No se pudo abrir metadata: %s", metadata_path);
        } else {
            // verifico estado COMMITED
            char* estado = config_get_string_value(metadata, "ESTADO");
            if (estado && string_equals_ignore_case(estado, "COMMITED")) {
                log_error(logger, "[TRUNCATE] Tag COMMITED, no se puede modificar");
            } else {
                // leo BLOCKS a lista
                bloques = list_create();
                const char* KEY_BLOCKS_RD =
                    config_has_property(metadata, "BLOCKS")  ? "BLOCKS"  :
                    (config_has_property(metadata, "BLOQUES") ? "BLOQUES" : "BLOCKS");
                char** arr = config_get_array_value(metadata, KEY_BLOCKS_RD);
                if (arr){
                    for (int i = 0; arr[i] != NULL; i++){
                        int* p = malloc(sizeof(int));
                        *p = atoi(arr[i]);
                        list_add(bloques, p);
                        free(arr[i]);
                    }
                    free(arr);
                }

                // controlo bloques
                int bloques_actuales = list_size(bloques);
                if (g_block_size > 0) {
                    int bloques_necesarios = (nuevo_tam == 0) ? 0 : (int)ceil((double)nuevo_tam / (double)g_block_size);
                    bool operation_success = true;

                    // crecer: linkea lógicos nuevos a físico 0
                    if (bloques_necesarios > bloques_actuales) {
                        int desde = bloques_actuales;
                        int hasta = bloques_necesarios - 1;

                        char* physical0 = string_from_format("%s/physical_blocks/block%04d.dat", cs.punto_montaje, 0);
                        pthread_mutex_lock(&block_locks[0]);

                        for (int i = desde; i <= hasta && operation_success; ++i) {
                            char* logical = string_from_format("%s/logical_blocks/block%04d.dat", tag_path, i);
                            crear_hard_link(physical0, logical);
                            int* p = malloc(sizeof(int)); *p = 0;
                            list_add(bloques, p);
                            log_debug(logger, "[TRUNCATE] lógico #%d -> físico 0", i);
                            free(logical);
                        }

                        pthread_mutex_unlock(&block_locks[0]);
                        free(physical0);
                    }
                    // achicar: unlink y liberar bitmap si queda sin enlaces
                    else if (bloques_necesarios < bloques_actuales) {
                        for (int i = bloques_actuales - 1; i >= bloques_necesarios && operation_success; --i) {
                            int* pfis = (int*) list_get(bloques, i);
                            if (!pfis) {
                                log_error(logger, "[TRUNCATE] Metadata inconsistente (bloque %d sin físico)", i);
                                operation_success = false;
                                break;
                            }
                            int bloque_fisico = *pfis;

                            char* logical  = string_from_format("%s/logical_blocks/block%04d.dat", tag_path, i);
                            char* physical = string_from_format("%s/physical_blocks/block%04d.dat", cs.punto_montaje, bloque_fisico);

                            // lock del físico
                            pthread_mutex_lock(&block_locks[bloque_fisico]);

                            // unlink del lógico
                            if (unlink(logical) != 0) {
                                log_error(logger, "[TRUNCATE] unlink falló: %s (errno=%d)", logical, errno);
                                pthread_mutex_unlock(&block_locks[bloque_fisico]);
                                free(logical); free(physical);
                                operation_success = false;
                                break;
                            }

                            // consulto enlaces del físico
                            struct stat st;
                            if (stat(physical, &st) == -1) {
                                log_error(logger, "[TRUNCATE] stat físico falló: %s (errno=%d)", physical, errno);
                                pthread_mutex_unlock(&block_locks[bloque_fisico]);
                                free(logical); free(physical);
                                operation_success = false;
                                break;
                            }

                            // libero bitmap si queda solo el archivo físico
                            if (st.st_nlink == 1) {
                                liberar_bloque(bloque_fisico, g_block_size, g_bitmap_size);
                                log_debug(logger, "[TRUNCATE] físico %d liberado (sin referencias lógicas)", bloque_fisico);
                            }

                            // unlock del físico
                            pthread_mutex_unlock(&block_locks[bloque_fisico]);

                            // actualizo lista de bloques
                            int* removed = (int*) list_remove(bloques, i);
                            if (removed) free(removed);

                            // libero paths
                            free(logical);
                            free(physical);
                        }
                    }

                    // persistir metadata solo si la operación fue exitosa
                    if (operation_success) {
                        const char* KEY_BLOCKS_WR =
                            config_has_property(metadata, "BLOCKS")  ? "BLOCKS"  :
                            (config_has_property(metadata, "BLOQUES") ? "BLOQUES" : "BLOCKS");
                        const char* KEY_SIZE_WR =
                            config_has_property(metadata, "SIZE")    ? "SIZE"    :
                            (config_has_property(metadata, "TAMANIO") ? "TAMANIO" : "SIZE");

                        // serializo bloques a "[a,b,c]"
                        char* blocks_str = string_new();
                        string_append(&blocks_str, "[");
                        int nblocks = list_size(bloques);
                        for (int i = 0; i < nblocks; ++i){
                            int* pf = (int*) list_get(bloques, i);
                            char* num = string_from_format("%d", pf ? *pf : 0);
                            string_append(&blocks_str, num);
                            free(num);
                            if (i < nblocks - 1) string_append(&blocks_str, ",");
                        }
                        string_append(&blocks_str, "]");

                        char* size_str = string_from_format("%lld", (long long)nuevo_tam);

                        // escribo y guardo metadata
                        config_set_value(metadata, KEY_BLOCKS_WR, blocks_str);
                        config_set_value(metadata, KEY_SIZE_WR,   size_str);
                        config_save(metadata);

                        // libero buffers
                        free(blocks_str);
                        free(size_str);
                    }
                } else {
                    log_error(logger, "[TRUNCATE] g_block_size inválido");
                }
            }
        }
    }

    // cleanup - liberar recursos en orden inverso a la asignación
    if (bloques) {
        list_destroy_and_destroy_elements(bloques, free);
    }
    if (metadata) {
        config_destroy(metadata);
    }
    if (tag_locked && tag_lock) {
        pthread_mutex_unlock(tag_lock);
    }
    if (metadata_path) {
        free(metadata_path);
    }
    if (tag_path) {
        free(tag_path);
    }
    if (file_path) {
        free(file_path);
    }

    log_info(logger, "## %d - File Truncado %s:%s Tamaño %d", w->id_query, file, tag, nuevo_tam);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
    */