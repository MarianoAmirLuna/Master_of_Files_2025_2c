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
            log_info(logger, "Ejecutando la operacion CREATE_FILE");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case TRUNCATE_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            log_info(logger, "Ejecutando la operacion TRUNCATE_FILE");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case TAG_FILE:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            log_info(logger, "Ejecutando la operacion TAG_FILE");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case COMMIT_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            log_info(logger, "Ejecutando la operacion COMMIT_TAG");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case WRITE_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL && args[3] != NULL)
        {
            log_info(logger, "Ejecutando la operacion WRITE_BLOCK");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case READ_BLOCK:
        if(args != NULL && args[0] != NULL && args[1] != NULL && args[2] != NULL)
        {
            log_info(logger, "Ejecutando la operacion READ_BLOCK");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;

    case DELETE_TAG:
        if(args != NULL && args[0] != NULL && args[1] != NULL)
        {
            log_info(logger, "Ejecutando la operacion DELETE_TAG");
            t_packet* response = create_packet();
            int result_code = 999; // ponele que es un ok por ahora
            add_int_to_packet(response, result_code);
            send_and_free_packet(response, sock_client);
        }
        break;
    
    default:
        log_error(logger, "CODIGO DE OPERACION INVALIDO CODIGO: %d", opcode);
        break;
    }

    if (args != NULL)
    {
        string_iterate_lines(args, (void*) free);
        free(args);
    }
    

}

