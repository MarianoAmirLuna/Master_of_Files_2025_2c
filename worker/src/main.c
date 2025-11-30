#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_WORKER;
    create_log("worker", cw.log_level);
    log_trace(logger, "%s", "Hola soy WORKER");
    instance_signal_handler();
    if(argc == 3){
        char* config_path = argv[1];
        id_worker = atoi(argv[2]);
        load_config(config_path);
    }else{
        log_error(logger, "Cantidad inválida de argumentos: %d", argc);
        id_worker = 0;
        load_config("worker.config");
    }
    actual_worker = malloc(sizeof(worker));
    actual_worker->id = id_worker;
    actual_worker->is_free = true;
    cw = load_config_worker();

    inicializar_worker();
    
    pthread_mutex_t locker;
    pthread_mutex_init(&locker, NULL);
    
    int arr[] = {MODULE_MASTER, MODULE_STORAGE};
    for(int i=0;i<2;i++){
        //Como el ciclo va tan rápido necesito bloquear el subproceso principal
        pthread_mutex_lock(&locker);
        int v =arr[i];
        add_thread_by_enum(v);
        pthread_t* pth = get_pthread_by_enum(v);
        
        void* parameters = malloc(sizeof(int));
        memcpy(parameters, &v, sizeof(int));
        pthread_create(pth, NULL, connect_to_server, parameters);
        pthread_detach(*pth);
        pthread_mutex_unlock(&locker);
    }
    pthread_mutex_destroy(&locker);


    loop_atender_queries();

    return 0;
}

void* connect_to_server(void* params){
    op_code_module ocm;
    memcpy(&ocm, params, sizeof(op_code_module));
    free(params);
    log_debug(logger, "Connectando al servidor: %s", ocm_to_string(ocm)); 
    int wcl = 0; 
    if(ocm == MODULE_MASTER){
        wcl = client_connection(cw.ip_master,cw.puerto_master);
        sock_master = wcl;
    }
    if(ocm == MODULE_STORAGE){
        wcl = client_connection(cw.ip_storage,cw.puerto_storage);
        sock_storage = wcl;
    }
    if(handshake(wcl, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", wcl, ocm_to_string(ocm));
        exit(EXIT_FAILURE);
    }
    
    t_packet* p = create_packet();
    add_int_to_packet(p,itself_ocm);
    add_int_to_packet(p, id_worker);
    send_and_free_packet(p, wcl); //debo enviar john snow e id_worker a sus respectivos módulos conectados

    if(ocm == MODULE_STORAGE){
        t_list* l = recv_operation_packet(wcl);
        block_size = list_get_int(l, 1); //Véase en main.c de Storage en primer índice envían un ENUM BLOCK_SIZE 
        storage_block_size = block_size;
        data_bloque = malloc(storage_block_size);
        log_trace(logger, "TENGO EL BLOCK SIZE DEL STORAGE: %d", block_size);
        list_destroy_and_destroy_elements(l, free_element);
    }

    //add_socket_structure_by_name_ocm_sock_server(ocm_to_string(ocm), ocm, wcl, 0);
    
    void* parameters = malloc(sizeof(int)*3);
    int len_args = 2;
    int offset = 0;
    memcpy(parameters, &len_args, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &ocm, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &wcl, sizeof(int));

    loop_network(wcl, packet_callback, parameters, NULL);
    free(parameters);
    return NULL;
}

void packet_callback(void* params){
    int cntargs = 0;
    int sock = -1;
    op_code_module ocm=0;

    int offset = 0;
    memcpy(&cntargs, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock, params+offset, sizeof(int));
    //free(params);
    log_debug(logger, "OCM: %s", ocm_to_string(ocm));
    t_list* packet = recv_packet(sock);
    log_info(logger, "Recibi mensaje de %s cantidad del packet: %d", ocm_to_string(ocm), list_size(packet));
    
    int op_code = list_get_int(packet, 0); //si del otro lado se usa el add_int_to_packet(...) entonces es recomendable utilizar el list_get_int(...) si por alguna razón da malos valores intentar con casteo (int)...
    log_pink(logger,"OPCODE: %d = %s", op_code, get_opcode_as_string(op_code));
    if(ocm == MODULE_MASTER){
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE MASTER
        if(op_code == REQUEST_EXECUTE_QUERY){
            qid id_query =list_get_int(packet, 1);
            int pc = list_get_int(packet, 2);
            char* str =list_get_str(packet, 3);
            archivo_query_actual = malloc(strlen(str)+1);
            strcpy(archivo_query_actual, str);
            
            
            actual_worker->is_free=false;
            actual_worker->id_query = id_query;
            log_pink(logger, "Por re-setear el actual_query");
            
            actual_query = create_basic_query(id_query, archivo_query_actual, pc);

            log_info(logger, "## Query %d: Se recibe la Query. El path de operaciones es: %s", id_query, archivo_query_actual); 
            sem_post(&sem_query_recibida); //Aviso que ya tengo una query para ejecutar
            free(str);
        }
        if(op_code == REQUEST_DESALOJO){
            /*if(actual_worker == NULL || actual_query == NULL){
                t_packet* p = create_packet();
                add_int_to_packet(p, ERROR);
                send_and_free_packet(p, sock);
                list_destroy(packet);
                return;
            }*/
            //qid id_query = list_get_int(packet, 1);
            need_stop=1;
            
            sem_wait(&sem_need_stop);
            
            flushear_tabla_paginas(false);

            t_packet* p = create_packet();
            add_int_to_packet(p, SUCCESS);
            add_int_to_packet(p, actual_query->id);
            add_int_to_packet(p, actual_query->pc);
            
            send_and_free_packet(p, sock);
            actual_worker->is_free=true;

            log_info(logger, "## Query %d: Desalojado por pedido del Master", actual_query->id);
            actual_worker->id_query = -1;
            free(actual_query);
            need_stop=0;
        }
    }
    if(ocm == MODULE_STORAGE){
        if(op_code == GET_DATA){
       
            log_light_blue(logger, "Tamaño del paquete: %d", list_size(packet));
            int returned = list_get_int(packet, 0);
            if(returned != GET_DATA)
            {
                log_error(logger, "Ehh que pasó acá esto no es GET_DATA esto es: %d exit 1 is invoked", returned);
                exit(1);
            }
            else{
                log_orange(logger, "Estoy en get data");
                char* data = list_get_str(packet, 1);
                memcpy(data_bloque, data, storage_block_size);
                sem_post(&sem_get_data);
            }
        }
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE STORAGE
        /*if(op_code == BLOCK_SIZE)
        {
            storage_block_size = list_get_int(packet, 1);
            
            data_bloque = malloc(storage_block_size);
        }*/
        if(op_code==INSTRUCTION_ERROR || op_code==FILE_NOT_FOUND || op_code==TAG_NOT_FOUND || op_code==INSUFFICIENT_SPACE || op_code==WRITE_NO_PERMISSION || op_code==READ_WRITE_OVERFLOW)
        {
            t_packet* paq=create_packet();
            add_int_to_packet(paq, op_code);
            send_and_free_packet(paq, sock_master);
        }
        if(op_code == SUCCESS)
        {
        }
    }
    list_destroy(packet); //véase como en por ejemplo EJECUTAR_QUERY al recibir el list_get_str luego lo libero,
    //como el resto son enteros, los enteros no se pueden liberar porque lo hace el compilador
    //Por lo tanto sólo debo destruir/liberar la lista
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