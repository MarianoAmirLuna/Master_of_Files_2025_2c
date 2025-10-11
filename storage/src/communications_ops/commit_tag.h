#ifndef COMMUNICATION_OPS_COMMIT_TAG_H
#define COMMUNICATION_OPS_COMMIT_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif


void commit_tag_ops(char* file, char* tag, worker* w){

    //Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }

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