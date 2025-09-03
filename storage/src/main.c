#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_STORAGE;
    load_config("storage.config");
    cs =load_config_storage();
    create_log("storage", cs.log_level);
    log_violet(logger, "%s", "Hola soy STORAGE");
    
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
        
        void* parameter = malloc(sizeof(int)*3);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        pthread_t* pth = (pthread_t*)malloc(sizeof(pthread_t));
        int res_create = pthread_create(pth, NULL,go_loop_net, parameter);
        int res = pthread_detach(*pth);
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
        data
    );
    free(data);
    
    return NULL;
}

void packet_callback(void* params){
    log_info(logger, "Inside here");

    int sock_client = 0;
    op_code_module ocm= 0;
    int sock_server=0;
    int offset= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    log_info(logger, "En el packet callback sock_client: %d, ocm: %d, sock_server:%d", sock_client, ocm, sock_server);
    t_list* pack = recv_packet(sock_client);
    
    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }
    log_pink(logger, "RECIBI DATOS DEL %s", ocm_to_string(ocm));
    
    list_destroy_and_destroy_elements(pack, free_element);
}