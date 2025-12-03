#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_QUERY_CONTROL;
    
    create_log("query_control", cqc.log_level);
    instance_signal_handler();
    log_violet(logger, "%s", "Hola soy QUERY CONTROL");

    if(argc <= 3)
    {
        log_pink(logger,"%s, cant: %d", "Cantidad de argumentos inválida exit(1) IS INVOKED", argc);
        exit(1);
    }
    else if(argc == 4){
        //El primer argumento (es decir el argv[0]) es el path del programa en sí o el nombre del program.
        char* path_config=argv[1];
        load_config(path_config);
        
        archive_query = string_duplicate(argv[2]);
        priority = atoi(argv[3]);
        log_orange(logger, "Config: %s, Path Queries: %s, Prioridad: %d", path_config, archive_query, priority);
    }
    if(priority < 0){
        log_warning(logger, "Número de prioridad inválido, se seteó en 0");
        priority=0;
    }
    
    cqc = load_config_query_control();

    //Al conectarse al Master se debe enviar el path del archivo_query y la prioridad

    fd_master = client_connection(cqc.ip_master, cqc.puerto_master);
    if(handshake(fd_master, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", fd_master, ocm_to_string(MODULE_MASTER));
        exit(EXIT_FAILURE);
    }
    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %d", cqc.ip_master, cqc.puerto_master);

    t_packet* packet = create_packet();
    add_int_to_packet(packet, itself_ocm);
    add_int_to_packet(packet, priority);
    add_string_to_packet(packet, archive_query);
    send_and_free_packet(packet, fd_master);
    log_pink(logger, "Tamaño del archive_query = %d", strlen(archive_query));
    for(;;){
        t_list* l = recv_operation_packet(fd_master);
        log_debug(logger, "Recibi: %d cantidad de elementos", list_size(l));
        /*//Por ahora asumo que si recibe cantidad vacía rompo este programa y listo.
        if(list_size(l) == 0)
            break;*/
        int v = list_get_int(l, 0);
        if(v == REQUEST_READ){
            char* archivo = list_get_str(l,1);
            char* contenido = list_get_str(l,2);
            log_info(logger, "## Lectura realizada: %s, contenido: %s", archivo, contenido);
            free(archivo);
            free(contenido);
        }
        if(v == REQUEST_EXECUTE_QUERY){
            //Sera que el Master solicita el query y este modulo le responde? eso es lo que entendí.
            log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %d", archive_query, priority);
            /*t_packet* p = create_packet();
            add_int_to_packet(p, SUCCESS);
            send_and_free_packet(p, fd_master);*/
        }
        if(v == REQUEST_KILL){

            //Se debe poner el log de query finalizada y su motivo...
            //Debe ser un char* el motivo???
            char* motivo = list_get_str(l, 1);
            log_info(logger, "## Query Finalizada - %s", motivo);
            free(motivo);
            close(fd_master);
            break;
        }
        list_destroy(l);
    }
    return 0;
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