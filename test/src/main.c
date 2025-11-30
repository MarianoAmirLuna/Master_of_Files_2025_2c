#include "main.h"
#include "commons/crypto.h"

bool comparer(void* a, void *b){
    return (int)a < (int)b;
}

int main(int argc, char* argv[]) {
    
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    load_config("../storage/storage.config");
    config_storage cs = load_config_storage();
    char* data_bloque = malloc(16);
    char* primerdato = "Hola mundo";
    memcpy(data_bloque, primerdato, 16);
    log_orange(logger, "Resultado: %s", data_bloque);
    char* segundodato = "Bueno";
    memcpy(data_bloque, segundodato, 16);
    log_orange(logger, "Resultado: %s", data_bloque);
    

    return;

/*
    parse_code(CREATE, "MATERIAS:BASE");
    parse_code(TAG, "MATERIAS:BASE MATERIAS:V2");
    parse_code(TRUNCATE, "MATERIAS:BASE 1024");
    parse_code(READ, "MATERIAS:BASE 0 8");
    return 0;
*/

    op_code_module ocm = MODULE_STORAGE;
    int wcl = client_connection("127.0.0.1", cs.puerto_escucha);
    if(handshake(wcl, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", wcl, ocm_to_string(ocm));
        exit(EXIT_FAILURE);
    }
    send_john_snow_packet(MODULE_WORKER, wcl);
    t_list* l = recv_operation_packet(wcl);
    int what_operation_is = list_get_int(l, 0);
    int block_size = list_get_int(l, 1);
    log_orange(logger, "TENGO EL BLOCK SIZE DEL STORAGE: %d", block_size);
    list_destroy_and_destroy_elements(l, free_element);



    t_packet* pack = create_packet();
/*
    add_int_to_packet(pack, CREATE_FILE);
    add_string_to_packet(pack, "test_file2 tag_0001");
    send_and_free_packet(pack, wcl);
*/

    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0001 2 aaaa");
    send_and_free_packet(pack, wcl);

/*
    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0005 2 aaaa");
    send_and_free_packet(pack, wcl);
*/


    void* parameters = malloc(sizeof(int)*3);
    int len_args = 2;
    
    int offset = 0;
    memcpy(parameters, &len_args, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &ocm, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &wcl, sizeof(int));

    loop_network(wcl, packet_callback, parameters, NULL);

    return 0;
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
    
    if(ocm == MODULE_MASTER){
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE MASTER
        if(op_code == EJECUTAR_QUERY){
            
        }
    }
    if(ocm == MODULE_STORAGE){
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE STORAGE
        if(op_code == BLOCK_SIZE)
        {
            int storage_block_size = list_get_int(packet, 1);
        }
    }
    

    list_destroy_and_destroy_elements(packet, free_element);
}
