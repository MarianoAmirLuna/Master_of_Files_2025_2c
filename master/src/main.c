#include "main.h"
#include "readline/readline.h"
int main(int argc, char* argv[]) {
    itself_ocm = MODULE_MASTER;
    char* config_path = argv[1];
    load_config(config_path);
    //load_config("master.config");
    cm =load_config_master();
    create_log("master", cm.log_level);
    log_violet(logger, "%s", "Hola soy MASTER");
    pthread_mutex_init(&mutex_sched, NULL);
    instance_signal_handler();
    queries = list_create();
    workers = list_create();
    dict_state = dictionary_create();
    for(int i=STATE_READY;i<=STATE_EXIT;i++){
        dictionary_put(dict_state, state_to_string(i), i==STATE_READY ? queue_create() : list_create());
    }
    sem_init(&sem_idx, 0,1);
    sem_init(&sem_incoming_client, 0,1);
    sem_init(&sem_locker, 0,1);
    sem_init(&sem_worker, 0,1);
    sem_init(&sem_desalojo, 0,0);
    int sock_server = server_connection(cm.puerto_escucha);
    
    void* param = malloc(sizeof(int)*2);
    memcpy(param, &sock_server, sizeof(int));
    memcpy(param+sizeof(int), &cm.puerto_escucha, sizeof(int));
    
    pthread_t* pth = malloc(sizeof(pthread_t));
    pthread_create(pth, NULL, scheduler, NULL);
    pthread_detach(*pth);

    /*pthread_t* pth_console = malloc(sizeof(pthread_t));
    pthread_create(pth_console, NULL, console, NULL);
    pthread_detach(*pth_console);*/
    
    attend_multiple_clients(param);
    
    
    return 0;
}

void* console(){
    log_light_blue(logger, "%s", "Console instance");
    //Solo para solicitar boludeces por consola
    for(;;){
        char* rd = readline(">");
        if(string_is_empty(rd))
        {
            log_warning(logger, "No ingresaste ningún comando");
            free(rd);
            break;
        }
        char** spl = string_split(rd, ":");
        int spl_size = string_array_size(spl);
        log_pink(logger, "Comando recibido: %s - Cantidad de argumentos: %d", rd, spl_size);
        for(int n=0;n<list_size(workers);n++){
            worker* w = list_get(workers, n);
            t_packet* p = create_packet();
            for(int i=0;i<spl_size;i++){
                add_int_to_packet(p, atoi(spl[i]));
            }
            send_and_free_packet(p, w->fd);
            log_light_green(logger, "Envié a Worker %d el comando %s", w->id, rd);
            sleep(1);
        }
        free(rd);
    }
    return NULL;
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
        sem_wait(&sem_incoming_client);
        int id = -1;
        if(ocm == MODULE_QUERY_CONTROL){
            int prioridad = list_get_int(l,1);
            char* archive_query= list_get_str(l,2);
            //log_pink(logger, "Tamaño del archive_query = %d", strlen(archive_query));
            query* q = malloc(sizeof(query));
            q->archive_query = string_duplicate(archive_query);
            /*q->archive_query = (char*)malloc(strlen(archive_query)+1);
            strcpy(q->archive_query, archive_query);*/
            q->priority = prioridad;
            q->fd = sock_client;
            q->id = increment_idx();
            q->sp = STATE_READY;
            q->temp = temporal_create();
            id = q->id;
            log_orange(logger, "Recibi el dato de Query Control: Query:%s ID=%d, Prioridad: %d", archive_query, q->id, prioridad);
            add_query_on_state(q, q->sp);
            list_add(queries, q);
            log_info(logger, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento: %d",
                q->archive_query,
                q->priority,
                q->id,
                list_size(workers)
            );
            free(archive_query);
            //print_queries();
            //Deberia hacer una planificación ahora mismo para saber si puede asignar un 
        }
        if(ocm == MODULE_WORKER){
            pthread_mutex_lock(&mutex_sched);
            int id_worker = list_get_int(l,1);
            worker* w = malloc(sizeof(worker));
            w->id = id_worker;
            w->id_query = -1; //No se asignó ningún query a este worker
            w->fd = sock_client;
            w->is_free = 1; //Al conectarse el worker está libre
            w->resp_desalojo = (response_desalojo){-1, -1, -1};
            id = id_worker;

            list_add(workers, w);

            pthread_mutex_unlock(&mutex_sched);
            degree_multiprocess = list_size(workers);
            //Acaso cuando recibo el id_worker en seguida le tengo que asignar un query?? de ahí para pasar el path de la query al worker???????
            log_orange(logger, "Recibi el id_worker de Worker: ID_WORKER = %d", id_worker);
            log_info(logger, "## Se conecta el Worker %d - Cantidad total de Workers: %d", id_worker, list_size(workers));
        }

        list_destroy(l);

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
        sem_post(&sem_incoming_client);
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
    log_warning(logger, "HANDLIND DISCONNECTION CALLBACK OCM: %s", ocm_to_string(ocm));
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
                query_to(q, STATE_EXIT);
                /*if(desalojo(w))
                {
                    query_to(q, STATE_EXIT);
                }else{
                    log_warning(logger, "No se pudo desalojar el worker %d que ejecutaba la query %d", w->id, q->id);   
                }*/
                //Me tendra que responder el PC???
                log_info(logger, "## Se desaloja la Query <%d> (%d) del Worker <%d> - Motivo: DESCONEXION",
                    q->id,
                    q->priority,
                    w->id
                );
            }
        }

        log_info(logger, "## Se desconecta un Query Control. Se finaliza la Query %d con prioridad %d. Nivel multiprocesamiento %d", 
            id_query, q->priority, list_size(workers)
        );
    }
    
    if(ocm == MODULE_WORKER){
        wid id_worker = id;
        //Si se desconectó un worker la query que se encontraba en ejecución en ese Worker se finalizará con error y notificará al Query Control correspondiente.
        int idx = -1;
        log_light_blue(logger, "HERE 1");
        //worker* w = get_worker_by_wid(id_worker);
        worker* w = list_find_by_idx_list(workers, by_worker_wid, (int)id_worker, &idx);
        //worker* w = get_worker_by_fd(sock_client, &idx);
        pthread_mutex_lock(&mutex_sched);
        if(idx != -1 && w != NULL){
            log_light_blue(logger, "HERE 2: idx=%d", idx);
            int qid = w->id_query;
            int id_worker = w->id;

            query* q = get_query_by_qid(w->id_query);
            log_light_blue(logger, "HERE 3: Q is NULL? %d", q == NULL);
            if(q != NULL){
                //Notificar al query
                t_packet* p = create_packet();
                add_int_to_packet(p, ERROR);
                send_and_free_packet(p, q->fd);
                log_light_blue(logger, "HERE 4: sending to %d", q->fd);
                q->sp = STATE_EXIT;
                on_changed(on_query_state_changed,   q);
                
            }else{
                log_warning(logger, "No se encontró la query que estaba ejecutando el worker %d", w->id);
            }
            
            //WARNING: Test this
            log_light_blue(logger, "Removiendo el elemento en el indice %d tengo en total: %d", idx, list_size(workers));
            free_element(list_remove(workers, idx));
            degree_multiprocess = list_size(workers);

            log_info(logger, "## Se desconecta el Worker %d - Se finaliza la Query %d - Cantidad total de Workers: %d",
                id_worker,
                qid,
                degree_multiprocess
            );
        }
        log_light_blue(logger, "HERE 5: idx=%d", idx);
        pthread_mutex_unlock(&mutex_sched);
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

    list_destroy(pack);
}

void work_query_control(t_list* packet, int id, int sock){
    
}

void work_worker(t_list* pack, int id, int sock){
    int opcode = list_get_int(pack, 0);
    worker* w = get_worker_by_wid(id);
    /*if(opcode == REPORT_ERROR){
        log_error(logger, "## El Worker %d reporta un ERROR en la Query %d",
            id,
            w->id_query
        );
        query* q = get_query_by_qid(w->id_query);
        if(q == NULL){
            log_error(logger, "La query es NULL (%s:%d)", __func__,__LINE__);
        }
        t_packet* p = create_packet();
        add_int_to_packet(p, ERROR);
        send_and_free_packet(p, q->fd);
        w->id_query = -1; //Debo especificar que ahora este worker no tiene asignado ningún query.
        w->is_free=1;
    }*/
   if(opcode == REQUEST_DESALOJO){
        log_light_blue(logger, "Respuesta del worker desalojo %d", id);
        int status = list_get_int(pack, 1);
        int qid = list_get_int(pack, 2);
        int pc = list_get_int(pack, 3);
        log_orange(logger, "El Worker %d solicita desalojo de la Query %d en PC %d",
            id,
            qid,
            pc
        );
        w->resp_desalojo.status = status;
        w->resp_desalojo.id_query = qid;
        w->resp_desalojo.pc = pc;
        
        sem_post(&sem_desalojo);
    }
    if(opcode == QUERY_END || opcode==INSTRUCTION_ERROR || opcode==FILE_NOT_FOUND || opcode==TAG_NOT_FOUND || opcode==INSUFFICIENT_SPACE || opcode==WRITE_NO_PERMISSION || opcode==READ_WRITE_OVERFLOW)
    {
        int qid;
        if(opcode == QUERY_END){
            qid = list_get_int(pack, 1);
            log_info(logger, "## Se terminó la Query: %d en el Worker %d",
                qid, id
            );
        }else{
            qid = w->id_query;
            log_orange(logger, "El Worker %d reporta un ERROR en la Query %d - OPCODE ERROR: %d",
                id,
                qid,
                opcode
            );
        }
        query* q = get_query_by_qid(qid);
        if(q == NULL || qid == -1){
            log_error(logger, "La query %d es NULL (%s:%d)", qid, __func__,__LINE__);
            w->id_query = -1; //Debo especificar que ahora este worker no tiene asignado ningún query.
            w->is_free=1;
            return;
        }
        t_packet* p = create_packet();
        add_int_to_packet(p, REQUEST_KILL);
        add_string_to_packet(p, opcode == QUERY_END ? "Por fin de query" : get_motivo_error(opcode));
        send_and_free_packet(p, q->fd);
        q->sp= STATE_EXIT;
        log_light_blue(logger, "Un query (ID=%d,Archivo=%s) cambió de estado valor: %s", q->id, q->archive_query, state_to_string(q->sp));
        w->id_query = -1; //Debo especificar que ahora este worker no tiene asignado ningún query.
        w->is_free=1;
    }
    if(opcode == REQUEST_READ){
        query* q = get_query_by_qid(w->id_query);
        
        char* filetag = list_get_str(pack,1);
        char* content = list_get_str(pack, 2);
        t_packet* p = create_packet();
        add_int_to_packet(p, opcode);
        add_string_to_packet(p, filetag);
        add_string_to_packet(p, content);
        send_and_free_packet(p, q->fd);
        log_info(logger, "## Se envia un mensaje de lectura de la Query %d en el Worker %d al Query Control", 
            w->id_query, 
            id
        );

        free(content);
        free(filetag);
    }
    if(opcode == ACTUAL_STATUS){
        int is_free = list_get_int(pack, 1);
        int id_query = list_get_int(pack, 2);
        int pc = list_get_int(pack, 3);

        log_orange(logger, "Estado actual: Worker %d, Query %d, PC %d, Esta libre: %d",
            id, id_query, pc, is_free
        );
    }
}

void instance_signal_handler(){
    if(signal(SIGINT, catch_handler_termination) == SIG_ERR){
        log_error(logger, "Problema seteando un handler para señales");
    }
    if(signal(SIGTERM, catch_handler_termination) == SIG_ERR){
        log_error(logger, "Problema seteando un handler para señales");
    }
    if(signal(SIGABRT, catch_handler_termination) == SIG_ERR){
        log_error(logger, "Problema seteando un handler para señales");
    }
}