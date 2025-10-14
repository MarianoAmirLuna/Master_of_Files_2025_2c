#ifndef COMMUNICATION_OPS_CREATE_FILE_H
#define COMMUNICATION_OPS_CREATE_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

void create_file_ops(char* file, char* tag, worker* w){
    if(file == NULL || tag == NULL){
        log_error(logger, "Hubo un problema file o tag son nulos en (%s:%d)", __func__,__LINE__);
        log_error(logger, "FILE es NULL? %d TAG IS NULL? %d", file == NULL, tag == NULL );
        return;
    }
    /*Esta operación creará un nuevo File dentro del FS. Para ello recibirá el nombre del File y un Tag inicial para crearlo.
    Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.*/
    char* path = string_from_format("%s/files", cs.punto_montaje);
    log_info(logger, "Ejecutando la operacion CREATE_FILE");
    crear_directorio(file, path); // creo el archivo (no verifico si existe)
    free(path); //El string_from_format va a crear nueva dirección del puntero, por eso es aconsejable liberarlo antes de asignar otro
    
    // creo el tag
    path = string_from_format("%s/%s", path, file); // uso el path porque es dentro del archivo
    crear_directorio(tag, path); // creo el tag
    free(path); //El string_from_format va a crear nueva dirección del puntero, por eso es aconsejable liberarlo antes de asignar otro
    
    // espacio de bloques logicos
    path = string_from_format("%s/%s", path, tag);
    crear_directorio("logical_blocks", path);
    free(path); //El string_from_format va a crear nueva dirección del puntero, por eso es aconsejable liberarlo antes de asignar otro
    
    // metadata
    path = string_from_format("%s/metadata.config", path);
    crear_metadata_config(path, g_block_size, NULL, WORK_IN_PROGRESS);
    free(path);

    log_orange(logger, "FILE: %s, TAG: %s", file, tag);
    if(w == NULL){
        log_error(logger, "EL WORKER ES NULO WTF");
    }
    else{
        log_debug(logger, "El qid es: %d", w->id_query);
    }
    log_info(logger, "## %d - File Creado %s:%s", w->id_query, file, tag);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif