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
    //La única comprobación que importa en CREATE es la de si el TAG existe, no el FILE.
    if(!file_tag_exist_or_not(file, tag, w)){
        return;
    }
    
    char* logical_blocks_dir = get_logical_blocks_dir(cs, file, tag);
    //Crea todos los directorios, significa que crea el files, el files/variable file, el files/variable file/variable tag y el files/variable file/variable tag/ con logical_blocks
    create_nested_directories(logical_blocks_dir);
    char* metadata = get_metadata_fullpath(cs, file,tag);
    t_config* conf = crear_metadata_config(metadata, g_block_size, NULL, WORK_IN_PROGRESS);
    
    free(metadata);
    free(logical_blocks_dir);
    config_destroy(conf);

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