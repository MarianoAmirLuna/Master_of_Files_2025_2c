#ifndef COMMUNICATION_OPS_WRITE_BLOCK_H
#define COMMUNICATION_OPS_WRITE_BLOCK_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif


void write_block_ops(char* file, char* tag, int bloque_logico, char* contenido, worker* w){
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
    /*char* file = args[0];
    char* tag  = args[1];
    int bloque_logico = atoi(args[2]);
    char* contenido   = args[3];*/
        
    if(!file_tag_exist_or_not(file, tag, w)){
        return; //Ya se envió el error al worker
    }

    //TODO: CHECK ESPACIO INSUFICIENTE
    //TODO: CHECK OVERFLOW WRITE OR READ
    
    t_config* metadata = get_metadata_from_file_tag(cs, file, tag);
    if(metadata == NULL){
        log_error(logger, "No se pudo obtener la metadata del File:Tag %s:%s", file, tag);
        return;
    }
    if(get_state_metadata(metadata) == COMMITED){
        send_basic_packet(w->fd, WRITE_NO_PERMISSION);
        log_error(logger, "El tag se encuentra en estado COMMITED, no se puede escribir");
        config_destroy(metadata);
        return;
    }
    config_destroy(metadata);
    // lock del file tag (bloqueo logico para que no toquen el mismo tag al mismo tiempo)
    pthread_mutex_t* tag_lock = get_file_tag_lock(file, tag);
    pthread_mutex_lock(tag_lock);

    log_orange(logger, "estoy bloqueando al otro bobo :) ESTAS DURMIENDO 30 SEGUNDOS");
    sleep(30);
    //DANGER: OJO ACA ESTAS DURMIENDO 30 SEGUNDOS

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
    log_info(logger, "## %d - Bloque Lógico Escrito %s:%s Número de Bloque %d", w->id_query, file, tag, bloque_logico);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}


#endif