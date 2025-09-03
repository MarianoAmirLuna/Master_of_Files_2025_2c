#include "main.h"

int main(int argc, char* argv[]) {
    itself_ocm = MODULE_QUERY_CONTROL;
    load_config("query_control.config");
    cqc = load_config_query_control();
    create_log("query_control", cqc.log_level);
    log_violet(logger, "%s", "Hola soy QUERY CONTROL");

    if(argc < 3)
    {
        log_pink(logger,"%s, cant: %d", "Cantidad de argumentos inválida", argc);
    }    
    if(argc == 3){
        //El primer argumento se debe cargar el load_config con su ruta definida
    }

    //Al conectarse al Master se debe enviar el path del archivo_query y la prioridad

    //Por ahora pongo boludeces a enviar
    char* archivo_query = "PEPE"; 
    int prioridad = 4;

    int fd_master = client_connection(cqc.ip_master, cqc.puerto_master);
    if(handshake(fd_master, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", fd_master, ocm_to_string(MODULE_MASTER));
        exit(EXIT_FAILURE);
    }
    t_packet* packet = create_packet();
    add_int_to_packet(packet, itself_ocm);
    add_string_to_packet(packet, archivo_query);
    add_int_to_packet(packet, prioridad);
    send_and_free_packet(packet, fd_master);

    for(;;){
        t_list* l = recv_operation_packet(fd_master);
        log_info(logger, "Recibi: %d cantidad de elementos", list_size(l));
        
        //Por ahora asumo que si recibe cantidad vacía rompo este programa y listo.
        if(list_size(l) == 0)
            break;
    }

    return 0;
}
