#ifndef COMUNICATION_WORKER_H
#define COMUNICATION_WORKER_H

#include "base.h"




void tratar_mensaje(t_list* pack, int sock_client)
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

    switch (opcode)
    {
    case CREATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Esta operación creará un nuevo File dentro del FS. Para ello recibirá el nombre del File y un Tag inicial para crearlo.
            Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.*/
            char* path = string_from_format("%s/files", cs.punto_montaje);
            log_info(logger, "Ejecutando la operacion CREATE_FILE");
            crear_directorio(args[0], path); // creo el archivo (no verifico si existe)

            // creo el tag
            path = string_from_format("%s/%s", path, args[0]); // uso el path porque es dentro del archivo
            crear_directorio(args[1], path); // creo el tag

            // espacio de bloques logicos
            path = string_from_format("%s/%s", path, args[1]);
            crear_directorio("logical_blocks", path);

            // metadata
            path = string_from_format("%s/metadata.config", path);
            crear_metadata_config(path, g_block_size, list_create(), WORK_IN_PROGRESS);
            free(path);
        }
        break;

    case TRUNCATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            /*Esta operación se encargará de modificar el tamaño del File:Tag especificados agrandando o achicando el tamaño del mismo 
            para reflejar el nuevo tamaño deseado (actualizando la metadata necesaria).
            Al incrementar el tamaño del File, se le asignarán tantos bloques lógicos (hard links) como sea necesario. 
            Inicialmente, todos ellos deberán apuntar el bloque físico nro 0.
            Al reducir el tamaño del File, se deberán desasignar tantos bloques lógicos como sea necesario (empezando por el final del archivo). 
            Si el bloque físico al que apunta el bloque lógico eliminado no es referenciado por ningún otro File:Tag, 
            deberá ser marcado como libre en el bitmap.*/
            // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
                Espacio_Insuficiente
            */
            log_info(logger, "Ejecutando la operacion TRUNCATE_FILE");

        }
        break;

    case TAG_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            /*Esta operación creará una copia completa del directorio nativo correspondiente al Tag de origen en un 
            nuevo directorio correspondiente al Tag destino y modificará en el archivo de metadata del Tag destino 
            para que el mismo se encuentre en estado WORK_IN_PROGRESS.*/

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */ //son los mismos hard links no nuevos bloques, por lo que el espacio no seria un problema 
            log_info(logger, "Ejecutando la operacion TAG_FILE");
        }
        break;

    case COMMIT_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Confirmará un File:Tag pasado por parámetro. En caso de que un Tag ya se encuentre confirmado, 
            esta operación no realizará nada. Para esto se deberá actualizar el archivo metadata del Tag pasando su estado a “COMMITED”.
            Se deberá, por cada bloque lógico, buscar si existe algún bloque físico que tenga el mismo contenido 
            (utilizando el hash y archivo blocks_hash_index.config). 
            En caso de encontrar uno, se deberá liberar el bloque físico actual y 
            reapuntar el bloque lógico al bloque físico pre-existente. En caso contrario, 
            simplemente se agregará el hash del nuevo contenido al archivo blocks_hash_index.config.*/

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */ // cuidado de no borrar hard links incorrectos
            log_info(logger, "Ejecutando la operacion COMMIT_TAG");
        }
        break;

    case WRITE_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
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
            log_info(logger, "Ejecutando la operacion WRITE_BLOCK");
        }
        break;

    case READ_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            /*Dado un File:Tag y número de bloque lógico, la operación de lectura obtendrá y devolverá el contenido del mismo.*/
            
            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
                Lectura_o_escritura_fuera_de_limite
            */
            log_info(logger, "Ejecutando la operacion READ_BLOCK");
        }
        break;

    case DELETE_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            /*Esta operación eliminará el directorio correspondiente al File:Tag indicado. 
            Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado 
            no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.*/
            // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

            /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
                File_inexistente
                Tag_inexistente
            */
            log_info(logger, "Ejecutando la operacion DELETE_TAG");
        }
        break;

    default:
        log_error(logger, "CODIGO DE OPERACION INVALIDO CODIGO: %d", opcode);
        break;
    }

    // esto es una respuesta barata, despues le agrego a cada uno su respuesta personalizada
    t_packet* response = create_packet();
    int result_code = 999; // ponele que es un ok por ahora
    add_int_to_packet(response, result_code);
    send_and_free_packet(response, sock_client);
            
    if (args != NULL)
    {
        string_iterate_lines(args, (void*) free);
        free(args);
    }

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