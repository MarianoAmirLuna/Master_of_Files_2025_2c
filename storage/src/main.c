#include "main.h"

/*
+----------------------------------------------------------------------------------------------+
|                        Inicio de Modulo - Conexion con Otros Modulos                         |
+----------------------------------------------------------------------------------------------+
*/


int main(int argc, char* argv[]) {
    //TODO: ESTE MODULO TAMBIEN TIENE EL superblock.config y el blocks_hash_index.config
    itself_ocm = MODULE_STORAGE;
    load_config("storage.config");
    cs =load_config_storage();
    create_log("storage", cs.log_level);
    log_violet(logger, "%s", "Hola soy STORAGE");

    inicializar_file_system();

    int sock_server = server_connection(cs.puerto_escucha);

    void* param = malloc(sizeof(int)*2);
    memcpy(param, &sock_server, sizeof(int));
    memcpy(param+sizeof(int), &cs.puerto_escucha, sizeof(int));
    attend_multiple_clients(param);

    return 0;
}


void* attend_multiple_clients(void* params)
{
    int sock=0, port=0;
    memcpy(&sock, params, sizeof(int));
    memcpy(&port, params+sizeof(int), sizeof(int));
    log_debug(logger, "Sock: %d, Port %d", sock,port);
    free(params);
    if(sock <= 0) {
        log_error(logger, "Socket inválido en attend_multiple_clients");
        return NULL;
    }

    for(;;){
        //pthread_mutex_lock(&lock);
        log_debug(logger, "Esperando cliente en socket %d, Puerto: %d", sock, port);
        int sock_client = wait_client(sock, port);
        log_debug(logger, "Socket cliente: %d", sock_client);

        if(handshake(sock_client, 1) != 0)
        {
            log_error(logger, "No pudo hacer handshake con socket %d error %d [%s:%d]", sock_client, errno, __func__, __LINE__);
            close(sock_client);
            continue;  // Continuamos esperando más conexiones en lugar de retornar
        }

        t_list* l = recv_operation_packet(sock_client);
        op_code_module ocm;
        memcpy(&ocm, list_get(l, 0), sizeof(op_code_module));
        list_destroy_and_destroy_elements(l, free_element);

        char* superblockpath = string_from_format("%s/%s", cs.punto_montaje, "superblock.config");
        t_config* c_superblock = config_create(superblockpath);
        int block_size = config_get_int_value(c_superblock,"BLOCK_SIZE");

        t_packet* pack = create_packet();
        add_int_to_packet(pack, BLOCK_SIZE);
        add_int_to_packet(pack, block_size);
        send_and_free_packet(pack, sock_client);

        config_destroy(c_superblock);

        void* parameter = malloc(sizeof(int)*3);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        pthread_t* pth = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(pth, NULL,go_loop_net, parameter);
        pthread_detach(*pth);
    }
}

void* go_loop_net(void* params){
    int len=sizeof(int)*3;
    void* data = malloc(len);
    memcpy(data, params, len);
    int sock_client = 0;
    memcpy(&sock_client, data, sizeof(int));
    free(params);
    loop_network(
        sock_client,
        packet_callback,
        data, 
        disconnect_callback
    );
    free(data);
    
    return NULL;
}

// nucleo del storage
void packet_callback(void* params){
    log_info(logger, "Inside here");

    int sock_client = 0;
    op_code_module ocm= 0; // aca veo el modulo que se comunico, deberia ser siempre worker (MODULE_WORKER: 9)
    int sock_server=0;
    int offset= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    log_info(logger, "En el packet callback sock_client: %d, ocm: %d, sock_server:%d", sock_client, ocm, sock_server);
    t_list* pack = recv_packet(sock_client); // aca estarian las operaciones
    log_pink(logger, "RECIBI DATOS DEL %s", ocm_to_string(ocm));

    tratar_mensaje(pack, sock_client); // manejo de operaciones del worker

    list_destroy_and_destroy_elements(pack, free_element);
}

void disconnect_callback(void* params){
    int sock_client = 0;
    op_code_module ocm= 0;
    int sock_server=0;
    int offset= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));

    log_warning(logger, "Se desconectó el cliente %s fd:%d", ocm_to_string(ocm), sock_client);
}

/*
+----------------------------------------------------------------------------------------------+
|                              Fin de Conexion con Otros Modulos                               |
+----------------------------------------------------------------------------------------------+


************************************************************************************************
*/









