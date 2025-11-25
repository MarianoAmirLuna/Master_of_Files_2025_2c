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
        Lectura fuera de limite faltaria
    */
    
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
    // Aplicar retardo de acceso a bloque
    msleep(cs.retardo_acceso_bloque);

    t_packet* p = create_packet();
    add_int_to_packet(p, READ_BLOCK);
    add_string_to_packet(p, buffer);
    send_and_free_packet(p, w->fd);

    fclose(f);
    free(block_path);
    free(buffer);

    // Log obligatorio de bloque lógico leído
    log_info(logger, "## %d - Bloque Lógico Leído %s:%s - Número de Bloque: %d", w->id_query, file, tag, numero_bloque);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif