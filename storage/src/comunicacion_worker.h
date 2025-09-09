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
            log_info(logger, "Ejecutando la operacion CREATE_FILE");
            crear_directorio(args[0], cs.punto_montaje); // creo el archivo (no verifico si existe)
            char* path = string_from_format("%s/%s", cs.punto_montaje, args[0]);
            crear_directorio(args[1], path);
            path = string_from_format("%s/%s", path, args[1]);
            crear_directorio("logical_blocks", path);
            crear_metadata_config(path, g_block_size, NULL, WORK_IN_PROGRESS);
            free(path);
        }
        break;

    case TRUNCATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            log_info(logger, "Ejecutando la operacion TRUNCATE_FILE");
        }
        break;

    case TAG_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            log_info(logger, "Ejecutando la operacion TAG_FILE");
        }
        break;

    case COMMIT_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            log_info(logger, "Ejecutando la operacion COMMIT_TAG");
        }
        break;

    case WRITE_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            log_info(logger, "Ejecutando la operacion WRITE_BLOCK");
        }
        break;

    case READ_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            log_info(logger, "Ejecutando la operacion READ_BLOCK");
        }
        break;

    case DELETE_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
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