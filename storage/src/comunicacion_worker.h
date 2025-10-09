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

    char* params = NULL;
    if(list_size(pack) > 1)
    {
        params = (char*) list_get(pack,1);
    }

    char** args = NULL;
    if(params != NULL)
    {
        args = string_split(params, " ");
    }
    
    //Descomentar esto sólo si desde worker usan el ejecutar_instruccion_v2
    //opcode = convert_instr_code_to_storage_operation(opcode);
    if(opcode == CREATE_FILE){
        create_file_ops(args[0], args[1], w);
    }
    if(opcode == TRUNCATE_FILE){
        truncate_file_ops(args[0],args[1], atoi(args[2]), w);
    }
    if(opcode == TAG_FILE){
        //Tiene 3 argumentos el TAG_FILE??? Con el args[3]??? Investigar.
        tag_file_ops(args[0], args[1], w);
    }
    if(opcode == COMMIT_TAG){
        commit_tag_ops(args[0], args[1], w);
    }
    if(opcode == WRITE_BLOCK){
        write_block_ops(args[0], args[1], atoi(args[2]), args[3], w);
    }
    if(opcode == READ_BLOCK){
        read_block_ops(args[0], args[1], atoi(args[2]), w);
    }
    if(opcode == DELETE_TAG){
        delete_tag_ops(args[0], args[1], w);
    }
    // esto es una respuesta barata, despues le agrego a cada uno su respuesta personalizada
    t_packet* response = create_packet();
    int result_code = 999; // ponele que es un ok por ahora //Que tarado.
    add_int_to_packet(response, result_code);
    send_and_free_packet(response, sock_client);
            
    if (args != NULL)
    {
        string_iterate_lines(args, (void*) free);
        free(args);
    }

    msleep(cs.retardo_operacion); //Según el TP dice que TODAS LAS OPERACIONES se deberá esperar un tiempo de retardo

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