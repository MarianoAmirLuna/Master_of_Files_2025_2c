#ifndef COMUNICATION_WORKER_H
#define COMUNICATION_WORKER_H

#ifndef STORAGE_BASE_H
#include "base.h"
#endif 

//NOTE: Added worker* dentro de esa estructura está tnato el id como el id_query, creería que hay que tener en cuenta qué id_worker envía el mensaje
//NOTESE QUE EXISTE t_list* workers 
void tratar_mensaje(t_list* pack, worker* w, int sock_client)
{
    msleep(cs.retardo_operacion); //Según el TP dice que TODAS LAS OPERACIONES se deberá esperar un tiempo de retardo, claro pa, pero esto va arriba

    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }

    int opcode = list_get_int(pack, 0);
    log_pink(logger, "OPCODE RECIBIDO EN STORAGE: %s", get_opcode_as_string(opcode));
    int real_sz = list_size(pack)-1;
   
    /*char* params = NULL;
    if(list_size(pack) > 1)
    {
        params = list_get_str(pack ,1);
    }
    
    log_debug(logger, "OPCODE ES: %d, PARAMS: %s", opcode, params);
    if(opcode == CREATE_FILE){
        log_debug(logger, "Segudno argumento es: %s", list_get_str(pack, 2));
    }
    char** args = NULL;
    if(params != NULL)
    {
        args = string_split(params, " ");
    }*/
    
    //Descomentar esto sólo si desde worker usan el ejecutar_instruccion_v2
    //opcode = convert_instr_code_to_storage_operation(opcode);
    char* file = list_get_str(pack,1);
    char* tag = list_get_str(pack, 2);
    /*if(opcode == GET_BLOCK_DATA){ // devuelve el tamaño del bloque porque ? no se, pero lo hace atte: lseijas
        t_packet* pdata = create_packet();
        add_int_to_packet(pdata, RETURN_BLOCK_DATA);
        //NO CONFUNDAS CON EL PUTO TAMAÑO DEL BLOQUE PORQUE ESO LO OBTIENE 
        //EL WORKER LA PRIMERA VZE QUE SE CONECTA CON STORAGE
        // SORETE
        //add_int_to_packet(pdata, g_block_size);
        send_and_free_packet(pdata, sock_client);
        return;
    }*/
    if(opcode == CREATE_FILE){
        if(real_sz < 2){
            log_error(logger, "Cantidad inválida de argumentos");
        }
        create_file_ops(file, tag, w);
    } 
    if(opcode == TRUNCATE_FILE){
        if(real_sz < 3){
            log_error(logger, "Cantidad inválida de argumentos");
        }
        int sz = list_get_int(pack, 3);
        truncate_file_ops(file,tag,sz, w);
    }
    if(opcode == TAG_FILE){
        char* file_destino = list_get_str(pack, 3);
        char* tag_destino = list_get_str(pack, 4);

        //Tiene 3 argumentos el TAG_FILE??? Con el args[3]??? Investigar.
        tag_file_ops(file, tag, file_destino, tag_destino,w);
        free(tag_destino);
        free(file_destino);
    }
    if(opcode == COMMIT_TAG){
        commit_tag_ops(file, tag, w);
    }
    if(opcode == WRITE_BLOCK || opcode == WRITE_BLOCK_NOT_ERROR){
        int sz = list_get_int(pack, 3);
        char* contenido = list_get_str(pack ,4);
        write_block_ops(file, tag, sz, contenido, w, opcode == WRITE_BLOCK_NOT_ERROR);
        free(contenido);
    }
    if(opcode == READ_BLOCK){
        int sz = list_get_int(pack, 3);
        read_block_ops(file, tag, sz, w);
    }
    if(opcode == DELETE_TAG){
        delete_tag_ops(file, tag, w);
    }
    free(file);
    free(tag);
    // esto es una respuesta barata, despues le agrego a cada uno su respuesta personalizada
    /*t_packet* response = create_packet();
    add_int_to_packet(response, SUCCESS);
    send_and_free_packet(response, w->fd);*/
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


