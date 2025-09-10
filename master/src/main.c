#include "main.h"
int main(int argc, char* argv[]) {
    itself_ocm = MODULE_MASTER;
    load_config("master.config");
    cm =load_config_master();
    create_log("master", cm.log_level);
    log_violet(logger, "%s", "Hola soy MASTER");
    
    queries = list_create();
    workers = list_create();
    dict_state = dictionary_create();
    for(int i=STATE_READY;i<=STATE_EXIT;i++){
        dictionary_put(dict_state, state_to_string(i), list_create());
    }
    sem_init(&sem_idx, 0,1);
    
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

        int id = -1;
        if(ocm == MODULE_QUERY_CONTROL){
            char* archive_query= (char*)list_get(l, 1);
            int prioridad = list_get_int(l,2);
            log_orange(logger, "Recibi el dato de Query Control: Query:%s, Prioridad: %d", archive_query, prioridad);
            query* q = malloc(sizeof(query));
            q->archive_query = malloc(strlen(archive_query)+1);
            strcpy(q->archive_query, archive_query);
            q->priority = prioridad;
            q->fd = sock_client;
            q->id = increment_idx();
            q->sp = STATE_READY;
            id = q->id;
            list_add(queries, q);
            log_info(logger, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento: %d",
                q->archive_query,
                q->priority,
                q->id,
                list_size(workers)
            );
            //Deberia hacer una planificación ahora mismo para saber si puede asignar un 
        }
        if(ocm == MODULE_WORKER){
            int id_worker = list_get_int(l,1);
            worker* w = malloc(sizeof(worker));
            w->id = id_worker;
            w->id_query = -1; //No se asignó ningún query a este worker
            w->fd = sock_client;
            id = id_worker;

            list_add(workers, w);
            degree_multiprocess = list_size(workers);
            //Acaso cuando recibo el id_worker en seguida le tengo que asignar un query?? de ahí para pasar el path de la query al worker???????
            log_orange(logger, "Recibi el id_worker de Worker: ID_WORKER = %d", id_worker);
            log_info(logger, "## Se conecta el Worker %d - Cantidad total de Workers: %d", id_worker, list_size(workers));
        }

        list_destroy_and_destroy_elements(l, free_element);

        void* parameter = malloc(sizeof(int)*4);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &id, sizeof(int));
        pthread_t* pth = malloc(sizeof(pthread_t));
        pthread_create(pth, NULL,go_loop_net, parameter);
        pthread_detach(*pth);
    }
}

void* go_loop_net(void* params){
    int len=sizeof(int)*4;
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
    int id =-1;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&id, params+offset, sizeof(int));
    
    if(ocm == MODULE_QUERY_CONTROL){
        //TODO: Si se desconectó y la query se encuentra en Ready se debe mandar exit directamente
        //TODO: ahora si estaba en Exec se debe notificar al Worker que la está ejecutando justo ese query y debe desalojar la query.
        qid id_query = id;
        query* q = get_query_by_qid(id_query);
        if(q->sp == STATE_READY){
            t_queue* que = get_queue_by_sp(STATE_READY);
            list_remove_by_condition_by(que->elements, by_query_qid, id_query);
            query_to(q, STATE_EXIT);
            //Se debe mandar a exit directamente
        }
        if(q->sp == STATE_EXEC){
            //Se debe notificar al worker que está ejecutando esto y pedir que desaloje
            worker* w = get_worker_by_qid(q->id);
            if(w == NULL){ //No se encuentra
                log_warning(logger, "%s","No se encontró el worker en la lista de workers");
            }else{
                t_packet* p = create_packet();
                add_int_to_packet(p, REQUEST_DESALOJO);
                add_int_to_packet(p, q->id);
                send_and_free_packet(p, w->fd);
                t_list* recpd = recv_operation_packet(w->fd);
                response resp = list_get_int(recpd ,0);
                if(resp == SUCCESS){
                    //Ok puedo notificar error al Query
                }
                //Me tendra que responder el PC???
                //log_info(logger, "## Se desaloja la Query")
            }
        }
    }
    
    if(ocm == MODULE_WORKER){
        wid id_worker = id;
        //Si se desconectó un worker la query que se encontraba en ejecución en ese Worker se finalizará con error y notificará al Query Control correspondiente.
        int idx = -1;
        worker* w = get_worker_by_wid(id_worker);
        //worker* w = get_worker_by_fd(sock_client, &idx);
        
        if(idx != -1 && w != NULL){

            query* q = get_query_by_qid(w->id_query);
            if(q != NULL){
                //Notificar al query
                t_packet* p = create_packet();
                add_int_to_packet(p, ERROR);
                
                send_and_free_packet(p, q->fd);
            }
            q->sp = STATE_EXIT;
            int id_worker = w->id;
            int qid = w->id_query;
            
            //TODO: free pointer worker
            list_remove(workers, idx);
            degree_multiprocess = list_size(workers);

            log_info(logger, "## Se desconecta el Worker %d - Se finaliza la Query %d - Cantidad total de Workers: %d",
                id_worker,
                qid,
                degree_multiprocess
            );
            w->is_free=1;
        }
        
    }
    log_warning(logger, "Se desconecto el cliente de %s fd:%d", ocm_to_string(ocm), sock_client);
}

void packet_callback(void* params){
    log_info(logger, "Inside here");

    int sock_client = 0;
    op_code_module ocm= 0;
    int sock_server=0;
    int offset= 0;
    int id =-1;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&id, params+offset, sizeof(int));

    log_info(logger, "En el packet callback sock_client: %d, ocm: %d, sock_server:%d", sock_client, ocm, sock_server);
    t_list* pack = recv_packet(sock_client);
    
    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }
    
    if(ocm == MODULE_QUERY_CONTROL) {
        log_pink(logger, "RECIBI DATOS DEL QUERY_CONTROL");
        work_query_control(pack, id, sock_client);
    }
    if(ocm == MODULE_WORKER){
        log_pink(logger, "RECIBI DATOS DEL MODULE WORKER");
        work_worker(pack, id, sock_client);
    }

    list_destroy_and_destroy_elements(pack, free_element);
}

void work_query_control(t_list* packet, int id, int sock){

}

void work_worker(t_list* pack, int id, int sock){
    int opcode = list_get_int(pack, 0);

    if(opcode == QUERY_END){
        worker* w = get_worker_by_wid(id);
        log_info(logger, "## Se terminó la Query: %d en el Worker %d",
            w->id_query, id
        );
    }
}