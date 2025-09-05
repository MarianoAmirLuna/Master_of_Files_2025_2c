#include "main.h"
int main(int argc, char* argv[]) {
    itself_ocm = MODULE_MASTER;
    load_config("master.config");
    cm =load_config_master();
    create_log("master", cm.log_level);
    log_violet(logger, "%s", "Hola soy MASTER");
    
    queries = list_create();
    
    int sock_server = server_connection(cm.puerto_escucha);
    
    void* param = malloc(sizeof(int)*2);
    memcpy(param, &sock_server, sizeof(int));
    memcpy(param+sizeof(int), &cm.puerto_escucha, sizeof(int));
    
    pthread_t* pth = malloc(sizeof(pthread_t));
    pthread_create(pth, NULL, scheduler, NULL);
    pthread_detach(*pth);

    attend_multiple_clients(param);
    
    //TODO: Instance handler key down for kill ALL SOCKETS

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

        if(ocm == MODULE_QUERY_CONTROL){
            char* archive_query= (char*)list_get(l, 1);
            int prioridad = list_get_int(l,2);
            log_orange(logger, "Recibi el dato de Query Control: Query:%s, Prioridad: %d", archive_query, prioridad);
        }
        if(ocm == MODULE_WORKER){
            int id_worker = list_get_int(l,1);
            log_orange(logger, "Recibi el id_worker de Worker: ID_WORKER = %d", id_worker);
        }

        list_destroy_and_destroy_elements(l, free_element);

        //TODO: Implement query structure of this id_worker, etc. 
        //list_add(queries, )


        /*if(ocm == MODULE_QUERY_CONTROL){
            //Se podría justo en este momento al recibir el prioridad y path queries agregar en una lista de query_control sus campos
        }*/
        void* parameter = malloc(sizeof(int)*3);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        pthread_t* pth = malloc(sizeof(pthread_t));
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
    
    
    
    if(ocm == MODULE_QUERY_CONTROL){
        //TODO: Si se desconectó y la query se encuentra en Ready se debe mandar exit directamente
        //ahora si estaba en Exec se debe notificar al Worker que la está ejecutando justo ese query y debe desalojar la query.
    }
    
    if(ocm == MODULE_WORKER){
        //Si se desconectó un worker la query  que se encontraba en ejecución en ese Worker se finalizará con error y notificará al Query Control correspondiente.

    }
    log_warning(logger, "Se desconecto el cliente de %s QUERY CONTROL fd:%d", ocm_to_string(ocm), sock_client);
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
    
    if(ocm == MODULE_QUERY_CONTROL) {
        log_pink(logger, "RECIBI DATOS DEL QUERY_CONTROL");
        //Si se quiere responder se utiliza el sock_client
        //Ejemplo send_and_free_packet(packet, sock_client);
    }
    if(ocm == MODULE_WORKER){
        log_pink(logger, "RECIBI DATOS DEL MODULE WORKER");
    }

    list_destroy_and_destroy_elements(pack, free_element);
}