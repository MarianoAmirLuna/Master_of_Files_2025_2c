#ifndef COMUNICATION_WORKER_H
#define COMUNICATION_WORKER_H

#ifndef STORAGE_BASE_H
#include "base.h"
#endif 

//NOTE: Added worker* dentro de esa estructura está tnato el id como el id_query, creería que hay que tener en cuenta qué id_worker envía el mensaje
//NOTESE QUE EXISTE t_list* workers 
void tratar_mensaje(t_list* pack, worker* w, int sock_client)
{

    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }

    int opcode = list_get_int(pack, 0);
    log_pink(logger, "OPCODE RECIBIDO EN STORAGE: %s", get_opcode_as_string(opcode));
    int real_sz = list_size(pack)-1;
   
   
    char* file = list_get_str(pack,1);
    char* tag = list_get_str(pack, 2);
   
    int YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA=0;
    if(opcode == CREATE_FILE){
        if(real_sz < 2){
            log_error(logger, "Cantidad inválida de argumentos");
        }
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA= create_file_ops(file, tag, w);
    } 
    if(opcode == TRUNCATE_FILE){
        if(real_sz < 3){
            log_error(logger, "Cantidad inválida de argumentos");
        }
        int sz = list_get_int(pack, 3);
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA = truncate_file_ops(file,tag,sz, w);
    }
    if(opcode == TAG_FILE){
        char* file_destino = list_get_str(pack, 3);
        char* tag_destino = list_get_str(pack, 4);

        //Tiene 3 argumentos el TAG_FILE??? Con el args[3]??? Investigar.
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA= tag_file_ops(file, tag, file_destino, tag_destino,w);
        free(tag_destino);
        free(file_destino);
    }
    if(opcode == COMMIT_TAG){
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA= commit_tag_ops(file, tag, w);
    }
    if(opcode == WRITE_BLOCK || opcode == WRITE_BLOCK_NOT_ERROR){
        int sz = list_get_int(pack, 3);
        char* contenido = list_get_str(pack ,4);
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA = write_block_ops(file, tag, sz, contenido, w, opcode != WRITE_BLOCK_NOT_ERROR);
        free(contenido);
    }
    if(opcode == READ_BLOCK){
        int sz = list_get_int(pack, 3);
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA = read_block_ops(file, tag, sz, w);
    }
    if(opcode == DELETE_TAG){
        YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA = delete_tag_ops(file, tag, w);
    }
    free(file);
    free(tag);
    
    if(YA_ENVIO_LA_MIERDA_DE_ERROR_O_RESPUESTA) // YA ENVIO Y NO VOY A HACER UNA REVERENDA CHOTA
        return; 
    
    t_packet* response = create_packet();
    add_int_to_packet(response, TUVE_UNA_RESPUESTA_DEL_PUTO_STORAGE); //LE VOY A RESPONDER AL HIJO DE RE MIL PUTA
    send_and_free_packet(response, w->fd);
}


/* posibles errores que hay que manejar
    File inexistente
        Una operación quiere realizar una acción sobre un File:Tag que no existe (salvo la operación de CREATE que crea un nuevo File:Tag).
    Tag inexistente
        Una operación quiere realizar una acción sobre un tag que no existe, salvo la operación de TAG que crea un nuevo Tag.
    Espacio Insuficiente
        Al intentar asignar un nuevo bloque físico, no se encuentra ninguno disponible.
    Escritura no permitida
        Una query intenta escribir o truncar un File:Tag que se encuentre en estado COMMITED.
    Lectura o escritura fuera de limite
        Una query intenta leer o escribir por fuera del tamaño del File:Tag.
*/

#endif


