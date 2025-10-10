#ifndef COMMUNICATION_OPS_DELETE_TAG_H
#define COMMUNICATION_OPS_DELETE_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

void delete_tag_ops(char* file, char* tag, worker* w){
    /*Esta operación eliminará el directorio correspondiente al File:Tag indicado. 
    Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado 
    no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.*/
    // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

    /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
        File_inexistente
        Tag_inexistente
    */

    log_error(logger, "%s NOT IMPLEMENTED (%s:%d)",__func__, __func__,__LINE__);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif