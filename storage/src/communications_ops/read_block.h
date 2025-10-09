#ifndef COMMUNICATION_OPS_READ_BLOCK_H
#define COMMUNICATION_OPS_READ_BLOCK_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif

void read_block_ops(char* file, char* tag, int numero_bloque, worker* w){
    log_error(logger, "%s NOT IMPLEMENTED (%s:%d)",__func__, __func__,__LINE__);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif