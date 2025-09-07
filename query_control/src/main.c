#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_QUERY_CONTROL;
    
    create_log("query_control", cqc.log_level);
    log_violet(logger, "%s", "Hola soy QUERY CONTROL");

    if(argc <= 3)
    {
        load_config("query_control.config");
        archive_query = "PEPE";

        log_pink(logger,"%s, cant: %d", "Cantidad de argumentos inválida", argc);
    }
    
    //El primer argumento es el path del programa en sí o el nombre del program.
    if(argc == 4){
        char* path_config=argv[1];
        load_config(path_config);
        
        archive_query = string_new();
        strcpy(archive_query, argv[2]);
        
        priority = atoi(argv[3]);
        log_orange(logger, "Config: %s, Path Queries: %s, Prioridad: %d", path_config, archive_query, priority);
    }
    if(priority <= 0){
        log_warning(logger, "Número de prioridad inválido, se seteó en 0");
        priority=0;
    }
    
    cqc = load_config_query_control();

    //Al conectarse al Master se debe enviar el path del archivo_query y la prioridad
    //Por ahora pongo boludeces a enviar

    int fd_master = client_connection(cqc.ip_master, cqc.puerto_master);
    if(handshake(fd_master, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", fd_master, ocm_to_string(MODULE_MASTER));
        exit(EXIT_FAILURE);
    }
    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %d", cqc.ip_master, cqc.puerto_master);

    t_packet* packet = create_packet();
    add_int_to_packet(packet, itself_ocm);
    add_string_to_packet(packet, archive_query);
    add_int_to_packet(packet, priority);
    send_and_free_packet(packet, fd_master);

    for(;;){
        t_list* l = recv_operation_packet(fd_master);
        log_info(logger, "Recibi: %d cantidad de elementos", list_size(l));
        
        int v = list_get_int(l, 0);
        if(v == REQUEST_READ){
            char* archivo = list_get_str(l,1);
            char* contenido = list_get_str(l,2);
            log_info(logger, "## Lectura realizada: %s, contenido: %s", archivo, contenido);
            //Es la lectura del log obligatorio: "## Lectura realizada: Archivo <File:Tag>, contenido: <CONTENIDO>"
        }
        if(v == REQUEST_EXECUTE_QUERY){
            //Sera que el Master solicita el query y este modulo le responde? eso es lo que entendí.
            log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %d", archive_query, priority);
        }
        if(v == REQUEST_KILL){

            //Se debe poner el log de query finalizada y su motivo...
            //Debe ser un char* el motivo???
            char* motivo = list_get_str(l, 1);
            log_info(logger, "## Query Finalizada - %s", motivo);
            break;
        }


        //Por ahora asumo que si recibe cantidad vacía rompo este programa y listo.
        if(list_size(l) == 0)
            break;
    }
    log_info(logger, "## Query Finalizada - %s", "Anotar motivo...");
    return 0;
};