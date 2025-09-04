#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_WORKER;
    load_config("worker.config");
    cw = load_config_worker();
    create_log("worker", cw.log_level);
    log_violet(logger, "%s", "Hola soy WORKER");
    
    //TODO: El worker tiene 2 argumentos: archivo_config y el ID_Worker
    
    pthread_mutex_t locker;
    pthread_mutex_init(&locker, NULL);

    int arr[] = {MODULE_MASTER, MODULE_STORAGE};
    for(int i=0;i<2;i++){
        //Como el ciclo va tan rápido necesito bloquear el subproceso principal
        pthread_mutex_lock(&locker);
        int v =arr[i];
        add_thread_by_enum(v);
        pthread_t* pth = get_pthread_by_enum(v);
        int res = 0;
        void* parameters = malloc(sizeof(int));
        memcpy(parameters, &v, sizeof(int));
        res = pthread_create(pth, NULL, connect_to_server, parameters);
        log_debug(logger, "Res pthread_create: %d", res);
        res = pthread_detach(*pth);
        pthread_mutex_unlock(&locker);
    }
    pthread_mutex_destroy(&locker);

    for(;;){
        //Fingir hacer algo para que no se muera esta consola
        sleep(10);
    }
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
    send_john_snow_packet(itself_ocm, wcl);
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
    list_destroy_and_destroy_elements(packet, free_element);
}
