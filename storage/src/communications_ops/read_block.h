#ifndef COMMUNICATION_OPS_READ_BLOCK_H
#define COMMUNICATION_OPS_READ_BLOCK_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

#ifndef EXTS_FILE_EXT_H
#include "exts/file_ext.h"
#endif

void read_block_ops(char* file, char* tag, int numero_bloque, worker* w){
    /*Dado un File:Tag y número de bloque lógico, la operación de lectura obtendrá y devolverá el contenido del mismo.*/
    
    /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
        File_inexistente
        Tag_inexistente
        Lectura_o_escritura_fuera_de_limite
    */
    
    if(!file_tag_exist_or_not(file, tag, w)){
        return; //Ya se envió el error al worker
    }
    
    char* block_name = get_block_name_logical(numero_bloque);
    char* logical_blocks_dir = get_logical_blocks_dir(cs, file, tag);
    char* block_path = string_from_format("%s/%s",logical_blocks_dir, block_name);
    free(logical_blocks_dir);
    free(block_name);
    
    if(!control_existencia_file(block_path)){
        log_error(logger, "El bloque lógico %d del File:Tag %s:%s no existe (%s:%d) BLOCKPATH=%s", numero_bloque, file, tag, __func__, __LINE__, block_path);
        free(block_path);
        return;
    }
    FILE* f =fopen(block_path, "r");
    if(f == NULL){
        log_error(logger, "No se pudo abrir el bloque lógico %d del File:Tag %s:%s (%s:%d) BLOCKPATH=%s", numero_bloque, file, tag, __func__, __LINE__, block_path);
        free(block_path);
        return;
    }
    int sz = get_size_file(f);
    char* buffer = malloc(sz+1);
    fgets(buffer, sz, f);
    t_packet* p = create_packet();
    add_int_to_packet(p, READ_BLOCK);
    add_string_to_packet(p, buffer);
    send_and_free_packet(p, w->fd);
    free(block_path);
    free(buffer);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif