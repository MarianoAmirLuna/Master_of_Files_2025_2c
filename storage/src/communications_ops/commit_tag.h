#ifndef COMMUNICATION_OPS_COMMIT_TAG_H
#define COMMUNICATION_OPS_COMMIT_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif
#ifndef CRYPTO_H_
#include "commons/crypto.h"
#endif
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
        return;
    }
    
    //Según la condiciión si no está comiteado debo especificarlo como comiteado
    set_state_metadata_from_config(metadata, COMMITED);

    t_config* bhi = get_block_hash_index(cs);
    char* logical_dir = get_logical_blocks_dir(cs, file, tag);
    t_list* blocks_list = get_files_from_dir(logical_dir); 
    
    //Segun entendí, debe iterar todos los bloques lógico dentro del logical_blocks 
    //y comprobar si el hash de cada bloque lógico existe algún bloque físico que tenga el mismo hash
    //y si lo tiene debe liberar el bloque físico actual y reapuntar el bloque lógico al bloque físico pre-existente.
    for(int i=0;i<list_size(blocks_list);i++){
        char* block_f = (char*)list_get(blocks_list, i);
        char* block_hash = crypto_md5(block_f, strlen(block_f));
        if(exists_hash_in_block_hash(bhi, block_hash)){
            //Debe hacer alguna mierda rara.
            //liberar bloque fisico actual
        }
    }

    list_destroy_and_destroy_elements(blocks_list, free);
    
    config_destroy(metadata);
    config_destroy(bhi);
    
    /*
        1. Primero verifico si el archivo ya estaba comitieado o no.
        2. Si no esta comiteado, cambio el estado a commited, al hacer esto antes de ponerme analizar los bloques fisicos evito
        de condicion de carrera.
        3. Genero los hash de cada bloque logico trabajado, y busco si hay otro bloque fisico con el mismo hash.
            a. Caso que se encuentre, directamente vinculo el bloque logico con el fisico correspondiente.
            b. En el caso de que no encuentre ninguno, busco un bloque fisico libre y copio la data, para luego linkquear el bloque logico con el bloque fisico.
        4. Avisar al worker que se comiteo correctamente el archivo.
     ---------------------------------------------------------------------------------------------------
     Detalles:
     1. Segun la documetacion, la funcion no debe hacer nada cuando un worker le envia el commit y el archivo esta comiteado.
     Debo avisar al worker que el archivo ya estaba comiteado? O directamente hago como si se completara el commit normalmente?
     2. Como impacta el commit de un archivo cuando se solicita una copia nueva del mismo? - Resolucion temporal:
      Se copian los datos los bloques logicos
     y luego cuando se cierre la copia ahi se calculan los bloques fisicos.
    */
    log_error(logger, "%s NOT IMPLEMENTED (%s:%d)",__func__, __func__,__LINE__);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}


#endif