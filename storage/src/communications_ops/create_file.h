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

    // creo el tag
    path = string_from_format("%s/%s", path, file); // uso el path porque es dentro del archivo
    crear_directorio(tag, path); // creo el tag

    // espacio de bloques logicos
    path = string_from_format("%s/%s", path, tag);
    crear_directorio("logical_blocks", path);

    // metadata
    path = string_from_format("%s/metadata.config", path);
    crear_metadata_config(path, g_block_size, list_create(), WORK_IN_PROGRESS);
    free(path);

    log_info(logger, "## %d - File Creado %s:%s", w->id_query, file, tag);

}

#endif