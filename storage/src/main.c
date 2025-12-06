#include "main.h"

/*
+----------------------------------------------------------------------------------------------+
|                        Inicio de Modulo - Conexion con Otros Modulos                         |
+----------------------------------------------------------------------------------------------+
*/


int main(int argc, char* argv[]) {
    //TODO: ESTE MODULO TAMBIEN TIENE EL superblock.config y el blocks_hash_index.config
    itself_ocm = MODULE_STORAGE;
    if(argc >= 2){
        char* config_path = argv[1];
        load_config(config_path);
    }else{
        load_config("storage.config");
    }
    cs =load_config_storage();
    create_log("storage", cs.log_level);
    log_violet(logger, "%s", "Hola soy STORAGE");
    instance_signal_handler();
    workers= list_create();

    inicializar_file_system();
    creo_semaforos_fs();

    int sock_server = server_connection(cs.puerto_escucha);

    void* param = malloc(sizeof(int)*2);
    memcpy(param, &sock_server, sizeof(int));
    memcpy(param+sizeof(int), &cs.puerto_escucha, sizeof(int));
    attend_multiple_clients(param);
    return 0;
}

worker* get_worker_by_id(int id){
    for(int i=0;i<list_size(workers);i++){
        worker* w = (worker*)list_get(workers, i);
        if(w->id == id){
            return w;
        }
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
        int id_worker = list_get_int(l, 1);
        list_destroy_and_destroy_elements(l, free_element);

        worker* w = malloc(sizeof(worker)); //El único que se conecta es un worker
        w->id = id_worker;
        w->id_query = -1;
        w->fd = sock_client;

        list_add(workers, w);
        log_info(logger, "## Se conecta el Worker %d - Cantidad de Workers: %d", w->id, list_size(workers));

        char* superblockpath = string_from_format("%s/%s", cs.punto_montaje, "superblock.config");
        t_config* c_superblock = config_create(superblockpath);
        int block_size = config_get_int_value(c_superblock,"BLOCK_SIZE");
        log_pink(logger, "El BLOCK_SIZE es: %d", block_size);
        t_packet* pack = create_packet();
        add_int_to_packet(pack, BLOCK_SIZE);
        add_int_to_packet(pack, block_size);
        send_and_free_packet(pack, sock_client);

        config_destroy(c_superblock);

        void* parameter = malloc(sizeof(int)*4);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &id_worker, sizeof(int));

        pthread_t* pth = (pthread_t*)malloc(sizeof(pthread_t));
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

// nucleo del storage
void packet_callback(void* params){
    log_info(logger, "Inside here");

    int sock_client = 0;
    op_code_module ocm= 0; // aca veo el modulo que se comunico, deberia ser siempre worker (MODULE_WORKER: 9)
    int sock_server=0;
    int offset= 0;
    int id_worker= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&id_worker, params+offset, sizeof(int));


    log_info(logger, "En el packet callback sock_client: %d, ocm: %d, sock_server:%d id_worker: %d", sock_client, ocm, sock_server, id_worker);
    t_list* pack = recv_packet(sock_client); // aca estarian las operaciones

    //En base a lo que entendí del TP el QUERY_ID sería la primera cosa que tendría cuando worker le manda una solicitud al Storage (este)
    //SI es el caso entonces se invoca el list_get_int(pack, 0) para obtener el id_query
    //Véase que qid o wid son typedef de int, por lo tanto SON INT

    qid id_query = list_get_int(pack, 0);
    worker* w = get_worker_by_id(id_worker);
    log_light_blue(logger, "WORKERS: %d", list_size(workers));
    if(w == NULL){
        log_error(logger, "EL WORKER ES NULO EN (%s:%d)", __func__,__LINE__);
    }
    w->id_query = id_query;
    //Necesitaría el id_query???????? Porque en los logs figura el uso de ID_QUERY

    log_pink(logger, "RECIBI DATOS DEL %s", ocm_to_string(ocm));

    tratar_mensaje(pack, w, sock_client); // manejo de operaciones del worker

    list_destroy(pack);
}

void disconnect_callback(void* params){
    int sock_client = 0;
    op_code_module ocm= 0;
    int sock_server=0;
    int offset= 0;
    int id_worker=0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&id_worker, params+offset, sizeof(int));

    worker* w = get_worker_by_id(id_worker);
    if(w != NULL){
        //Existe este worker y lo puedo eliminar
        int sz = list_size(workers);
        for(int i=0;sz;i++){
            worker* wl = (worker*)list_get(workers, i);
            if(wl != NULL){
                if(wl->id == w->id)
                {
                    free(list_remove(workers, i));
                    break;
                }
            }
        }
    }else{
        log_error(logger, "No exist el worker en la lista, del id_worker=%d", id_worker);
    }

    log_info(logger, "## Se desconecta el Worker %d - Cantidad de Workers: %d", id_worker, list_size(workers));
    log_warning(logger, "Se desconectó el cliente %s fd:%d", ocm_to_string(ocm), sock_client);
}

/// @brief Comprueba si el file o el tag existe, en caso de no existir se envía el packet de error y se retorna falso
/// @param file 
/// @param tag 
/// @param w 
/// @return Verdadero si existe, falso caso contrario.
int file_tag_exist_or_not(char* file, char* tag, worker* w){
    char* filespath =get_files_from_punto_montaje(cs);
    char* fullpath = string_from_format("%s/%s", filespath, file);
    free(filespath);
    if(!directory_exists(fullpath)){
        send_basic_packet(w->fd, FILE_NOT_FOUND); //File inexistente
        log_error(logger, "No se encontro el file deseado");
        free(fullpath);
        return 0;
    }
    char* fullpathtag = string_from_format("%s/%s", fullpath, tag);
    if(!directory_exists(fullpathtag)){
        send_basic_packet(w->fd, TAG_NOT_FOUND); //Tag inexistente
        log_error(logger, "No se encontro el tag deseado");
        free(fullpath);
        free(fullpathtag);
        return 0;
    }
    free(fullpath);
    free(fullpathtag);
    return 1;
}

int file_tag_exist_or_not_not_error(char* file, char* tag, worker* w){
    char* filespath =get_files_from_punto_montaje(cs);
    char* fullpath = string_from_format("%s/%s", filespath, file);
    free(filespath);
    if(!directory_exists(fullpath)){
        log_error(logger, "No se encontro el file deseado");
        free(fullpath);
        return 0;
    }
    char* fullpathtag = string_from_format("%s/%s", fullpath, tag);
    if(!directory_exists(fullpathtag)){
        log_error(logger, "No se encontro el tag deseado");
        free(fullpath);
        free(fullpathtag);
        return 0;
    }
    free(fullpath);
    free(fullpathtag);
    return 1;
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

/*
+----------------------------------------------------------------------------------------------+
|                              Fin de Conexion con Otros Modulos                               |
+----------------------------------------------------------------------------------------------+


************************************************************************************************
*/