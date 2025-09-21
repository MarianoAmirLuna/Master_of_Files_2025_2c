#ifndef COMUNICATION_WORKER_H
#define COMUNICATION_WORKER_H

#include "base.h"




void tratar_mensaje(t_list* pack, int sock_client)
{
    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }

    int opcode = list_get_int(pack, 0);

    char* params = NULL;
    if(list_size(pack) > 1)
    {
        params = (char*) list_get(pack,1);
    }

    char** args = NULL;
    if(params != NULL)
    {
        args = string_split(params, " ");
    }
    
    //Descomentar esto sólo si desde worker usan el ejecutar_instruccion_v2
    //opcode = convert_instr_code_to_storage_operation(opcode);

    switch (opcode)
    {
    case CREATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Esta operación creará un nuevo File dentro del FS. Para ello recibirá el nombre del File y un Tag inicial para crearlo.
            Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.*/
            char* path = string_from_format("%s/files", cs.punto_montaje);
            log_info(logger, "Ejecutando la operacion CREATE_FILE");
            crear_directorio(args[0], path); // creo el archivo (no verifico si existe)

            // creo el tag
            path = string_from_format("%s/%s", path, args[0]); // uso el path porque es dentro del archivo
            crear_directorio(args[1], path); // creo el tag

            // espacio de bloques logicos
            path = string_from_format("%s/%s", path, args[1]);
            crear_directorio("logical_blocks", path);

            // metadata
            path = string_from_format("%s/metadata.config", path);
            crear_metadata_config(path, g_block_size, list_create(), WORK_IN_PROGRESS);
            free(path);
        }
        break;

    case TRUNCATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
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

            // parseo args
            char* file = args[0];
            char* tag  = args[1];
            off_t nuevo_tam = atoll(args[2]);

            // paths
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
                                        liberar_bloque(bloque_fisico);
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
        }
    break;

    case TAG_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            /*Esta operación creará una copia completa del directorio nativo correspondiente al Tag de origen en un 
            nuevo directorio correspondiente al Tag destino y modificará en el archivo de metadata del Tag destino 
            para que el mismo se encuentre en estado WORK_IN_PROGRESS.*/

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */ //son los mismos hard links no nuevos bloques, por lo que el espacio no seria un problema 
            log_info(logger, "Ejecutando la operacion TAG_FILE");
        }
        break;

    case COMMIT_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Confirmará un File:Tag pasado por parámetro. En caso de que un Tag ya se encuentre confirmado, 
            esta operación no realizará nada. Para esto se deberá actualizar el archivo metadata del Tag pasando su estado a “COMMITED”.
            Se deberá, por cada bloque lógico, buscar si existe algún bloque físico que tenga el mismo contenido 
            (utilizando el hash y archivo blocks_hash_index.config). 
            En caso de encontrar uno, se deberá liberar el bloque físico actual y 
            reapuntar el bloque lógico al bloque físico pre-existente. En caso contrario, 
            simplemente se agregará el hash del nuevo contenido al archivo blocks_hash_index.config.*/

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */ // cuidado de no borrar hard links incorrectos
            log_info(logger, "Ejecutando la operacion COMMIT_TAG");
        }
        break;

    case WRITE_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            /*Esta operación recibirá el contenido de un bloque lógico de un File:Tag y guardará los cambios en el 
            bloque físico correspondiente, siempre y cuando el File:Tag no se encuentre en estado COMMITED y 
            el bloque lógico se encuentre asignado.
            Si el bloque lógico a escribir fuera el único referenciando a su bloque físico asignado, 
            se escribirá dicho bloque físico directamente. En caso contrario, se deberá buscar un nuevo bloque físico, 
            escribir en el mismo y asignarlo al bloque lógico en cuestión.*/

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
                Espacio_Insuficiente
                Escritura_no_permitida
                Lectura_o_escritura_fuera_de_limite
            */
            char* file = args[0];
            char* tag  = args[1];
            int bloque_logico = atoi(args[2]);
            char* contenido   = args[3];

            // valido existencia file
            char* path = string_from_format("%s/%s", cs.punto_montaje, file);
            if (control_existencia_file(path))
            {
                log_error(logger, "No se encontro el file deseado");
            }

            // valido existencia tag
            path = string_from_format("%s/%s/", cs.punto_montaje, file, tag);
            if (control_existencia_file(path))
            {
                log_error(logger, "No se encontro el tag deseado");
            }
            // lock del file tag (bloqueo logico para que no toquen el mismo tag al mismo tiempo)
            pthread_mutex_t* tag_lock = get_file_tag_lock(file, tag);
            pthread_mutex_lock(tag_lock);

                log_orange(logger, "estoy bloqueando al otro bobo :)");
                sleep(30);

/*
                int bloque_fisico = obtener_bloque_fisico(file, tag, bloque_logico); // todo: declarar funcion obtener_bloque_fisico

                // lock del bloque fisico
                pthread_mutex_lock(&block_locks[bloque_fisico]);
                log_debug(logger, "[WRITE_BLOCK] Lock Bloque Físico %d", bloque_fisico);

                escribir_bloque_fisico(bloque_fisico, contenido); // todo: escribir bloque fisico


                pthread_mutex_unlock(&block_locks[bloque_fisico]);
                log_debug(logger, "[WRITE_BLOCK] Unlock Bloque Físico %d", bloque_fisico);
*/



            pthread_mutex_unlock(tag_lock);
            log_info(logger, "Ejecutando la operacion WRITE_BLOCK");
        }
        break;

    case READ_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            /*Dado un File:Tag y número de bloque lógico, la operación de lectura obtendrá y devolverá el contenido del mismo.*/
            
            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
                Lectura_o_escritura_fuera_de_limite
            */
            log_info(logger, "Ejecutando la operacion READ_BLOCK");
        }
        break;

    case DELETE_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Esta operación eliminará el directorio correspondiente al File:Tag indicado. 
            Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado 
            no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.*/
            // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */
            log_info(logger, "Ejecutando la operacion DELETE_TAG");
        }
        break;

    default:
        log_error(logger, "CODIGO DE OPERACION INVALIDO CODIGO: %d", opcode);
        break;
    }

    // esto es una respuesta barata, despues le agrego a cada uno su respuesta personalizada
    t_packet* response = create_packet();
    int result_code = 999; // ponele que es un ok por ahora
    add_int_to_packet(response, result_code);
    send_and_free_packet(response, sock_client);
            
    if (args != NULL)
    {
        string_iterate_lines(args, (void*) free);
        free(args);
    }

}


/* posibles errores que hay que manejar
    File inexistente
        Una operación quiere realizar una acción sobre un File:Tag que no existe (salvo la operación de CREATE que crea un nuevo File:Tag).
    Tag inexistente
        Una operación quiere realizar una acción sobre un tag que no existe, salvo la operación de TAG que crea un nuevo Tag.
    Espacio Insuficiente
        Al intentar asignar un nuevo bloque físico, no se encuentra ninguno disponible.
    Escritura no permitida
        Una query intenta escribir o truncar un File:Tag que se encuentre en estado COMMITED.
    Lectura o escritura fuera de limite
        Una query intenta leer o escribir por fuera del tamaño del File:Tag.
*/

#endif